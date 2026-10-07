//
//  GodRaysVolume.hlsl
//  Square
//
//  First pass of the volumetric god rays (see PostEffectGodRays): the light of the sun in the
//  air along the ray of the pixel, from the camera to its geometry (at most max distance), in
//  steps (jittered by the pixel: blurred after), each one lit where the cascades of the shadow of
//  the sun say so; more looking toward the sun (Henyey-Greenstein), the scattered share by the
//  density of the air. The background (no geometry): nothing.
//
#include <Camera>
#include <Transform>
#include <Vertex>
#include <DirectionShadowLight>
#include <DeferredFullscreen>

Sampler2D(g_position);
Vec2 rays_size;   //pixels of the target
Vec4 rays_sun;    //toward the sun, -
Vec4 rays_volume; //max distance, steps, anisotropy, density of the air

//interleaved gradient noise of a pixel, [0, 1)
float rays_noise(in Vec2 pixel)
{
	return frac(52.9829189 * frac(dot(pixel, Vec2(0.06711056, 0.00583715))));
}

//a point of the air in the light of the sun (1) or in its shadow (0); past the shadow: lit
float rays_lit(in Vec3 position)
{
	float depth = abs(mul(Vec4(position, 1.0), camera.m_view).z);
	uint  cascades = uint(clamp(direction_shadow_camera.m_options.y, 1, DIRECTION_SHADOW_CSM_NUMBER_OF_FACES));
	float shadow_distance = abs(direction_shadow_camera.m_data[cascades - 1][DEPTH]);
	if (depth >= shadow_distance) return 1.0;
	uint  cascade = find_csm_layer(depth);
	Vec4  light_space = mul_direction_light_view_projection(Vec4(position, 1.0), cascade);
	Vec3  coords = light_space.xyz / light_space.w;
	coords.xy = coords.xy * 0.5 + 0.5;
	if (coords.x <= 0.0 || coords.x >= 1.0 || coords.y <= 0.0 || coords.y >= 1.0) return 1.0;
	coords = invY(coords);
	float closest = shadow2DArray(direction_shadow_map, Vec3(coords.xy, cascade)).r;
#ifdef GLSL_BACKEND
	float current = coords.z * 0.5 + 0.5;
#else
	float current = coords.z;
#endif
	return current - 0.001 <= closest ? 1.0 : 0.0;
}

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / rays_size;
	Vec4 g_pos = texture2DLod(g_position, uv, 0.0);
	if (g_pos.w < 0.5) return Vec4(0.0, 0.0, 0.0, 1.0);
	//the ray in the air
	Vec3  from = camera.m_position;
	Vec3  way = g_pos.xyz - from;
	float length_of_way = length(way);
	Vec3  direction = way / max(length_of_way, 0.0001);
	float reach = min(length_of_way, rays_volume.x);
	//its steps, lit or not
	const int steps = int(rays_volume.y);
	float jitter = rays_noise(input.m_position.xy);
	float lit = 0.0;
	[loop]
	for (int i = 0; i < 128; ++i)
	{
		if (i >= steps) break;
		float t = (float(i) + jitter) / rays_volume.y * reach;
		lit += rays_lit(from + direction * t);
	}
	lit /= rays_volume.y;
	//toward the sun more (Henyey-Greenstein, 1 all around)
	float g = rays_volume.z;
	float cos_theta = dot(direction, rays_sun.xyz);
	float phase = (1.0 - g * g) / pow(max(1.0 + g * g - 2.0 * g * cos_theta, 0.0001), 1.5);
	//the share of the light the air scatters along the way
	float scattered = 1.0 - exp(-rays_volume.w * reach);
	float light = lit * phase * scattered;
	return Vec4(light, light, light, 1.0);
}
