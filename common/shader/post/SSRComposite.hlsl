//
//  SSRComposite.hlsl
//  Square
//
//  Second pass of the screen space reflections (see PostEffectSSR): the frame plus the
//  reflection, blurred by the roughness (5 samples, weighted by their confidence), times the
//  Fresnel (Schlick) of the material: F0 from metallic and albedo (PBR), the specular color
//  (Legacy). Debug 1: only the reflection; 2: the projection check of the trace.
//
#include <Camera>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Sampler2D(g_reflection);
Sampler2D(g_position);
Sampler2D(g_normal);
Sampler2D(g_albedo);
Sampler2D(g_emissive);
Vec2  ssr_size;      //pixels of the frame
Vec4  ssr_params;    //max distance (world), steps, thickness (world), max roughness
float ssr_intensity;
float ssr_debug;     //1: only the reflection, 2: projection check

#include <SSRCommon>

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / ssr_size;
	Vec3 color = texture2DLod(g_source, uv, 0.0).rgb;
	if (ssr_debug > 1.5) return Vec4(texture2DLod(g_reflection, uv, 0.0).rgb, 1.0);
	Vec4 g_pos = texture2DLod(g_position, uv, 0.0);
	//background: the frame
	if (g_pos.w < 0.5) return Vec4(ssr_debug > 0.5 ? Vec3(0.0, 0.0, 0.0) : color, 1.0);
	Vec4  g_nor = texture2DLod(g_normal, uv, 0.0);
	float roughness = ssr_roughness(g_nor.w, g_pos.w);
	float max_roughness = ssr_params.w;
	//the reflection, blurred by the roughness (premultiplied by its confidence)
	Vec2  texel = 1.0 / textureSize2D(g_reflection, 0);
	float radius = 1.0 + 4.0 * saturate(roughness / max_roughness);
	Vec4  sum = texture2DLod(g_reflection, uv, 0.0);
	Vec4  s1 = texture2DLod(g_reflection, uv + texel * Vec2(-radius, -radius), 0.0);
	Vec4  s2 = texture2DLod(g_reflection, uv + texel * Vec2( radius, -radius), 0.0);
	Vec4  s3 = texture2DLod(g_reflection, uv + texel * Vec2(-radius,  radius), 0.0);
	Vec4  s4 = texture2DLod(g_reflection, uv + texel * Vec2( radius,  radius), 0.0);
	Vec3  premultiplied = sum.rgb * sum.a + s1.rgb * s1.a + s2.rgb * s2.a + s3.rgb * s3.a + s4.rgb * s4.a;
	float weight = sum.a + s1.a + s2.a + s3.a + s4.a;
	Vec3  reflection = weight > 0.0001 ? premultiplied / weight : Vec3(0.0, 0.0, 0.0);
	float confidence = weight / 5.0;
	if (ssr_debug > 0.5) return Vec4(reflection * confidence, 1.0);
	//Fresnel (Schlick): F0 of the material
	Vec3 f0;
	if (g_pos.w > 1.5)
	{
		f0 = texture2DLod(g_emissive, uv, 0.0).rgb; //Legacy: the specular color
	}
	else
	{
		Vec4 g_alb = texture2DLod(g_albedo, uv, 0.0);
		f0 = lerp(Vec3(0.04, 0.04, 0.04), g_alb.rgb, saturate(g_alb.a));
	}
	Vec3  n = normalize(g_nor.xyz);
	Vec3  to_camera = normalize(camera.m_position - g_pos.xyz);
	float n_dot_v = saturate(dot(n, to_camera));
	Vec3  fresnel = f0 + (Vec3(1.0, 1.0, 1.0) - f0) * pow(1.0 - n_dot_v, 5.0);
	return Vec4(color + reflection * fresnel * confidence * ssr_intensity, 1.0);
}
