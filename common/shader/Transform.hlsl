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
};
cbuffer Transform
{
	TransformStruct transform;
};

