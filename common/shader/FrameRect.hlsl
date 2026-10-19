//
//  FrameRect.hlsl
//  Square
//
//  The scene of a camera at a share of its targets (Drawer::resolution_scale: the targets as
//  large as the screen, the scene in their corner; camera.m_rect, 1 when it covers them all):
//  the uv of a point of the screen (0..1 over the scene: its ndc, the projections) and the uv
//  of the targets (their texels); a uv of a texture kept in the scene (its neighbours at its
//  edge read the edge, not what is out of it: an old frame).
//
#pragma once
#include <Camera>

//the uv of the targets of a point of the screen
Vec2 frame_target_uv(in Vec2 screen_uv)
{
	return screen_uv * camera.m_rect.xy;
}

//the uv of the screen of a uv of the targets
Vec2 frame_screen_uv(in Vec2 target_uv)
{
	return target_uv * camera.m_rect.zw;
}

//a uv of a texture of size texels kept in the scene (at most its last texel; under 0: the
//clamp of the sampler)
Vec2 frame_clamp(in Vec2 uv, in Vec2 size)
{
	return min(uv, camera.m_rect.xy - 0.5 / size);
}

//a uv of the targets in the scene
bool frame_inside(in Vec2 uv)
{
	return uv.x >= 0.0 && uv.y >= 0.0 && uv.x <= camera.m_rect.x && uv.y <= camera.m_rect.y;
}
