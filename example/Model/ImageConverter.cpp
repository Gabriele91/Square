//
//  Square
//
//  Created by Gabriele Di Bari on 28/09/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include "ImageConverter.h"
#include <cstdlib>

namespace ImageConverter
{
    namespace
    {
        unsigned int u16_le(const std::vector<unsigned char>& d, size_t at) { return at + 2 <= d.size() ? unsigned(d[at]) | (unsigned(d[at + 1]) << 8) : 0; }
        unsigned int u32_le(const std::vector<unsigned char>& d, size_t at) { return at + 4 <= d.size() ? u16_le(d, at) | (u16_le(d, at + 2) << 16) : 0; }

        bool is_png(const std::vector<unsigned char>& d)  { return d.size() >= 8 && d[0] == 0x89 && d[1] == 'P' && d[2] == 'N' && d[3] == 'G'; }
        bool is_jpeg(const std::vector<unsigned char>& d) { return d.size() >= 3 && d[0] == 0xFF && d[1] == 0xD8 && d[2] == 0xFF; }
        bool is_bmp(const std::vector<unsigned char>& d)  { return d.size() >= 54 && d[0] == 'B' && d[1] == 'M'; }
        //TGA: no magic; its extension, and a plausible header (color map 0/1, a TGA image type)
        bool is_tga(const std::vector<unsigned char>& d, const std::string& extension)
        {
            if (d.size() < 18 || d[1] > 1) return false;
            const unsigned char type = d[2];
            const bool tga_type = type == 1 || type == 2 || type == 3 || type == 9 || type == 10 || type == 11;
            return tga_type && Square::case_insensitive_equal(extension, ".tga");
        }

        //decoded by the engine (PNG, JPEG, BMP, TGA 2.0), then a PNG of 3/4 channels
        bool to_png(const std::vector<unsigned char>& file, Result& result, const std::string& what)
        {
            std::vector<unsigned char> pixels;
            unsigned long width = 0, height = 0;
            Square::Render::TextureFormat format;
            Square::Render::TextureType type;
            if (!Square::Data::Image::load(file, pixels, width, height, format, type)) return false;
            unsigned int channels = 0;
            switch (format)
            {
            case Square::Render::TF_R8:    channels = 1; break;
            case Square::Render::TF_RG8:   channels = 2; break;
            case Square::Render::TF_RGB8:  channels = 3; break;
            case Square::Render::TF_RGBA8: channels = 4; break;
            default: return false;
            }
            //grey: RGB (grey alpha: RGBA)
            if (channels <= 2)
            {
                const unsigned int out_channels = channels + 2;
                std::vector<unsigned char> expanded(size_t(width) * height * out_channels);
                for (size_t i = 0, n = size_t(width) * height; i != n; ++i)
                {
                    const unsigned char grey = pixels[i * channels];
                    expanded[i * out_channels + 0] = grey;
                    expanded[i * out_channels + 1] = grey;
                    expanded[i * out_channels + 2] = grey;
                    if (channels == 2) expanded[i * out_channels + 3] = pixels[i * channels + 1];
                }
                pixels = std::move(expanded);
                channels = out_channels;
            }
            result.m_data = Square::Data::Image::encode_png(pixels.data(), width, height, channels);
            result.m_description = what + " -> PNG " + (channels == 4 ? "RGBA" : "RGB");
            return !result.m_data.empty();
        }

        //the pixels of a BMP of 16 bit (u16), rows from the top
        bool read_bmp16(const std::vector<unsigned char>& file, std::vector<unsigned int>& pixels, unsigned long& width, unsigned long& height)
        {
            const unsigned int offset = u32_le(file, 10);
            const int bmp_width  = int(u32_le(file, 18));
            const int bmp_height = int(u32_le(file, 22));
            if (bmp_width <= 0 || bmp_height == 0) return false;
            width  = (unsigned long)bmp_width;
            height = (unsigned long)std::abs(bmp_height);
            const size_t stride = ((size_t(width) * 2 + 3) / 4) * 4;
            if (offset + stride * height > file.size()) return false;
            pixels.resize(size_t(width) * height);
            for (unsigned long y = 0; y != height; ++y)
            {
                //the rows of a BMP are from the bottom, unless the height is negative
                const unsigned long file_y = bmp_height < 0 ? y : height - 1 - y;
                for (unsigned long x = 0; x != width; ++x)
                    pixels[size_t(y) * width + x] = u16_le(file, offset + stride * file_y + size_t(x) * 2);
            }
            return true;
        }

        //5 or 6 bits to 8 (the high bits repeated in the low ones: 31 is 255, 63 is 255)
        unsigned char expand5(unsigned int v) { return (unsigned char)((v << 3) | (v >> 2)); }
        unsigned char expand6(unsigned int v) { return (unsigned char)((v << 2) | (v >> 4)); }

        //a BMP of 16 bit 5:6:5 to a PNG RGB (stb_image decodes its green wrong)
        bool bmp565_to_png(const std::vector<unsigned char>& file, Result& result)
        {
            std::vector<unsigned int> pixels;
            unsigned long width = 0, height = 0;
            if (!read_bmp16(file, pixels, width, height)) return false;
            std::vector<unsigned char> rgb(pixels.size() * 3);
            for (size_t i = 0; i != pixels.size(); ++i)
            {
                rgb[i * 3 + 0] = expand5((pixels[i] >> 11) & 0x1F);
                rgb[i * 3 + 1] = expand6((pixels[i] >> 5) & 0x3F);
                rgb[i * 3 + 2] = expand5(pixels[i] & 0x1F);
            }
            result.m_data = Square::Data::Image::encode_png(rgb.data(), width, height, 3);
            result.m_description = "BMP 16 bit 5:6:5 -> PNG RGB";
            return !result.m_data.empty();
        }

