//
//  Square
//
//  Created by Gabriele on 09/09/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Core/Filesystem.h"
#include "Square/Driver/Render.h"
#include "Square/Data/Image.h"

extern "C"
{
    unsigned char* stbi_load_from_memory
    (
     unsigned char const *buffer,
     int len,
     int *x,
     int *y,
     int *comp,
     int req_comp
     );
    void stbi_image_free(void* image);
    void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);
}

namespace Square
{
namespace Data
{
	//TGA
	extern bool decode_tga
	(
		std::vector<unsigned char>& out_image,
		unsigned long&  image_width,
		unsigned long&  image_height,
		Render::TextureFormat& image_format,
		Render::TextureType&   image_type,
		const unsigned char* in_tga,
		size_t in_size
	);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	static uint32_t to_big_endian(uint32_t value) 
	{
		uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
		return (static_cast<uint32_t>(bytes[0]) << 24) |
			   (static_cast<uint32_t>(bytes[1]) << 16) |
			   (static_cast<uint32_t>(bytes[2]) << 8) |
			   (static_cast<uint32_t>(bytes[3]));
	}
#endif


	Image::ImageTypeFormat Image::get_image_type_format_from_ext(const std::string& path)
	{
		//get ext
		std::string ext = Filesystem::get_extension(path);
		// Test type
		if (ext == ".png")					      return Image::ImageTypeFormat::ITF_PNG;
		else if (ext == ".jpg" || ext == ".jpge") return Image::ImageTypeFormat::ITF_JPG;
		else if (ext == ".bmp")					  return Image::ImageTypeFormat::ITF_BMP;
		else if (ext == ".tga")					  return Image::ImageTypeFormat::ITF_TGA;
		return  Image::ImageTypeFormat::ITF_UNKNOWN;
	}

	Image::ImageTypeFormat Image::get_image_type_format_from_data(const std::vector<unsigned char>& in_data)
	{
		if (in_data.size() < sizeof(int)) return Image::ImageTypeFormat::ITF_UNKNOWN;

		// Get magic
		unsigned int magic = 0;
		std::memcpy(&magic, in_data.data(), sizeof(magic));
		#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			magic = to_big_endian(magic);
		#endif


		// PNG magic number (exact match)
		if (magic == 0x89504E47) return Image::ImageTypeFormat::ITF_PNG;

		// JPEG magic number (first 3 bytes match)
		else if ((magic & 0xFFFFFF00) == 0xFFD8FF00) return Image::ImageTypeFormat::ITF_JPG;

		// BMP magic number (first 2 bytes match)
		else if ((magic & 0xFFFF0000) == 0x424D0000) return Image::ImageTypeFormat::ITF_BMP;

		// TGA detection requires more sophisticated approach
		// TGA header typically starts with a variable-length image ID
		// Most TGA files have first byte as image ID length (0-255)
		else if ((magic & 0x000000FF) <= 0xFF &&  // Image ID length valid
			    ((magic & 0x0000FF00) >> 8) <= 1)  // Color map type (0 or 1)
			 {
				 // Check footer with null terminator included in length
			     const char footer[] = "TRUEVISION-XFILE."; // NULL included
				 size_t footer_len = SqareArrayLangth(footer);
				 if ((footer_len + sizeof(int)) < in_data.size() 
				 && std::memcmp(in_data.data() + in_data.size() - footer_len, footer, footer_len) == 0)
				 {
					 return Image::ImageTypeFormat::ITF_TGA;
				 }
			 }
		
		return  Image::ImageTypeFormat::ITF_UNKNOWN;
	}

