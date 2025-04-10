//
//  Support.hlsl
//  Square
//
//  Created by Gabriele Di Bari on 26/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Matrix>

Vec2 projection_to_screen(in Vec4 pos)
{
    Vec2 screen_pos = pos.xy / pos.w;
    return (screen_pos.xy + Vec2(1.0,1.0)) * 0.5;
}

Mat3 compute_normal_matrix(in Mat4 model)
{
    Mat3 m3x3 = (Mat3)(model);
    return transpose(inverse(m3x3));
}

Mat3 compute_tbn(in Mat3 normal_matrix,
                 in Vec3 normal,
                 in Vec3 tangent,
                 in Vec3 bitangents)
{
    //compute T/B/N
    Vec3 t_pixel = normalize(mul(normal_matrix,tangent));
    Vec3 b_pixel = normalize(mul(normal_matrix,bitangents));
    Vec3 n_pixel = normalize(mul(normal_matrix,normal));
    //as matrix
    return Mat3(t_pixel, b_pixel, n_pixel);
}

Vec3 normal_from_texture(in Vec4 normal_color)
{
	//inv x axis (todo:try)
	normal_color.x = 1.0f - normal_color.x;
	//return
	return normalize(normal_color.rgb * 2.0f - 1.0f);
}

//Compute position
Vec4 mul_model_view_projection(in Vec4 position)
{
	position = mul(position, transform.m_model);
	position = mul(position, camera.m_view);
	position = mul(position, camera.m_projection);
	return position;
}

Vec4 mul_view_projection(in Vec4 position)
{
	position = mul(position, camera.m_view);
	position = mul(position, camera.m_projection);
	return position;
}

Vec4 mul_model(in Vec4 position)
{
	position = mul(position, transform.m_model);
	return position;
}


Vec4 mul_model_view_projection(in Vec3 vertex)
{
	return mul_model_view_projection(Vec4(vertex, 1.0));
}

Vec4 mul_view_projection(in Vec3 vertex)
{
	return mul_view_projection(Vec4(vertex, 1.0));
}

Vec4 mul_model(in Vec3 vertex)
{
	return mul_model(Vec4(vertex, 1.0));
}