//
//  SSRCommon.hlsl
//  Square
//
//  Shared by the passes of the screen space reflections (see PostEffectSSR). Include it after
//  <Camera>.
//
#pragma once

//uv of the screen of a world point (the one of the G-Buffer, from SV_POSITION); w: clip w (not
//positive: behind the camera). The rows of the targets go down on D3D/Metal, up on OpenGL
//(checked by the debug 2 of the trace: black on every backend)
Vec4 ssr_clip(in Vec3 world)
{
	return mul(mul(Vec4(world, 1.0), camera.m_view), camera.m_projection);
}

Vec2 ssr_clip_to_uv(in Vec4 clip)
{
	Vec2 ndc = clip.xy / max(abs(clip.w), 0.00001);
#ifdef SQ_BACKEND_GLSL
	return Vec2(0.5 + 0.5 * ndc.x, 0.5 + 0.5 * ndc.y);
#else
	return Vec2(0.5 + 0.5 * ndc.x, 0.5 - 0.5 * ndc.y);
#endif
}

Vec2 ssr_project(in Vec3 world, out float w)
{
	Vec4 clip = ssr_clip(world);
	w = clip.w;
	return ssr_clip_to_uv(clip);
}

//roughness of a G-Buffer pixel (its material: gbuffer_material): PBR (model 1) keeps it,
//Legacy (model 2) the Blinn-Phong exponent (roughness = sqrt(2 / (shininess + 2)))
float ssr_roughness(in float material, in float model)
{
	if (model > 1.5) return sqrt(2.0 / (max(material, 0.0) + 2.0));
	return saturate(material);
}

//interleaved gradient noise of a pixel, [0, 1)
float ssr_noise(in Vec2 pixel)
{
	return frac(52.9829189 * frac(dot(pixel, Vec2(0.06711056, 0.00583715))));
}
