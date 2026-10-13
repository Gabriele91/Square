//
//  GBufferPosition.hlsl
//  Square
//
//  The position and the normal of a pixel of the G-Buffer (deferred):
//   - the first target (g_position, R32F) holds the distance along the view of a surface,
//     negated (r < 0: its z in view space by the sign of the view, gbuffer_view_sign; the
//     background keeps the clear color, r >= 0). The world position comes back from the depth,
//     the uv of the pixel and the camera (its projection; its model: the inverse of its view);
//   - the second target (g_normal, RGBA16F) holds the world normal (octahedral: xy), the
//     roughness or the shininess of the model (z) and the shading model (w: 0 the background,
//     the clear; see SurfaceDeferredPBR / SurfaceDeferredLegacy).
//
#pragma once
#include <Camera>

//the sign of the z in view space of what the camera sees (1: in front of it z grows, -1: it
//goes down): its projection takes the depth up along it (perspective and orthographic)
float gbuffer_view_sign()
{
	return mul(Vec4(0.0, 0.0, 1.0, 0.0), camera.m_projection).z >= 0.0 ? 1.0 : -1.0;
}

//the world position (xyz) of a pixel and if a surface is there (w: 1, the background 0): its
//texel of the first target, its uv on the screen
Vec4 gbuffer_world(in Vec4 texel, in Vec2 uv)
{
	const float z = -texel.r * gbuffer_view_sign();
	const float surface = texel.r < 0.0 ? 1.0 : 0.0;
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
	return Vec4(world, surface);
}

//the first target of a world position (the geometry pass): its distance along the view, negated
Vec4 gbuffer_encode_world(in Vec3 world)
{
	const float z = mul(Vec4(world, 1.0), camera.m_view).z;
	return Vec4(-z * gbuffer_view_sign(), 0.0, 0.0, 0.0);
}

//the signs of the components of a vector (0 counts as positive)
Vec2 gbuffer_sign_not_zero(in Vec2 value)
{
	return Vec2(value.x >= 0.0 ? 1.0 : -1.0, value.y >= 0.0 ? 1.0 : -1.0);
}

//a unit vector on the octahedron unfolded on a square ([-1, 1]^2)
Vec2 gbuffer_octahedral(in Vec3 normal)
{
	const Vec3 n = normal / (abs(normal.x) + abs(normal.y) + abs(normal.z));
	Vec2 square = n.xy;
	if (n.z < 0.0)
	{
		//the lower half folded over the corners
		square = (Vec2(1.0, 1.0) - abs(n.yx)) * gbuffer_sign_not_zero(n.xy);
	}
	return square;
}

//the second target of a normal, the roughness (or the shininess) and the shading model (the
//geometry pass)
Vec4 gbuffer_encode_normal(in Vec3 normal, in float material, in float model)
{
	return Vec4(gbuffer_octahedral(normalize(normal)), material, model);
}

//the world normal of a texel of the second target
Vec3 gbuffer_decode_normal(in Vec4 texel)
{
	Vec3 n = Vec3(texel.x, texel.y, 1.0 - abs(texel.x) - abs(texel.y));
	if (n.z < 0.0)
	{
		//the lower half unfolded
		const Vec2 folded = (Vec2(1.0, 1.0) - abs(n.yx)) * gbuffer_sign_not_zero(n.xy);
		n.x = folded.x;
		n.y = folded.y;
	}
	return normalize(n);
}

//the roughness (PBR) or the shininess (Legacy) of a texel of the second target
float gbuffer_material(in Vec4 texel)
{
	return texel.z;
}

//the shading model of a texel of the second target (0: the background)
float gbuffer_model(in Vec4 texel)
{
	return texel.w;
}