        //a BMP of 16 bit 1:5:5:5 (x:5:5:5 or a1:5:5:5) to a TGA 16 RLE (the same pixels)
        bool bmp555_to_tga(const std::vector<unsigned char>& file, bool has_alpha, Result& result)
        {
            std::vector<unsigned int> pixels;
            unsigned long width = 0, height = 0;
            if (!read_bmp16(file, pixels, width, height)) return false;
            std::vector<unsigned char> tga(pixels.size() * 2);
            for (size_t i = 0; i != pixels.size(); ++i)
            {
                const unsigned int pixel = has_alpha ? pixels[i] : (pixels[i] | 0x8000); //else opaque
                tga[i * 2 + 0] = (unsigned char)(pixel);
                tga[i * 2 + 1] = (unsigned char)(pixel >> 8);
            }
            result.m_data = Square::Data::Image::encode_tga_rle(tga.data(), width, height, 2, true);
            result.m_description = std::string("BMP 16 bit ") + (has_alpha ? "1:5:5:5" : "5:5:5") + " -> TGA 16 bit RLE";
            return !result.m_data.empty();
        }

        //a TGA of 16 bit without compression to a TGA 16 RLE (the same pixels)
        bool tga16_to_rle(const std::vector<unsigned char>& file, Result& result)
        {
            const size_t id_size = file[0];
            const size_t color_map_size = file[1] ? size_t(u16_le(file, 5)) * ((file[7] + 7) / 8) : 0;
            const unsigned long width  = u16_le(file, 12);
            const unsigned long height = u16_le(file, 14);
            const unsigned char descriptor = file[17];
            const size_t offset = 18 + id_size + color_map_size;
            if (!width || !height || offset + size_t(width) * height * 2 > file.size()) return false;
            std::vector<unsigned char> pixels(file.begin() + offset, file.begin() + offset + size_t(width) * height * 2);
            //no alpha bit (attribute bits 0): opaque
            if (!(descriptor & 0x0F))
            {
                for (size_t i = 1; i < pixels.size(); i += 2) pixels[i] |= 0x80;
            }
            result.m_data = Square::Data::Image::encode_tga_rle(pixels.data(), width, height, 2, (descriptor & 0x20) != 0);
            result.m_description = "TGA 16 bit -> TGA 16 bit RLE";
            return !result.m_data.empty();
        }
    }

    Result convert(const std::vector<unsigned char>& file, const std::string& extension)
    {
        Result result;
        if (is_png(file))
        {
            //grey (color type 0, 4 at 25): RGB, RGBA
            if ((file.size() > 25 && (file[25] == 0 || file[25] == 4)) && to_png(file, result, "PNG grey")) return result;
            result.m_data = file;
            result.m_description = "PNG, kept";
            return result;
        }
        if (is_jpeg(file))
        {
            if (to_png(file, result, "JPEG")) return result;
        }
        else if (is_bmp(file))
        {
            const unsigned int bits = u16_le(file, 28);
            const unsigned int compression = u32_le(file, 30);
            const unsigned int header_size = u32_le(file, 14);
            if (bits == 16)
            {
                //masks: BI_RGB is 5:5:5; BI_BITFIELDS/BI_ALPHABITFIELDS after the header
                unsigned int green = 0x03E0, alpha = 0;
                if (compression == 3 || compression == 6)
                {
                    green = u32_le(file, 58);
                    if (compression == 6 || header_size >= 56) alpha = u32_le(file, 66);
                }
                if (green == 0x07E0)
                {
                    if (bmp565_to_png(file, result)) return result;
                }
                else if (bmp555_to_tga(file, alpha == 0x8000, result))
                {
                    return result;
                }
            }
            //palette (1, 4, 8 bit): 1 byte per pixel at most, a PNG RGB would be bigger
            else if (bits <= 8)
            {
                result.m_data = file;
                result.m_description = "BMP " + std::to_string(bits) + " bit palette, kept";
                return result;
            }
            else if (to_png(file, result, "BMP " + std::to_string(bits) + " bit"))
            {
                return result;
            }
        }
        else if (is_tga(file, extension))
        {
            const unsigned int bits = file[16];
            if (bits == 16)
            {
                //RLE already: as it is
                if (file[2] == 10)
                {
                    result.m_data = file;
                    result.m_description = "TGA 16 bit RLE, kept";
                    return result;
                }
                if (file[2] == 2 && tga16_to_rle(file, result)) return result;
            }
            if (to_png(file, result, "TGA " + std::to_string(bits) + " bit")) return result;
        }
        //as it is
        result.m_data = file;
        result.m_description = "kept as it is (not converted)";
        return result;
    }

    Result convert_normal_map(const std::vector<unsigned char>& file)
    {
        Result result;
        std::vector<unsigned char> pixels;
        unsigned long width = 0, height = 0;
        Square::Render::TextureFormat format;
        Square::Render::TextureType type;
        const bool decoded = Square::Data::Image::load(file, pixels, width, height, format, type);
        const unsigned int channels = !decoded ? 0
                                    : format == Square::Render::TF_RGB8 ? 3
                                    : format == Square::Render::TF_RGBA8 ? 4 : 0;
        if (!channels)
        {
            result.m_data = file;
            result.m_description = "normal map kept as it is (not RGB/RGBA: green not inverted)";
            return result;
        }
        //OpenGL (green up) to DirectX (green down)
        for (size_t i = 1; i < pixels.size(); i += channels)
        {
            pixels[i] = 255 - pixels[i];
        }
        result.m_data = Square::Data::Image::encode_png(pixels.data(), width, height, channels);
        result.m_description = std::string("normal map OpenGL -> DirectX (green inverted), PNG ") + (channels == 4 ? "RGBA" : "RGB");
        return result;
    }
}
