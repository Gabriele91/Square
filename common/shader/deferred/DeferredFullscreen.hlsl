//
//  DeferredFullscreen.hlsl
//  Square
//
//  Shared vertex stage for the full-screen deferred light passes (ambient and
//  directional). The quad is fed already in NDC space, so the vertex shader is a
//  pass-through; the fragment recovers the screen UV from the clip position.
//
#pragma once

struct DeferredVSOutput
{
	Vec4 m_position : SV_POSITION; // clip position (pixel coords in the fragment stage)
};

DeferredVSOutput vertex(Position3D input)
{
	DeferredVSOutput output;
	output.m_position = Vec4(input.m_position, 1.0);
	return output;
}
