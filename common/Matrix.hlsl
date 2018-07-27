//
//  Matrix.hlsl.h
//  Square
//
//  Created by Gabriele Di Bari on 27/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

Mat4 inverse(in Mat4 input)
{
    #define minor(a,b,c) determinant(Mat3(input.a, input.b, input.c))
    Mat4 cofactors = Mat4(
         minor(_22_23_24, _32_33_34, _42_43_44),
        -minor(_21_23_24, _31_33_34, _41_43_44),
         minor(_21_22_24, _31_32_34, _41_42_44),
        -minor(_21_22_23, _31_32_33, _41_42_43),

        -minor(_12_13_14, _32_33_34, _42_43_44),
         minor(_11_13_14, _31_33_34, _41_43_44),
        -minor(_11_12_14, _31_32_34, _41_42_44),
         minor(_11_12_13, _31_32_33, _41_42_43),

         minor(_12_13_14, _22_23_24, _42_43_44),
        -minor(_11_13_14, _21_23_24, _41_43_44),
         minor(_11_12_14, _21_22_24, _41_42_44),
        -minor(_11_12_13, _21_22_23, _41_42_43),

        -minor(_12_13_14, _22_23_24, _32_33_34),
         minor(_11_13_14, _21_23_24, _31_33_34),
        -minor(_11_12_14, _21_22_24, _31_32_34),
         minor(_11_12_13, _21_22_23, _31_32_33)
    );
    #undef minor
    return transpose(cofactors) / determinant(input);
}

Mat3 inverse(in Mat3 input)
{
    float a00 = input[0][0], a01 = input[0][1], a02 = input[0][2];
    float a10 = input[1][0], a11 = input[1][1], a12 = input[1][2];
    float a20 = input[2][0], a21 = input[2][1], a22 = input[2][2];

    return Mat3( (a22 * a11 - a12 * a21), (-a22 * a01 + a02 * a21),  (a12 * a01 - a02 * a11),
                (-a22 * a10 + a12 * a20),  (a22 * a00 - a02 * a20), (-a12 * a00 + a02 * a10),
                 (a21 * a10 - a11 * a20), (-a21 * a00 + a01 * a20),  (a11 * a00 - a01 * a10)) / determinant(input);
}
