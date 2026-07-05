//
//  DeferredVolume.hlsl
//  Square
//
//  Shared vertex stage for the volume-based deferred light passes (point sphere,
//  spot cone). The light volume model matrix (orientation + scale + translation)
//  is computed on the CPU by DrawerPassDeferred and uploaded in the LightVolume
//  constant buffer, so the vertex shader is identical for both light types.
//
#pragma once

struct LightVolumeStruct
{
	Mat4 m_model;
};

cbuffer LightVolume
{
	LightVolumeStruct volume;
};

struct DeferredVSOutput
{
	Vec4 m_position : SV_POSITION; // clip position (pixel coords in the fragment stage)
};

DeferredVSOutput vertex(Position3D input)
{
	DeferredVSOutput output;
	Vec4 world_position = mul(Vec4(input.m_position, 1.0), volume.m_model);
	world_position      = mul(world_position, camera.m_view);
	output.m_position   = mul(world_position, camera.m_projection);
	return output;
}
