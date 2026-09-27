//
//  SSAO.hlsl
//  Square
//
//  Raw screen space ambient occlusion (see PostEffectSSAO), from the G-Buffer world
//  positions and normals, Alchemy AO (McGuire et al. 2011):
//    ao = max(0, 1 - 2 * intensity / N * sum(max(0, v.n - bias) / (v.v + e)))^contrast
//  16 samples in screen space, within a radius of ssao_params.x world units projected on
//  the screen, rotated per pixel on a 4x4 pattern (removed by the 4x4 blur of SSAOApply).
//  Output: occlusion in r (1 = open, 0 = closed).
//
#include <Camera>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_position);
Sampler2D(g_normal);
Vec2 ssao_size;   //pixels of the frame
Vec4 ssao_params; //radius (world), intensity, bias (world), contrast
Vec2 ssao_pixels; //pixels of a world unit at distance 1, radius on the screen at most

//occlusion of the point at uv on p (normal n)
float ssao_occlusion(in Vec3 p, in Vec3 n, in Vec2 uv)
{
	Vec4 occluder = texture2DLod(g_position, uv, 0.0); //in a loop: no derivatives
	//background: nothing
	if (occluder.w < 0.5) return 0.0;
	Vec3  v = occluder.xyz - p;
	float vv = dot(v, v);
	//within the radius (it fades towards it)
	float falloff = saturate(1.0 - vv / (ssao_params.x * ssao_params.x));
	//in front of the surface (cosine), more the nearer (1 / distance)
	return max(0.0, dot(v, n) - ssao_params.z) / (vv + 0.01) * falloff;
}

Vec2 ssao_rotate(in Vec2 v, in Vec2 cos_sin)
{
	return Vec2(v.x * cos_sin.x - v.y * cos_sin.y, v.x * cos_sin.y + v.y * cos_sin.x);
}

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / ssao_size;
	Vec4 g_pos = texture2DLod(g_position, uv, 0.0);
	//background: open
	if (g_pos.w < 0.5) return Vec4(1.0, 1.0, 1.0, 1.0);
	Vec3 p = g_pos.xyz;
	Vec3 n = normalize(texture2DLod(g_normal, uv, 0.0).xyz);
	//radius on the screen (pixels): smaller far away
	float distance = max(length(p - camera.m_position), 0.1);
	float radius = min(ssao_params.x * ssao_pixels.x / distance, ssao_pixels.y);
	//rotation of the samples: one of 16 angles, on a 4x4 pattern of pixels
	Vec2  cell = fmod(floor(input.m_position.xy), 4.0);
	float angle = (cell.y * 4.0 + cell.x) * (6.28318530718 / 16.0);
	Vec2  cos_sin = Vec2(cos(angle), sin(angle));
	//4 directions, 4 distances each (alternating a 45 degrees turn)
	static const Vec2 directions[4] =
	{
		Vec2( 1.0,  0.0),
		Vec2(-1.0,  0.0),
		Vec2( 0.0,  1.0),
		Vec2( 0.0, -1.0)
	};
	float ao = 0.0;
	for (int i = 0; i < 4; ++i)
	{
		Vec2 k1 = ssao_rotate(directions[i], cos_sin) * radius / ssao_size;
		Vec2 k2 = Vec2(k1.x * 0.707 - k1.y * 0.707, k1.x * 0.707 + k1.y * 0.707);
		ao += ssao_occlusion(p, n, uv + k1 * 0.25);
		ao += ssao_occlusion(p, n, uv + k2 * 0.5);
		ao += ssao_occlusion(p, n, uv + k1 * 0.75);
		ao += ssao_occlusion(p, n, uv + k2);
	}
	ao = pow(max(0.0, 1.0 - 2.0 * ssao_params.y / 16.0 * ao), ssao_params.w);
	return Vec4(ao, ao, ao, 1.0);
}
