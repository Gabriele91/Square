//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "TextureManager.h"
#include <cstdio>

namespace
{
    const char* texture_filter_to_string(const Square::Data::GLTF::TextureFilter filter)
    {
        switch (filter)
        {
            case Square::Data::GLTF::TextureFilter::NEAREST: return "nearest";
            case Square::Data::GLTF::TextureFilter::LINEAR: return "linear";
            case Square::Data::GLTF::TextureFilter::NEAREST_MIPMAP_NEAREST: return "nearest_mipmap_nearest";
            case Square::Data::GLTF::TextureFilter::LINEAR_MIPMAP_NEAREST: return "linear_mipmap_nearest";
            case Square::Data::GLTF::TextureFilter::NEAREST_MIPMAP_LINEAR: return "nearest_mipmap_linear";
            case Square::Data::GLTF::TextureFilter::LINEAR_MIPMAP_LINEAR: return "linear_mipmap_linear";
            default: return "unknown";
        }
    }

    const char* texture_wrap_mode_to_string(const Square::Data::GLTF::TextureWrapMode mode)
    {
        switch (mode)
        {
        case Square::Data::GLTF::TextureWrapMode::CLAMP_TO_EDGE: return "clamp";
        case Square::Data::GLTF::TextureWrapMode::MIRRORED_REPEAT: return "mirrored_repeat";
        case Square::Data::GLTF::TextureWrapMode::REPEAT: return "repeat";
        default: return "unknown";
        }
    }
}

TextureManager::TextureManager(Square::Context& context, const std::string& output)
: m_context(context)
, m_output(output)
{}

TextureManager::TextureManager(Square::Context& context, const std::string& output, const Square::Data::GLTF::GLTF& gltf, bool convert)
: m_context(context)
, m_output(output)
, m_convert(convert)
{
    for (auto& image : gltf.images)
    {
        add_image(image, gltf.path);
    }
    for (auto& sempler : gltf.samplers)
    {
        add_sampler(sempler);
    }
    for (auto& texture : gltf.textures)
    {
        add_texture(texture, gltf.views, gltf.buffers);
    }
}

size_t TextureManager::add_image(const Square::Data::GLTF::Image& in_image, const std::string& gltfpath)
{
    if (std::holds_alternative<Square::Data::GLTF::ImagePath>(in_image))
    {
        const Square::Data::GLTF::ImagePath& image_path = std::get<Square::Data::GLTF::ImagePath>(in_image);
        const std::string image_name = Square::Filesystem::get_basename(image_path.uri);
        // Real image path
        auto gltfwd = Square::Filesystem::get_directory(gltfpath);
        auto imagepath = Square::Filesystem::join(gltfwd, image_path.uri);
        // Converted: read when its texture is written
        if (m_convert)
        {
            m_images.push_back(ImageFile{ imagepath });
            m_image_names.push_back(image_name);
            return m_images.size();
        }
        // The copy is a texture resource too (.png/.jpg...): "_img" keeps it apart
        // from the .sqtex named after the image
        auto newname = m_names.make(image_name + "_img", "image_img")
                     + Square::Filesystem::get_extension(image_path.uri);
        auto outputpath = Square::Filesystem::join(m_output, newname);
        // Copy
        if (!Square::Filesystem::copyfile(imagepath, outputpath))
        {
            m_context.logger()->warning("unable to copy: " + image_path.uri);
        }
        m_images.push_back(outputpath);
        m_image_names.push_back(image_name);
        return m_images.size();
    }
    else if (std::holds_alternative<Square::Data::GLTF::ImageBuffer>(in_image))
    {
        const Square::Data::GLTF::ImageBuffer& buffer_description = std::get<Square::Data::GLTF::ImageBuffer>(in_image);
        m_images.push_back(TextureBufferDescription{ buffer_description.name, buffer_description.buffer_view });
        m_image_names.push_back(buffer_description.name);
        return m_images.size();
    }

    m_context.logger()->warning("Invalid image");
    return ~size_t(0);
}

size_t TextureManager::add_sampler(const Square::Data::GLTF::Sampler& sampler)
{
    const char _template[] =
    {
        "mag_filter %s\n"
        "min_filter %s\n"
        "wrap_s %s\n"
        "wrap_t %s\n"
        "wrap_r %s\n"
    };
    char output_template[255] = { '\0' };
    std::snprintf(&output_template[0], 255, _template,
        texture_filter_to_string(sampler.mag_filter),
        texture_filter_to_string(sampler.min_filter),
        texture_wrap_mode_to_string(sampler.wrap_s),
        texture_wrap_mode_to_string(sampler.wrap_t),
        texture_wrap_mode_to_string(sampler.wrap_r)
    );
    m_samplers.push_back(output_template);
    return m_samplers.size();
}

