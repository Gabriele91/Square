//
//  Square
//
//  Created by Gabriele on 09/09/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Core/Filesystem.h"
#include "Square/Graphics/Render.h"
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

	//load image
	Image::SPtr Image::load(const std::string& path)
	{
		//get ext
		std::string ext = Filesystem::get_extension(path);
		//test
		if (   ext != ".png"
			&& ext != ".jpg"
			&& ext != ".jpeg"
			&& ext != ".tga") return nullptr;
		//read all file
		std::vector<char> data_file = Filesystem::file_read_all(path);
		//fail to read?
		if (!data_file.size()) return nullptr;
		//png
		if (   ext == ".png"
			|| ext == ".jpg"
			|| ext == ".jpeg")
		{
			stbi_set_flip_vertically_on_load(false);
			//rgb/rgba attribute
			unsigned char* data = nullptr;
			int width = 0;
			int height = 0;
			int components = 0;
			//decoder
			data = stbi_load_from_memory((const unsigned char*)data_file.data(),
										 (int)data_file.size(),
										 &width,
										 &height,
										 &components,
										 0);
			if (!data) return nullptr;
			//save
			std::shared_ptr< Image > o_image = nullptr;
			//save
			switch (components)
			{
				case 1: o_image = from_r(data, width, height);   break;
				case 2: o_image = from_rg(data, width, height);   break;
				case 3: o_image = from_rgb(data, width, height);   break;
				case 4: o_image = from_rgba(data, width, height);   break;
				default:  break;
			}
			//
			stbi_image_free(data);

			return o_image;
		}
		//tga
		else
		{
			//decode
			unsigned long image_width = 0;
			unsigned long image_height = 0;
			Render::TextureType   image_type;
			Render::TextureFormat image_format;
			std::vector<unsigned char> image_data;
			//decode
			if (!decode_tga(image_data,
							image_width,
							image_height,
							image_format,
							image_type,
							(const unsigned char*)data_file.data(),
							data_file.size()))
				return nullptr;
			//save
			Image::SPtr o_image = nullptr;
			//types
			switch (image_format)
			{
			case Render::TF_R8:     o_image = from_r(image_data.data(), image_width, image_height);        break;
			case Render::TF_RGB565: o_image = from_rgb565(image_data.data(), image_width, image_height);   break;
			case Render::TF_RGB5A1: o_image = from_rgb5a1(image_data.data(), image_width, image_height);   break;
			case Render::TF_RGB8:   o_image = from_rgb(image_data.data(), image_width, image_height);      break;
			case Render::TF_RGBA8:  o_image = from_rgba(image_data.data(), image_width, image_height);     break;
			default: break;
			}
			return o_image;
		}
	}

	//load raw
	bool Image::load
	(
		const std::string& path,
		std::vector<unsigned char>& image_data,
		unsigned long&  image_width,
		unsigned long&  image_height,
		Render::TextureFormat& image_format,
		Render::TextureType&   image_type
	)
	{
		//get ext
		std::string ext = Filesystem::get_extension(path);
		//test
		if (ext != ".png"
			&& ext != ".jpg"
			&& ext != ".jpeg"
			&& ext != ".tga") return nullptr;
		//read all file
		std::vector<char> data_file = Filesystem::file_read_all(path);
		//fail to read?
		if (!data_file.size()) return nullptr;
		//png
		if (   ext == ".png"
			|| ext == ".jpg"
			|| ext == ".jpeg")
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
		//tga
		else
		{
			//decode
			if (!decode_tga(image_data,
							image_width,
							image_height,
							image_format,
							image_type,
							(const unsigned char*)data_file.data(),
							data_file.size()))
				return false;
			return true;
		}
	}
}
}
