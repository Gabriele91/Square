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
#include "GLTFReader.h"
#include "ImageConverter.h"
#include "UniqueNames.h"

//////////////////////////////////////////////////////////////////////////////////////////
//The textures of a glTF: a .sqtex per texture (its sampler and its image). The images:
// - convert (--images png, the default): in the .sqtex ("data"), converted by ImageConverter;
// - else: the image files copied next to the .sqtex ("url"), the embedded ones in it ("data").
class TextureManager
{
    struct TextureBufferDescription
    {
        std::string m_name;
        size_t m_index;
    };
    //an image file to convert (--images png)
    struct ImageFile
    {
        std::string m_path;
    };
    using TextureType = std::variant< std::string, TextureBufferDescription, ImageFile >;

    Square::Context& m_context;
    std::string m_output;
    bool m_convert{ true };                   //--images png: the images converted (see ImageConverter)
    std::vector< TextureType > m_images;
    std::vector< std::string > m_image_names; //wanted name of the textures made from each image
    std::vector< std::string > m_samplers;
    std::vector< std::string > m_textures;
    UniqueNames                m_names;       //texture resources: .sqtex and copied images
    std::unordered_map< size_t, ImageConverter::Result > m_converted; //image id -> its conversion

public:
    TextureManager(Square::Context& context, const std::string& output);

    //convert: the images for the engine (PNG, TGA RLE), else as they are
    TextureManager(Square::Context& context, const std::string& output, const Square::Data::GLTF::GLTF& gltf, bool convert = true);

    size_t add_image(const Square::Data::GLTF::Image& in_image, const std::string& gltfpath);

    size_t add_sampler(const Square::Data::GLTF::Sampler& sampler);

    size_t add_texture(const Square::Data::GLTF::Texture& texture, const Square::Data::GLTF::Views& views, const Square::Data::GLTF::Buffers& buffers);

    std::optional<std::string> at(size_t index) const;

private:

    //the bytes of an image (a file, or a buffer view of the glTF); empty if missing
    std::vector<unsigned char> image_bytes(const TextureType& in_image, const Square::Data::GLTF::Views& views, const Square::Data::GLTF::Buffers& buffers) const;

    size_t add_texture_internal(size_t image_id, const std::string& sampler, const Square::Data::GLTF::Views& views, const Square::Data::GLTF::Buffers& buffers);
};
