//
//  SSAOCommon.hlsl
//  Square
//
//  Shared by the passes of the SSAO (see PostEffectSSAO). Include it after <Camera>.
//  The occlusion passes can be at half size: the texel t of their textures is the G-Buffer
//  pixel t * ssao_scale (the depth pass reads that one), so a position is rebuilt at the uv of
//  that pixel, from the view depth (along the camera forward) and the view rays.
//
#pragma once

Vec2  ssao_full_size; //pixels of the frame (G-Buffer)
float ssao_scale;     //1: full size, 2: half size
Vec4  ssao_ray_c;     //view ray of the center of the screen (the camera forward), world
Vec4  ssao_ray_x;     //+ndc.x * this: the view ray at ndc.x (per unit of view depth)
Vec4  ssao_ray_y;     //+ndc.y * this

//uv of the G-Buffer pixel of a texel (of a texture of size `size`) at uv
Vec2 ssao_source_uv(in Vec2 uv, in Vec2 size)
{
	return (floor(uv * size) * ssao_scale + 0.5) / ssao_full_size;
}

//view depth of a world point
float ssao_view_depth(in Vec3 world)
{
	return dot(world - camera.m_position, ssao_ray_c.xyz);
}

//world point at the uv of the frame, at a view depth. The rows of the targets go down on
//D3D/Metal, up on OpenGL (as SSRCommon)
Vec3 ssao_world(in Vec2 uv, in float depth)
{
#ifdef GLSL_BACKEND
	Vec2 ndc = Vec2(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0);
#else
	Vec2 ndc = Vec2(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0);
#endif
	return camera.m_position + (ssao_ray_c.xyz + ssao_ray_x.xyz * ndc.x + ssao_ray_y.xyz * ndc.y) * depth;
}

//weight of a sample at view depth `depth` for a pixel at view depth `center` (the edges are kept:
//other surfaces count less, the background nothing)
float ssao_depth_weight(in float depth, in float center, in float sharpness)
{
	if (depth <= 0.0) return 0.0;
	return exp(-abs(depth - center) / max(center, 0.001) * sharpness);
}
