//
//  SSRComposite.hlsl
//  Square
//
//  Last pass of the screen space reflections (see PostEffectSSR): the frame plus the
//  reflection, blurred (Settings::blur: off; low, 5 samples wider with the roughness; medium
//  and high, the 5 samples on a mirror, the levels of the blur chain on a rougher surface, two
//  near levels mixed), times the Fresnel (Schlick) of the material: F0 from metallic and albedo (PBR), the specular color
//  (Legacy). Debug 1: only the reflection; 2: the projection check of the trace.
//
#include <Camera>
#include <GBufferPosition>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Sampler2D(g_reflection);   //the trace (color, confidence)
Sampler2D(g_reflection_1); //the blur levels, premultiplied by the confidence (see SSRDownsample)
Sampler2D(g_reflection_2);
Sampler2D(g_reflection_3);
Sampler2D(g_reflection_4);
Sampler2D(g_reflection_5);
Sampler2D(g_position);
Sampler2D(g_normal);
Sampler2D(g_albedo);
Sampler2D(g_emissive);
Vec2  ssr_size;      //pixels of the frame
Vec4  ssr_params;    //max distance (world), steps, thickness (world), max roughness
float ssr_intensity;
float ssr_debug;     //1: only the reflection, 2: projection check
float ssr_blur;      //Settings::BlurQuality (0 off, 1 low, 2 medium, 3 high)
float ssr_levels;    //levels of the blur chain (0: none, at most 5)

#include <SSRCommon>

//the weight of a level for the level of detail lod (the two near levels, linear between them)
float ssr_level_weight(float lod, float level)
{
	return saturate(1.0 - abs(lod - level));
}

//a sample of the trace, premultiplied by its confidence
Vec4 ssr_trace(Vec2 uv)
{
	Vec4 value = texture2DLod(g_reflection, uv, 0.0);
	return Vec4(value.rgb * value.a, value.a);
}

//5 samples of the trace (the center, 4 corners radius texels away): the noise of the rays out
Vec4 ssr_trace_box(Vec2 uv, float radius)
{
	Vec2 texel = radius / textureSize2D(g_reflection, 0);
	return ( ssr_trace(uv)
	       + ssr_trace(uv + texel * Vec2(-1.0, -1.0))
	       + ssr_trace(uv + texel * Vec2( 1.0, -1.0))
	       + ssr_trace(uv + texel * Vec2(-1.0,  1.0))
	       + ssr_trace(uv + texel * Vec2( 1.0,  1.0)) ) / 5.0;
}

//the reflection at lod (0: the trace, by 5 samples; n: the level n), premultiplied
Vec4 ssr_reflection(Vec2 uv, float lod)
{
	Vec4 sum = ssr_trace_box(uv, 1.0) * ssr_level_weight(lod, 0.0);
	if (ssr_levels >= 1.0) sum += texture2DLod(g_reflection_1, uv, 0.0) * ssr_level_weight(lod, 1.0);
	if (ssr_levels >= 2.0) sum += texture2DLod(g_reflection_2, uv, 0.0) * ssr_level_weight(lod, 2.0);
	if (ssr_levels >= 3.0) sum += texture2DLod(g_reflection_3, uv, 0.0) * ssr_level_weight(lod, 3.0);
	if (ssr_levels >= 4.0) sum += texture2DLod(g_reflection_4, uv, 0.0) * ssr_level_weight(lod, 4.0);
	if (ssr_levels >= 5.0) sum += texture2DLod(g_reflection_5, uv, 0.0) * ssr_level_weight(lod, 5.0);
	return sum;
}

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / ssr_size;
	Vec3 color = texture2DLod(g_source, uv, 0.0).rgb;
	if (ssr_debug > 1.5) return Vec4(texture2DLod(g_reflection, uv, 0.0).rgb, 1.0);
	Vec4 g_pos = gbuffer_world(texture2DLod(g_position, uv, 0.0), uv);
	//background: the frame
	if (g_pos.w < 0.5) return Vec4(ssr_debug > 0.5 ? Vec3(0.0, 0.0, 0.0) : color, 1.0);
	Vec4  g_nor = texture2DLod(g_normal, uv, 0.0);
	float roughness = ssr_roughness(g_nor.w, g_pos.w);
	float max_roughness = ssr_params.w;
	//the reflection, blurred by the roughness
	float rough = saturate(roughness / max_roughness);
	Vec4  sum;
	if      (ssr_blur < 0.5)   sum = ssr_trace(uv);                            //off
	else if (ssr_levels < 0.5) sum = ssr_trace_box(uv, 1.0 + 4.0 * rough);    //low
	else                       sum = ssr_reflection(uv, rough * ssr_levels);  //medium, high
	float confidence = saturate(sum.a);
	Vec3  reflection = sum.a > 0.0001 ? sum.rgb / sum.a : Vec3(0.0, 0.0, 0.0);
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
