//
//  DeferredVelocity.hlsl
//  Square
//
//  The velocity pass of the deferred pipeline (see DrawerPassDeferred::velocity_pass): the
//  renderables with their own motion blur (Renderable::motion_blur) drawn again on the G-Buffer
//  depth (the same projection: the same depth, what is in front hides them), each pixel its
//  motion on the screen (uv) from where it was in the last frame (the model and the camera of
//  then) to where it is now. The others stay 0 (cleared).
//
#include <Camera>
#include <Transform>
#include <Vertex>
#include <Support>

struct VelocityStruct
{
	Mat4 m_previous_model;
	Mat4 m_previous_view;
	Mat4 m_previous_projection;
};

cbuffer Velocity
{
	VelocityStruct velocity;
};

struct VelocityVSOutput
{
	Vec4 m_position : SV_POSITION; // clip position
	Vec4 m_current  : TEXCOORD0;   // clip position now
	Vec4 m_previous : TEXCOORD1;   // clip position in the last frame
};

//uv of the screen of a clip position (the rows go down on D3D/Metal, up on OpenGL)
Vec2 velocity_clip_to_uv(in Vec4 clip)
{
	Vec2 ndc = clip.xy / max(abs(clip.w), 0.00001);
#ifdef SQ_BACKEND_GLSL
	return Vec2(0.5 + 0.5 * ndc.x, 0.5 + 0.5 * ndc.y);
#else
	return Vec2(0.5 + 0.5 * ndc.x, 0.5 - 0.5 * ndc.y);
#endif
}

VelocityVSOutput vertex(Position3DNormalTangetBinomialUV input)
{
	VelocityVSOutput output;
	//now: as the G-Buffer (the same depth)
	output.m_position = mul_view_projection(mul_model(input.m_position).xyz);
	output.m_current  = output.m_position;
	//then
	Vec4 previous = mul(Vec4(input.m_position, 1.0), velocity.m_previous_model);
	previous = mul(previous, velocity.m_previous_view);
	output.m_previous = mul(previous, velocity.m_previous_projection);
	return output;
}

Vec4 fragment(VelocityVSOutput input) : SV_TARGET0
{
	//behind the camera then: no motion known
	if (input.m_previous.w <= 0.0) return Vec4(0.0, 0.0, 0.0, 0.0);
	return Vec4(velocity_clip_to_uv(input.m_current) - velocity_clip_to_uv(input.m_previous), 0.0, 0.0);
}