size_t TextureManager::add_texture(const Square::Data::GLTF::Texture& texture, const Square::Data::GLTF::Views& views, const Square::Data::GLTF::Buffers& buffers)
{
    if (texture.sampler.has_value() && texture.sampler < m_samplers.size() && texture.source < m_images.size())
    {
        const auto& sampler = m_samplers[texture.sampler.value()];
        return add_texture_internal(texture.source, sampler, views, buffers);
    }
    else if (!texture.sampler.has_value() && texture.source < m_images.size())
    {
        const std::string sampler = "mag_filter linear\n"
                                     "min_filter linear_mipmap_linear\n"
                                     "wrap_s repeat\n"
                                     "wrap_t repeat\n"
                                     "wrap_r repeat\n";
        return add_texture_internal(texture.source, sampler, views, buffers);
    }
    // Output
    if (texture.sampler.has_value())
    {
        m_context.logger()->warning("Invalid texture(" +  std::to_string(texture.sampler.value()) + ", " +  std::to_string(texture.source) + ")");
    }
    else
    {
        m_context.logger()->warning("Invalid texture(NONE, " +  std::to_string(texture.source) + ")");
    }
    // Return invalid id
    return ~size_t(0);
}

std::optional<std::string> TextureManager::at(size_t index) const
{
    std::optional<std::string> texture { };
    if (index < m_textures.size())
    {
        return Square::Filesystem::get_basename(m_textures[index]);
    }
    return texture;
}

std::vector<unsigned char> TextureManager::image_bytes(const TextureType& in_image, const Square::Data::GLTF::Views& views, const Square::Data::GLTF::Buffers& buffers) const
{
    if (std::holds_alternative<ImageFile>(in_image))
    {
        return Square::Filesystem::binary_file_read_all(std::get<ImageFile>(in_image).m_path);
    }
    if (std::holds_alternative<TextureBufferDescription>(in_image))
    {
        const TextureBufferDescription& description = std::get<TextureBufferDescription>(in_image);
        if (description.m_index >= views.size()) return {};
        const auto& view = views[description.m_index];
        const auto& buffer = buffers[view.buffer];
        if (view.offset + view.length > buffer.size()) return {};
        return std::vector<unsigned char>(buffer.begin() + view.offset, buffer.begin() + view.offset + view.length);
    }
    return {};
}

size_t TextureManager::add_texture_internal(size_t image_id, const std::string& sampler, const Square::Data::GLTF::Views& views, const Square::Data::GLTF::Buffers& buffers)
{
    const TextureType& in_image = m_images[image_id];
    //named after its image (a second texture of the same image gets "_2")
    const std::string texture_name = m_names.make(m_image_names[image_id], "texture" + std::to_string(m_textures.size()));
    //a copied image: the texture refers to it
    if (std::holds_alternative<std::string>(in_image))
    {
        std::string image_uri = std::get<std::string>(in_image);
        std::string texture_sampler_body = sampler + "url " + Square::Filesystem::get_filename(image_uri) + "\n";
        std::string texture_sampler_path = Square::Filesystem::join(m_output, texture_name + ".sqtex");

        Square::Filesystem::text_file_write_all(texture_sampler_path, texture_sampler_body);
        m_textures.push_back(texture_sampler_path);
        return m_textures.size();
    }
    //the image in the texture: "data" and its bytes (converted once per image)
    std::vector<unsigned char> bytes = image_bytes(in_image, views, buffers);
    if (bytes.empty())
    {
        m_context.logger()->warning("unable to read the image of: " + texture_name);
        return 0;
    }
    if (m_convert)
    {
        auto converted = m_converted.find(image_id);
        if (converted == m_converted.end())
        {
            const std::string extension = std::holds_alternative<ImageFile>(in_image)
                                        ? Square::Filesystem::get_extension(std::get<ImageFile>(in_image).m_path)
                                        : std::string();
            converted = m_converted.insert({ image_id, ImageConverter::convert(bytes, extension) }).first;
            m_context.logger()->info("image " + m_image_names[image_id] + ": " + converted->second.m_description);
        }
        bytes = converted->second.m_data;
    }
    std::string texture_path = Square::Filesystem::join(m_output, texture_name + ".sqtex");
    std::string texture_body = sampler + "data";
    FILE* texture_pfile = std::fopen(texture_path.c_str(), "wb");
    if (!texture_pfile)
    {
        m_context.logger()->warning("unable to write: " + texture_path);
        return 0;
    }
    std::fwrite(texture_body.data(), texture_body.size(), 1, texture_pfile);
    std::fwrite(bytes.data(), bytes.size(), 1, texture_pfile);
    std::fclose(texture_pfile);
    m_textures.push_back(texture_path);
    return m_textures.size();
}
