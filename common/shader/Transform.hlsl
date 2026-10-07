//
//  Transform.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 26/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

struct TransformStruct
{
	Mat4 m_model;
	Mat4 m_inv_model;
	Mat4 m_rotation;
	Vec3 m_position;
	Vec3 m_scale;
	float m_lod_fade; // the cross-fade of its level of detail (Render::Renderable::lod_fade)
};
cbuffer Transform
{
	TransformStruct transform;
};

// The cross-fade of the levels of detail (transform.m_lod_fade), in a fragment shader (its
// pixel: of the screen, or of a shadow map, its filter blends the pattern in a soft shadow):
// 1 drawn; t in (0,1) a level fading in, drawn on the pixels of a 4x4 ordered pattern under t;
// -t a level fading out, on the others (the two together: every pixel once)
void lod_fade_clip(Vec2 pixel)
{
	const float fade = transform.m_lod_fade;
	if (fade >= 1.0) return;
	static const float bayer[16] =
	{
		 0.0,  8.0,  2.0, 10.0,
		12.0,  4.0, 14.0,  6.0,
		 3.0, 11.0,  1.0,  9.0,
		15.0,  7.0, 13.0,  5.0
	};
	const uint2 p = uint2(pixel) & 3;
	const float threshold = (bayer[p.y * 4 + p.x] + 0.5) / 16.0;
	if (fade >= 0.0 && threshold >= fade) discard;
	if (fade < 0.0 && threshold < -fade) discard;
}

