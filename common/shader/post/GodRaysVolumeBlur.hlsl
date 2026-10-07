//
//  GodRaysVolumeBlur.hlsl
//  Square
//
//  Second pass of the volumetric god rays (see PostEffectGodRays): the noise of the jittered
//  steps blurred, 3x3 taps a texel and a half apart (each one bilinear: 4 texels).
//
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_rays);
Vec2 rays_size; //pixels of the target

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / rays_size;
	Vec2 texel = 1.5 / rays_size;
	Vec3 sum = Vec3(0.0, 0.0, 0.0);
	[unroll]
	for (int y = -1; y <= 1; ++y)
	{
		[unroll]
		for (int x = -1; x <= 1; ++x)
		{
			sum += texture2DLod(g_rays, uv + Vec2(float(x), float(y)) * texel, 0.0).rgb;
		}
	}
	return Vec4(sum / 9.0, 1.0);
}
