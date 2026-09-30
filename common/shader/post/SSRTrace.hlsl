//
//  SSRTrace.hlsl
//  Square
//
//  First pass of the screen space reflections (see PostEffectSSR): the ray of each pixel,
//  reflected on its normal, marches in world space; each step is projected on the screen and
//  compared with the G-Buffer surface there by the distance from the camera (the two points
//  are on the same view ray): behind it, within the thickness, is a hit, refined by
//  bisection. Output: the color of the frame at the hit, alpha the confidence.
//  Debug 2: the projection of the pixel itself against its uv (black right, red wrong).
//
#include <Camera>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Sampler2D(g_position);
Sampler2D(g_normal);
Vec2  ssr_size;   //pixels of the target
Vec4  ssr_params; //max distance (world), steps, thickness (world), max roughness
float ssr_fade;   //share of the screen border where it fades
float ssr_debug;  //2: projection check

#include <SSRCommon>

//how much a ray at ray_position is behind the G-Buffer surface on its pixel (> 0 behind), and the uv
float ssr_depth_difference(in Vec3 ray_position, out Vec2 uv, out bool valid)
{
	float w;
	uv = ssr_project(ray_position, w);
	valid = w > 0.0 && uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0;
	if (!valid) return -1.0;
	Vec4 surface = texture2DLod(g_position, uv, 0.0);
	if (surface.w < 0.5) return -1.0; //background: nothing to hit
	return length(ray_position - camera.m_position) - length(surface.xyz - camera.m_position);
}

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / ssr_size;
	Vec4 g_pos = texture2DLod(g_position, uv, 0.0);
	//background: no reflection
	if (g_pos.w < 0.5) return Vec4(0.0, 0.0, 0.0, 0.0);
	//projection check: the pixel back on the screen
	if (ssr_debug > 1.5)
	{
		float w;
		Vec2 projected = ssr_project(g_pos.xyz, w);
		//error in pixels, less the half pixel between this target and the G-Buffer (black: right)
		return Vec4(saturate(length((projected - uv) * ssr_size) - 0.75), 0.0, 0.0, 1.0);
	}
	Vec4  g_nor = texture2DLod(g_normal, uv, 0.0);
	float roughness = ssr_roughness(g_nor.w, g_pos.w);
	float max_roughness = ssr_params.w;
	if (roughness >= max_roughness) return Vec4(0.0, 0.0, 0.0, 0.0);
	Vec3  n = normalize(g_nor.xyz);
	Vec3  view = normalize(g_pos.xyz - camera.m_position);
	Vec3  ray = reflect(view, n);
	//rays back to the camera leave the screen: they fade
	float camera_fade = saturate((dot(ray, view) + 0.4) / 0.4);
	if (camera_fade <= 0.0) return Vec4(0.0, 0.0, 0.0, 0.0);
	//march
	float max_distance = ssr_params.x;
	int   steps = int(ssr_params.y);
	float thickness = ssr_params.z;
	float step_length = max_distance / float(steps);
	Vec3  origin = g_pos.xyz + n * (0.002 * length(g_pos.xyz - camera.m_position) + 0.01);
	float t_before = 0.0;
	float t = step_length * (0.25 + ssr_noise(input.m_position.xy));
	bool  hit = false;
	[loop]
	for (int i = 0; i < 128; ++i)
	{
		if (i >= steps) break;
		Vec2  step_uv;
		bool  valid;
		float difference = ssr_depth_difference(origin + ray * t, step_uv, valid);
		if (!valid) break; //off the screen
		if (difference > 0.0 && difference < thickness)
		{
			hit = true;
			break;
		}
		t_before = t;
		t += step_length;
	}
	if (!hit) return Vec4(0.0, 0.0, 0.0, 0.0);
	//bisection between the last step in front and the first behind
	float t_front = t_before;
	float t_behind = t;
	[loop]
	for (int j = 0; j < 6; ++j)
	{
		float t_middle = (t_front + t_behind) * 0.5;
		Vec2  middle_uv;
		bool  valid;
		float difference = ssr_depth_difference(origin + ray * t_middle, middle_uv, valid);
		if (valid && difference > 0.0) t_behind = t_middle;
		else                           t_front = t_middle;
	}
	float w;
	Vec2 hit_uv = ssr_project(origin + ray * t_behind, w);
	//a back face (the ray hits it from behind): nothing
	Vec3 hit_normal = normalize(texture2DLod(g_normal, hit_uv, 0.0).xyz);
	if (dot(hit_normal, ray) > 0.0) return Vec4(0.0, 0.0, 0.0, 0.0);
	//confidence: screen border, distance along the ray, rays to the camera, roughness
	Vec2  border = min(hit_uv, 1.0 - hit_uv) / ssr_fade;
	float border_fade = saturate(min(border.x, border.y));
	float distance_fade = 1.0 - saturate(t_behind / max_distance);
	float roughness_fade = 1.0 - saturate(roughness / max_roughness);
	float confidence = border_fade * distance_fade * camera_fade * roughness_fade;
	//the color of the frame there (a NaN/Inf pixel counts as black)
	Vec3 color = texture2DLod(g_source, hit_uv, 0.0).rgb;
	if (any(isnan(color)) || any(isinf(color))) color = Vec3(0.0, 0.0, 0.0);
	return Vec4(max(color, Vec3(0.0, 0.0, 0.0)), confidence);
}
