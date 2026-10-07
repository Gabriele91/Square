//
//  GodRaysCommon.hlsl
//  Square
//
//  Shared by the passes of the god rays (see PostEffectGodRays). Include it after <Camera> and
//  the uniform rays_sun (xyz: toward the sun, w: the screen shares out of the view where the
//  rays fade).
//
#pragma once

//uv of the screen of the sun (a direction: a point at infinity); w: its clip w (not positive:
//behind the camera). The rows of the targets go down on D3D/Metal, up on OpenGL
Vec2 rays_sun_uv(out float w)
{
	Vec4 clip = mul(mul(Vec4(rays_sun.xyz, 0.0), camera.m_view), camera.m_projection);
	w = clip.w;
	Vec2 ndc = clip.xy / max(abs(clip.w), 0.00001);
#ifdef GLSL_BACKEND
	return Vec2(0.5 + 0.5 * ndc.x, 0.5 + 0.5 * ndc.y);
#else
	return Vec2(0.5 + 0.5 * ndc.x, 0.5 - 0.5 * ndc.y);
#endif
}

//how much the sun makes rays: 1 in the view, less out of it (none behind the camera); a fade
//not positive: always 1 (the volumetric light)
float rays_sun_visibility(in Vec2 sun_uv, in float w)
{
	if (rays_sun.w <= 0.0) return 1.0;
	if (w <= 0.0) return 0.0;
	Vec2  beyond = max(max(-sun_uv, sun_uv - 1.0), Vec2(0.0, 0.0));
	float outside = max(beyond.x, beyond.y);
	return saturate(1.0 - outside / rays_sun.w);
}
