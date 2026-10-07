//
//  GodRaysMask.hlsl
//  Square
//
//  First pass of the god rays (see PostEffectGodRays): the sky (the background of the G-Buffer,
//  and the black emissive geometry: a sky dome) over the threshold, brighter near the sun on the
//  screen; the rest black (it blocks the light).
//
#include <Camera>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Sampler2D(g_position);
Sampler2D(g_albedo);
Sampler2D(g_emissive);
Vec2 rays_size; //pixels of the target
Vec4 rays_sun;  //toward the sun, fade
Vec4 rays_mask; //threshold, radius (screen heights), aspect of the frame, the emissive sky (1 on)
#include <GodRaysCommon>

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / rays_size;
	//the geometry: no light through it (but a sky of geometry: black, only emissive)
	Vec4 position = texture2DLod(g_position, uv, 0.0);
	if (position.w > 0.5)
	{
		Vec3  albedo = texture2DLod(g_albedo, uv, 0.0).rgb;
		Vec3  emissive = texture2DLod(g_emissive, uv, 0.0).rgb;
		const bool black = max(albedo.r, max(albedo.g, albedo.b)) < 0.02;
		const bool glows = max(emissive.r, max(emissive.g, emissive.b)) > 0.001;
		const bool sky = rays_mask.w > 0.5 && black && glows;
		if (!sky) return Vec4(0.0, 0.0, 0.0, 1.0);
	}
	float w;
	Vec2  sun = rays_sun_uv(w);
	if (w <= 0.0) return Vec4(0.0, 0.0, 0.0, 1.0);
	//the sky near the sun
	Vec2  offset = (uv - sun) * Vec2(rays_mask.z, 1.0);
	float close = saturate(1.0 - length(offset) / rays_mask.y);
	Vec3  sky = max(texture2DLod(g_source, uv, 0.0).rgb - rays_mask.x, Vec3(0.0, 0.0, 0.0));
	return Vec4(sky * close * close, 1.0);
}
