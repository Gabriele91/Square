//
//  GodRaysBlur.hlsl
//  Square
//
//  Second pass of the god rays (see PostEffectGodRays): every pixel the sum of the mask toward
//  the sun, each sample weaker than the one before (decay), over a share of the way (density).
//
#include <Camera>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_mask);
Vec2 rays_size; //pixels of the target
Vec4 rays_sun;  //toward the sun, fade
Vec4 rays_blur; //density, decay, weight, samples
#include <GodRaysCommon>

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / rays_size;
	float w;
	Vec2  sun = rays_sun_uv(w);
	if (w <= 0.0) return Vec4(0.0, 0.0, 0.0, 1.0);
	const int samples = int(rays_blur.w);
	Vec2  advance = (uv - sun) * rays_blur.x / rays_blur.w;
	Vec2  at = uv;
	Vec3  sum = Vec3(0.0, 0.0, 0.0);
	float light = 1.0;
	[loop]
	for (int i = 0; i < 128; ++i)
	{
		if (i >= samples) break;
		at -= advance;
		sum += texture2DLod(g_mask, at, 0.0).rgb * light * rays_blur.z;
		light *= rays_blur.y;
	}
	return Vec4(sum, 1.0);
}
