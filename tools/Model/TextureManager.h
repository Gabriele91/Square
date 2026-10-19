//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Square/Square.h>
#include <variant>
#include <optional>
#include <map>
#include <unordered_set>
#include "GLTFReader.h"
#include "ImageConverter.h"
#include "UniqueNames.h"

//////////////////////////////////////////////////////////////////////////////////////////
//The textures of a glTF: a .sqtex per texture (its sampler and its image). The images:
// - convert (--images png, the default): in the .sqtex ("data"), converted by ImageConverter;
// - else: the image files copied next to the .sqtex ("url"), the embedded ones in it ("data").
//The normal maps (the normalTexture of the materials) are OpenGL in glTF (green up), the engine
//reads them DirectX (green down): always in the .sqtex, a PNG with the green inverted.
//Compressed (--images bc, the default; astc): the converted image compressed with its levels
//(ImageConverter::compress) in a file next to the .sqtex ("url"), the converted one too for a
//GPU without the format ("fallback"); an image that cannot be compressed is as with png.
class TextureManager
{
    struct TextureBufferDescription
    {
        Square::String m_name;
        size_t m_index;
    };
    //an image file to convert (--images png)
    struct ImageFile
    {
        Square::String m_path;
    };
    using TextureType = std::variant< Square::String, TextureBufferDescription, ImageFile >;

    Square::Context& m_context;
    Square::String m_output;
    bool m_convert{ true };                   //--images png: the images converted (see ImageConverter)
    Square::Vector< TextureType > m_images;
    Square::Vector< Square::String > m_image_names; //wanted name of the textures made from each image
    Square::Vector< Square::String > m_samplers;
    Square::Vector< Square::String > m_textures;
    UniqueNames                m_names;       //texture resources: .sqtex and copied images
    Square::UnorderedSet< size_t > m_normal_maps; //glTF textures used as normal map
    size_t                     m_next_texture{ 0 }; //glTF index of the next add_texture
    Square::Map< std::pair<size_t, bool>, ImageConverter::Result > m_converted; //(image id, normal map) -> its conversion
    ImageConverter::Compression m_compression{ ImageConverter::Compression::NONE }; //of the converted images
    Square::Map< std::pair<size_t, bool>, Square::String > m_compressed; //(image id, normal map) -> the "url"/"fallback" lines of its files ("": not compressed)

public:
    TextureManager(Square::Context& context, const Square::String& output);

    //convert: the images for the engine (PNG, TGA RLE), else as they are; compression: of the
    //converted images (DDS BC, KTX ASTC)
    TextureManager(Square::Context& context, const Square::String& output, const Square::Data::GLTF::GLTF& gltf, bool convert = true, ImageConverter::Compression compression = ImageConverter::Compression::NONE);

    size_t add_image(const Square::Data::GLTF::Image& in_image, const Square::String& gltfpath);

    size_t add_sampler(const Square::Data::GLTF::Sampler& sampler);

    size_t add_texture(const Square::Data::GLTF::Texture& texture, const Square::Data::GLTF::Views& views, const Square::Data::GLTF::Buffers& buffers);

    std::optional<Square::String> at(size_t index) const;

private:

    //the bytes of an image (a file, or a buffer view of the glTF); empty if missing
    Square::Vector<unsigned char> image_bytes(const TextureType& in_image, const Square::Data::GLTF::Views& views, const Square::Data::GLTF::Buffers& buffers) const;

    size_t add_texture_internal(size_t image_id, bool normal_map, const Square::String& sampler, const Square::Data::GLTF::Views& views, const Square::Data::GLTF::Buffers& buffers);

    //the converted image compressed, and itself as fallback, in files next to the textures: the
    //"url"/"fallback" lines of a .sqtex ("" when it cannot be compressed)
    Square::String compress_image(size_t image_id, bool normal_map, const Square::String& texture_name, const Square::Vector<unsigned char>& converted);
    //the KTX (ASTC) of an image next to its DDS (its name without extension): Compression::ALL
    bool compress_mobile_image(size_t image_id, bool normal_map, const Square::String& image_name, const Square::Vector<unsigned char>& converted);
};
