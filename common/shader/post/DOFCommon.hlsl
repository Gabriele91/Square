//
//  DOFCommon.hlsl
//  Square
//
//  The parts of the depth of field (see PostEffectDOF) shared by its blur (DOF) and its
//  composite (DOFComposite): its uniforms (pixels of the pass that draws: its target), the circle
//  of confusion of a pixel, the motion along x of a pixel.
//
#pragma once
#include <Camera>
#include <GBufferPosition>

Sampler2D(g_source);
Sampler2D(g_position);
Vec2 dof_size;    //pixels of the target of the pass
Vec4 dof_focus;   //start, end of the sharp range; the near range, the far range (world units)
Vec4 dof_params;  //max radius (pixels of the pass), samples, near on
Vec4 dof_motion;  //motion blur: pixels (of the pass) at its most, from, to (screen x shares)

//the circle of confusion of a world position (background: the farthest), [0, 1]
float dof_coc(in Vec4 g_pos)
{
	if (g_pos.w < 0.5) return 1.0;
	float distance = length(g_pos.xyz - camera.m_position);
	float far = saturate((distance - dof_focus.y) / dof_focus.w);
	float near = saturate((dof_focus.x - distance) / dof_focus.z) * dof_params.z;
	return max(far, near);
}

//a world position near the camera (in focus or nearer): what the motion smears
float dof_near_focus(in Vec4 g_pos)
{
	return g_pos.w > 0.5 && length(g_pos.xyz - camera.m_position) < dof_focus.y + 4.0 ? 1.0 : 0.0;
}

//the motion along x of a pixel (pixels of the pass): on what is near, growing to the right
float dof_motion_pixels(in Vec4 g_pos, in Vec2 uv)
{
	return dof_motion.x * dof_near_focus(g_pos) * smoothstep(dof_motion.y, dof_motion.z, uv.x);
}
