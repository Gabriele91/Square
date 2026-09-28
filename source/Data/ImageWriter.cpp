//
//  ImageWriter.cpp
//  Square
//
//  Image encoders: PNG (ImageUtils/stb_image_write.cpp) and TGA with RLE compression
//  (ImageUtils/tga.cpp); the formats the loaders of ImageLoader.cpp read back.
//
#include "Square/Config.h"
#include "Square/Data/Image.h"

extern "C"
{
	extern int stbi_write_png_compression_level;
	typedef void stbi_write_func(void *context, void *data, int size);
	int stbi_write_png_to_func
	(
		stbi_write_func *func,
		void *context,
		int w,
		int h,
		int comp,
		const void  *data,
		int stride_in_bytes
	);
}

namespace Square
{
namespace Data
{
	//TGA
	extern bool encode_tga
	(
		std::vector<unsigned char>& out_tga,
		unsigned long image_width,
		unsigned long image_height,
		unsigned long image_bytes_pixel,
		bool top_down,
		const unsigned char* in_image
	);

	//////////////////////////////////////////////////////////////////////
	// PNG (ImageUtils/stb_image_write.cpp)
	std::vector<unsigned char> Image::encode_png
	(
		const unsigned char* pixels,
		unsigned long width,
		unsigned long height,
		unsigned int  channels
	)
	{
		std::vector<unsigned char> out;
		if (!pixels || !width || !height || channels < 1 || channels > 4) return out;
		//the most compression: the level of stb is the length of the searched match chains (8 by
		//default), no gain after 256 (1024: 0.03% smaller, twice the time)
		stbi_write_png_compression_level = 256;
		//append the chunks of the file to out
		auto write = [](void* context, void* data, int size)
		{
			auto* out = (std::vector<unsigned char>*)context;
			out->insert(out->end(), (unsigned char*)data, (unsigned char*)data + size);
		};
		if (!stbi_write_png_to_func(write, &out, (int)width, (int)height, (int)channels, pixels, (int)(width * channels)))
		{
			out.clear();
		}
		return out;
	}

	//////////////////////////////////////////////////////////////////////
	// TGA RLE (ImageUtils/tga.cpp)
	std::vector<unsigned char> Image::encode_tga_rle
	(
		const unsigned char* pixels,
		unsigned long width,
		unsigned long height,
		unsigned int  bytes_per_pixel,
		bool          top_down
	)
	{
		std::vector<unsigned char> out;
		if (!encode_tga(out, width, height, bytes_per_pixel, top_down, pixels)) out.clear();
		return out;
	}
}
}