	//load image
	Shared<Image> Image::load(Allocator* allocator,const std::string& path)
	{
		
		std::vector<unsigned char> image_data;
		unsigned long image_width { 0 };
		unsigned long image_height{ 0 };
		Render::TextureFormat image_format;
		Render::TextureType image_type;
		// load
		if (!load(path, image_data, image_width, image_height, image_format, image_type))
		{
			return nullptr;
		}
		// Convert 
		Shared<Image> o_image = nullptr;
		//types
		switch (image_format)
		{
			case Render::TF_R8:     o_image = from_r(allocator, image_data.data(), image_width, image_height);        break;
			case Render::TF_RGB565: o_image = from_rgb565(allocator, image_data.data(), image_width, image_height);   break;
			case Render::TF_RGB5A1: o_image = from_rgb5a1(allocator, image_data.data(), image_width, image_height);   break;
			case Render::TF_RGB8:   o_image = from_rgb(allocator, image_data.data(), image_width, image_height);      break;
			case Render::TF_RGBA8:  o_image = from_rgba(allocator, image_data.data(), image_width, image_height);     break;
			default: break;
		}
		return o_image;
	}

	//load raw
	bool Image::load
	(
		const std::string& path,
		std::vector<unsigned char>& image_data,
		unsigned long&  image_width,
		unsigned long&  image_height,
		Render::TextureFormat& image_format,
		Render::TextureType&  image_type
	)
	{
		// get type
		ImageTypeFormat image_type_format = get_image_type_format_from_ext(path);
		// test
		if (image_type_format == ImageTypeFormat::ITF_UNKNOWN) return false;
		// read all file
		std::vector<unsigned char> data_file = Filesystem::binary_file_read_all(path);
		//fail to read?
		if (!data_file.size()) return false;
		// load
		return load(image_type_format, data_file, image_data, image_width, image_height, image_format, image_type);
	}

	//load raw
	bool Image::load
	(
		const std::vector<unsigned char>& data_file,
		std::vector<unsigned char>& image_data,
		unsigned long&  image_width,
		unsigned long&  image_height,
		Render::TextureFormat& image_format,
		Render::TextureType& image_type
	)
	{
		// get type
		ImageTypeFormat image_type_format = get_image_type_format_from_data(data_file);
		// load
		return load(image_type_format, data_file, image_data, image_width, image_height, image_format, image_type);
	}

	//load raw
	bool Image::load
	(
		ImageTypeFormat image_type_format,
		const std::vector<unsigned char>& data_file,
		std::vector<unsigned char>& image_data,
		unsigned long&  image_width,
		unsigned long&  image_height,
		Render::TextureFormat& image_format,
		Render::TextureType& image_type
	)
	{
		//test
		if (image_type_format == ImageTypeFormat::ITF_UNKNOWN) return false;
		//fail to read?
		if (!data_file.size()) return false;
		//png
		switch (image_type_format)
		{
		case Square::Data::Image::ImageTypeFormat::ITF_PNG:
		case Square::Data::Image::ImageTypeFormat::ITF_JPG:
		case Square::Data::Image::ImageTypeFormat::ITF_BMP:
		{
			
			stbi_set_flip_vertically_on_load(false);
			//rgb/rgba attribute
			unsigned char* data = nullptr;
			int width = 0;
			int height = 0;
			int components = 0;
			//decoder
			data = stbi_load_from_memory(
				(const unsigned char*)data_file.data(),
				(int)data_file.size(),
				&width,
				&height,
				&components,
				0
			);
			//fail to load
			if (!data) return false;
			//get type
			switch (components)
			{
			case 1: image_format = Render::TF_R8;     image_type = Render::TT_R;    break;
			case 2: image_format = Render::TF_RG8;    image_type = Render::TT_RG;   break;
			case 3: image_format = Render::TF_RGB8;   image_type = Render::TT_RGB;  break;
			case 4: image_format = Render::TF_RGBA8;  image_type = Render::TT_RGBA; break;
			default:
				stbi_image_free(data);
				return false;
			break;
			}
			//copy size
			image_width = width;
			image_height = height;
			//copy into C++ buffer
			image_data.resize(width*height*components);
			std::memcpy(image_data.data(), data, image_data.size());
			//success
			stbi_image_free(data);
			return true;
		}
		break;
		case Square::Data::Image::ImageTypeFormat::ITF_TGA:
			return decode_tga(image_data,
							  image_width,
							  image_height,
							  image_format,
							  image_type,
							  (const unsigned char*)data_file.data(),
							  data_file.size());
			break;
		default:
			return false;
		break;
		}
	}
}
}
