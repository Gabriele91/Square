//
//  Square
//
//  Created by Gabriele Di Bari on 28/09/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Square/Square.h>

//////////////////////////////////////////////////////////////////////////////////////////
//The images of the textures for the engine (--images png, the default): every image becomes a
//PNG of its channels (grey to RGB: the engine would read it as red), but
// - a PNG RGB/RGBA/palette is kept as it is (any depth);
// - a BMP with a palette (1/4/8 bit) is kept as it is: a PNG RGB would be bigger;
// - a 16 bit image in 1:5:5:5 (a BMP 5:5:5, a TGA 16), that PNG has not, a TGA with RLE
//   compression of 16 bit (the engine keeps it RGB5A1);
// - a BMP 5:6:5 (TGA has not it) a PNG RGB: 24 bit, the same colors;
// - an image the engine cannot read is kept as it is.
namespace ImageConverter
{
    struct Result
    {
        std::vector<unsigned char> m_data;
        std::string                m_description; //what was done ("BMP 8 bit -> PNG RGB")
    };

    //the image (the bytes of its file) for the engine; extension: of its file (".tga"), a TGA
    //has no magic number
    Result convert(const std::vector<unsigned char>& file, const std::string& extension);

    //a normal map of glTF (OpenGL: green up) for the engine (DirectX: green down, the shaders
    //invert it): a PNG RGB/RGBA with the green inverted; kept as it is if it cannot be decoded
    Result convert_normal_map(const std::vector<unsigned char>& file);

    //the compression of the textures in the GPU (--images bc, the default; astc)
    enum class Compression
    {
        NONE,
        BC,   //DDS: BC5 the normal maps (the shaders rebuild z), BC3 with alpha, else BC1
        ASTC  //KTX: ASTC 4x4 (Apple GPUs, mobiles; not the desktop ones)
    };

    //the extension of the files of a compression (".dds", ".ktx")
    std::string extension(Compression compression);

    //the image (a file the engine reads: the m_data of convert/convert_normal_map) compressed,
    //with its levels down to 1x1 (box filter; the normal maps normalized); m_data empty when it
    //cannot be (not decoded; BC: a size not a multiple of 4, DirectX wants it)
    Result compress(const std::vector<unsigned char>& file, bool normal_map, Compression compression);
}
