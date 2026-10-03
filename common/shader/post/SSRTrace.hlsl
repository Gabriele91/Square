//
//  SSRTrace.hlsl
//  Square
//
//  First pass of the screen space reflections (see PostEffectSSR): the ray of each pixel,
//  reflected on its normal, marches (ssr_march):
//   0) world: steps of the same length in world space, each one projected on the screen;
//   1) screen: the ray projected on the screen once, then walked with steps of the same
//      length in pixels (DDA, McGuire and Mara 2014), clipped to the near plane and to the
//      screen; the world point of a step by perspective interpolation (position / w and 1 / w
//      are linear on the screen). No step is wasted on the same pixel, none jumps a thin object
//      near the camera.
//  Each step is compared with the G-Buffer surface on its pixel by the distance from the camera
//  (the two points are on the same view ray): behind it, within the thickness, is a hit,
//  refined by bisection. Output: the color of the frame at the hit, alpha the confidence.
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
float ssr_march;  //0: world, 1: screen

#include <SSRCommon>

//how much a ray at ray_position (on the screen at uv) is behind the G-Buffer surface there
//(> 0 behind)
float ssr_surface_difference(in Vec3 ray_position, in Vec2 uv, out bool valid)
{
	valid = uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0;
	if (!valid) return -1.0;
	Vec4 surface = texture2DLod(g_position, uv, 0.0);
	if (surface.w < 0.5) return -1.0; //background: nothing to hit
	return length(ray_position - camera.m_position) - length(surface.xyz - camera.m_position);
}

//the same of a world point, projected
float ssr_depth_difference(in Vec3 ray_position, out Vec2 uv, out bool valid)
{
	float w;
	uv = ssr_project(ray_position, w);
	if (w <= 0.0)
	{
		valid = false;
		return -1.0;
	}
	return ssr_surface_difference(ray_position, uv, valid);
}

//0) world: steps of the same length along the ray. Out: the uv of the hit, the distance along
//the ray
bool ssr_march_world(in Vec3 origin, in Vec3 ray, in float jitter, out Vec2 hit_uv, out float hit_t)
{
	float max_distance = ssr_params.x;
	int   steps = int(ssr_params.y);
	float thickness = ssr_params.z;
	float step_length = max_distance / float(steps);
	float t_before = 0.0;
	float t = step_length * (0.25 + jitter);
	bool  hit = false;
	hit_uv = Vec2(0.0, 0.0);
	hit_t = 0.0;
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
	if (!hit) return false;
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
	hit_uv = ssr_project(origin + ray * t_behind, w);
	hit_t = t_behind;
	return true;
}

//the world point at s (0 start, 1 end) of a ray on the screen: q (position / w) and k (1 / w)
//are linear on the screen
Vec3 ssr_screen_point(in Vec3 q0, in Vec3 q1, in float k0, in float k1, in float s)
{
	return lerp(q0, q1, s) / lerp(k0, k1, s);
}

