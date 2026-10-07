//
//  SSRDenoise.hlsl
//  Square
//
//  The denoiser of the screen space reflections (see PostEffectSSR): a bilateral blur of the
//  trace, separable (a pass along ssr_direction, horizontal then vertical), as the one of
//  HotBite (DenoiserCS): the kernel wider with the roughness (a mirror keeps its details), a
//  sample weighted by how much its surface is the same one of the pixel (normal^5, distance
//  relative to the one from the camera: no blur over the edges) and by a cosine taper.
//  The trace in, the trace out (color, confidence), averaged premultiplied by the confidence.
//
#include <Camera>
#include <GBufferPosition>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);   //the trace (or the first pass)
Sampler2D(g_position);
Sampler2D(g_normal);
Vec2  ssr_size;        //pixels of the target (the trace)
Vec2  ssr_direction;   //(1, 0) horizontal, (0, 1) vertical
Vec4  ssr_params;      //max distance (world), steps, thickness (world), max roughness
float ssr_radius;      //pixels of the kernel at most (the roughest surfaces)

#include <SSRCommon>

#define SSR_DENOISE_MAX_RADIUS 16

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / ssr_size;
	Vec4 center = texture2DLod(g_source, uv, 0.0);
	Vec4 g_pos = gbuffer_world(texture2DLod(g_position, uv, 0.0), uv);
	//background: nothing to denoise
	if (g_pos.w < 0.5) return center;
	Vec3  n0 = normalize(texture2DLod(g_normal, uv, 0.0).xyz);
	float roughness = saturate(ssr_roughness(texture2DLod(g_normal, uv, 0.0).w, g_pos.w) / ssr_params.w);
	//the kernel: one pixel at least (the noise of the rays), wider with the roughness
	int   radius = int(clamp(floor(ssr_radius * roughness), 1.0, min(ssr_radius, float(SSR_DENOISE_MAX_RADIUS))));
	Vec3  to_camera = camera.m_position - g_pos.xyz;
	float camera_distance2 = max(dot(to_camera, to_camera), 0.0001);
	Vec2  texel = ssr_direction / ssr_size;
	Vec4  sum = Vec4(0.0, 0.0, 0.0, 0.0);
	float weights = 0.0;
	[loop]
	for (int i = -SSR_DENOISE_MAX_RADIUS; i <= SSR_DENOISE_MAX_RADIUS; ++i)
	{
		if (abs(i) > radius) continue;
		Vec2 sample_uv = uv + texel * float(i);
		if (sample_uv.x < 0.0 || sample_uv.x > 1.0 || sample_uv.y < 0.0 || sample_uv.y > 1.0) continue;
		Vec4 p1 = gbuffer_world(texture2DLod(g_position, sample_uv, 0.0), sample_uv);
		if (p1.w < 0.5) continue; //the background is not a surface
		Vec3  n1 = normalize(texture2DLod(g_normal, sample_uv, 0.0).xyz);
		Vec3  offset = p1.xyz - g_pos.xyz;
		//the same surface: the normals near, near in world (relative to the camera distance)
		float normal_weight = pow(saturate(dot(n0, n1)), 5.0);
		float relative_distance = max(dot(offset, offset) / camera_distance2, 0.1);
		float taper = cos(3.14159265 * abs(float(i)) / (2.0 * float(radius) + 2.0));
		float weight = normal_weight / relative_distance * taper;
		Vec4  value = texture2DLod(g_source, sample_uv, 0.0);
		if (any(isnan(value)) || any(isinf(value))) continue;
		sum += Vec4(value.rgb * value.a, value.a) * weight;
		weights += weight;
	}
	if (weights < 0.0001) return center;
	sum /= weights;
	//back to the trace: color, confidence
	return Vec4(sum.a > 0.0001 ? sum.rgb / sum.a : Vec3(0.0, 0.0, 0.0), sum.a);
}
