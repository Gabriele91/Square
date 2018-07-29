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
    #define minor(a,b) determinant(float2x2(input.a, input.b))
    return Mat3(
         minor(_33_22,_23_32),
        -minor(_33_12,_13_32),
         minor(_23_12,_13_22),
        
        -minor(_33_21,_23_31),
         minor(_33_11,_13_31),
        -minor(_23_11,_13_21),
        
         minor(_32_21,_22_31),
        -minor(_32_11,_12_31),
         minor(_22_11,_12_21)
    ) / determinant(input);
    #undef minor
}