//1) screen: steps of the same length in pixels along the ray projected on the screen (DDA)
bool ssr_march_screen(in Vec3 origin, in Vec3 ray, in float jitter, out Vec2 hit_uv, out float hit_t)
{
	float max_distance = ssr_params.x;
	int   steps = int(ssr_params.y);
	float thickness = ssr_params.z;
	hit_uv = Vec2(0.0, 0.0);
	hit_t = 0.0;
	//the end of the ray, before the near plane (w linear along it)
	const float min_w = 0.01;
	float ray_length = max_distance;
	Vec4  h0 = ssr_clip(origin);
	Vec4  h1 = ssr_clip(origin + ray * ray_length);
	if (h0.w < min_w) return false;
	if (h1.w < min_w)
	{
		ray_length *= (h0.w - min_w) / max(h0.w - h1.w, 0.00001);
		h1 = ssr_clip(origin + ray * ray_length);
	}
	Vec3  end = origin + ray * ray_length;
	//on the screen: uv linear, position / w and 1 / w for the world point
	Vec2  uv0 = ssr_clip_to_uv(h0);
	Vec2  uv1 = ssr_clip_to_uv(h1);
	float k0 = 1.0 / h0.w;
	float k1 = 1.0 / h1.w;
	Vec3  q0 = origin * k0;
	Vec3  q1 = end * k1;
	//clipped to the screen: s_max where it leaves it
	Vec2  delta = uv1 - uv0;
	float s_max = 1.0;
	if (delta.x > 0.0) s_max = min(s_max, (1.0 - uv0.x) / delta.x);
	if (delta.x < 0.0) s_max = min(s_max, -uv0.x / delta.x);
	if (delta.y > 0.0) s_max = min(s_max, (1.0 - uv0.y) / delta.y);
	if (delta.y < 0.0) s_max = min(s_max, -uv0.y / delta.y);
	//the step: one pixel at least, the visible part of the ray in the steps
	Vec2  pixels = abs(delta * ssr_size) * s_max;
	float length_pixels = max(pixels.x, pixels.y);
	if (length_pixels < 1.0) return false;
	float step_s = max(s_max / float(steps), s_max / length_pixels);
	float s_before = 0.0;
	float s = step_s * (0.5 + jitter);
	bool  hit = false;
	[loop]
	for (int i = 0; i < 128; ++i)
	{
		if (i >= steps || s > s_max) break;
		bool  valid;
		float difference = ssr_surface_difference(ssr_screen_point(q0, q1, k0, k1, s), lerp(uv0, uv1, s), valid);
		if (!valid) break; //off the screen
		if (difference > 0.0 && difference < thickness)
		{
			hit = true;
			break;
		}
		s_before = s;
		s += step_s;
	}
	if (!hit) return false;
	//bisection between the last step in front and the first behind
	float s_front = s_before;
	float s_behind = s;
	[loop]
	for (int j = 0; j < 6; ++j)
	{
		float s_middle = (s_front + s_behind) * 0.5;
		bool  valid;
		float difference = ssr_surface_difference(ssr_screen_point(q0, q1, k0, k1, s_middle), lerp(uv0, uv1, s_middle), valid);
		if (valid && difference > 0.0) s_behind = s_middle;
		else                           s_front = s_middle;
	}
	hit_uv = lerp(uv0, uv1, s_behind);
	hit_t = length(ssr_screen_point(q0, q1, k0, k1, s_behind) - origin);
	return true;
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
	Vec3  origin = g_pos.xyz + n * (0.002 * length(g_pos.xyz - camera.m_position) + 0.01);
	float jitter = ssr_noise(input.m_position.xy);
	Vec2  hit_uv;
	float hit_t;
	bool  hit = ssr_march > 0.5 ? ssr_march_screen(origin, ray, jitter, hit_uv, hit_t)
	                            : ssr_march_world(origin, ray, jitter, hit_uv, hit_t);
	if (!hit) return Vec4(0.0, 0.0, 0.0, 0.0);
	//a back face (the ray hits it from behind): nothing
	Vec3 hit_normal = normalize(texture2DLod(g_normal, hit_uv, 0.0).xyz);
	if (dot(hit_normal, ray) > 0.0) return Vec4(0.0, 0.0, 0.0, 0.0);
	//confidence: screen border, distance along the ray, rays to the camera, roughness
	Vec2  border = min(hit_uv, 1.0 - hit_uv) / ssr_fade;
	float border_fade = saturate(min(border.x, border.y));
	float distance_fade = 1.0 - saturate(hit_t / ssr_params.x);
	float roughness_fade = 1.0 - saturate(roughness / max_roughness);
	float confidence = border_fade * distance_fade * camera_fade * roughness_fade;
	//the color of the frame there (a NaN/Inf pixel counts as black)
	Vec3 color = texture2DLod(g_source, hit_uv, 0.0).rgb;
	if (any(isnan(color)) || any(isinf(color))) color = Vec3(0.0, 0.0, 0.0);
	return Vec4(max(color, Vec3(0.0, 0.0, 0.0)), confidence);
}
