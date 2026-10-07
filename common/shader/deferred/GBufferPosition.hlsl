//
//  GBufferPosition.hlsl
//  Square
//
//  The position of a pixel of the G-Buffer (deferred). Its first target (g_position, RG32F) holds
//  the depth along the view (r: view space, float) and the shading model (g: the model plus
//  GBUFFER_MODEL_OFFSET; the background less, it keeps the clear color). The world position comes
//  back from the depth, the uv of the pixel and the camera (its projection; its model: the inverse
//  of its view).
//
#pragma once
#include <Camera>
#define GBUFFER_MODEL_OFFSET 2.0

//the shading model of a texel of the first target (0: the background)
float gbuffer_model(in Vec4 texel)
{
	return texel.g >= 1.5 ? texel.g - GBUFFER_MODEL_OFFSET : 0.0;
}

//the world position (xyz) and the shading model (w) of a pixel: its texel of the first target,
//its uv on the screen (the same as the old position target: xyz world, w the model)
Vec4 gbuffer_world(in Vec4 texel, in Vec2 uv)
{
	const float model = gbuffer_model(texel);
	const float z = texel.r;
	//the uv on the screen as normalized device coordinates (the rows go down on D3D/Metal, up
	//on OpenGL)
#ifdef SQ_BACKEND_GLSL
	const Vec2 ndc = Vec2(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0);
#else
	const Vec2 ndc = Vec2(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0);
#endif
	//the point of the view (perspective or orthographic) at that depth: the projection of the
	//axis at z (offsets and w) and the scales of x and y (by mul: the same convention as the
	//vertex shaders, no indices of the matrix)
	const Vec4  axis = mul(Vec4(0.0, 0.0, z, 1.0), camera.m_projection);
	const float scale_x = mul(Vec4(1.0, 0.0, 0.0, 0.0), camera.m_projection).x;
	const float scale_y = mul(Vec4(0.0, 1.0, 0.0, 0.0), camera.m_projection).y;
	const float x = (ndc.x * axis.w - axis.x) / scale_x;
	const float y = (ndc.y * axis.w - axis.y) / scale_y;
	const Vec3  world = mul(Vec4(x, y, z, 1.0), camera.m_model).xyz;
	return Vec4(world, model);
}

//the first target of a world position and of its shading model (the geometry pass)
Vec4 gbuffer_encode_world(in Vec3 world, in float model)
{
	const float z = mul(Vec4(world, 1.0), camera.m_view).z;
	return Vec4(z, model + GBUFFER_MODEL_OFFSET, 0.0, 0.0);
}
