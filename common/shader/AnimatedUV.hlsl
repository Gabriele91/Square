//
//  AnimatedUV.hlsl
//  Square
//
//  The uv of a material animated by the time of the world (camera.m_time.x, its seconds):
//   - uv_scroll: the uv moved a second (a flow: water, a waterfall, a conveyor belt);
//   - flipbook: an atlas of frames (x columns, y rows, read left to right then top to bottom),
//     z frames a second, w the frames used (0: all of them); the uv of the mesh (0 to 1: a
//     frame each, no tiling) into the frame of now.
//  Zero (not set): the uv as they are. A vertex shader computes them (once a vertex).
//
#pragma once
#include <Camera>

Vec2 uv_scroll;
Vec4 flipbook;

Vec2 animated_uv(in Vec2 uv)
{
	const float seconds = camera.m_time.x;
	//the frame of the flipbook (one frame: the whole texture)
	const float columns = max(flipbook.x, 1.0);
	const float rows = max(flipbook.y, 1.0);
	const float frames = flipbook.w > 0.0 ? flipbook.w : columns * rows;
	const float frame = flipbook.z > 0.0 ? fmod(floor(seconds * flipbook.z), frames) : 0.0;
	const Vec2  cell = Vec2(fmod(frame, columns), floor(frame / columns));
	const Vec2  framed = (uv + cell) / Vec2(columns, rows);
	//one frame: the uv as they are (their tiling kept)
	const Vec2  at = columns * rows > 1.0 ? framed : uv;
	return at + uv_scroll * seconds;
}
