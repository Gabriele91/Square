//
//  Square
//
//  Created by Gabriele on 09/09/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include <functional>
#include "Square/Config.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Driver/Render.h"

namespace Square
{
namespace  Data
{

	//image declaretion
	class  Image;

	//pixel declaretion
	PACKED(struct ImagePixel
	{
		unsigned char m_r;
		unsigned char m_g;
		unsigned char m_b;
		unsigned char m_a;
	});

	//kernel declaretion
	using ImageKernel = std::function< void(Image& thiz, ImagePixel& pixel, unsigned long x,unsigned long y) >;

	//image definition
	class Image : public SharedObject<Image>, public Uncopyable
	{
	public:
		static Shared<Image> from_r     (Allocator* allocator, const unsigned char* buffer,unsigned long width,unsigned long height);
		static Shared<Image> from_rg    (Allocator* allocator, const unsigned char* buffer,unsigned long width,unsigned long height);
		static Shared<Image> from_rgb   (Allocator* allocator, const unsigned char* buffer,unsigned long width,unsigned long height);
		static Shared<Image> from_rgb565(Allocator* allocator, const unsigned char* buffer, unsigned long width, unsigned long height);
		static Shared<Image> from_rgb5a1(Allocator* allocator, const unsigned char* buffer, unsigned long width, unsigned long height);
		static Shared<Image> from_rgba  (Allocator* allocator, const unsigned char* buffer,unsigned long width,unsigned long height);
    
		Image(Allocator* allocator) : SharedObject_t(allocator) {}
    
		void apply_kernel(ImageKernel kernel);
		void flip_vertical();
		void flip_horizontal();
		void normal_inv_y();
    
		unsigned long get_width() const  { return m_width; }
		unsigned long get_height() const { return m_height; }
    
		std::vector< unsigned char > to_rgb();
		std::vector< unsigned char > to_rgb16();
		std::vector< unsigned char > to_rgba();

		//load image from file
		static Shared<Image> load(Allocator* allocator, const std::string& path);
		
		//load image from file raw
		static bool load
		(
			const std::string& path,
			std::vector<unsigned char>& out_image,
			unsigned long&  image_width,
			unsigned long&  image_height,
			Render::TextureFormat& image_format,
			Render::TextureType&   image_type
		);

		//load image from file raw (the data of a file: PNG, JPEG, BMP, TGA; the format from the data)
		static SQUARE_API bool load
		(
			const std::vector<unsigned char>& data_file,
			std::vector<unsigned char>& out_image,
			unsigned long&  image_width,
			unsigned long&  image_height,
			Render::TextureFormat& image_format,
			Render::TextureType&   image_type
		);

		//encode a PNG file (8 bit per channel): pixels row after row from the top, channels 1
		//(grey), 2 (grey alpha), 3 (RGB) or 4 (RGBA); empty on a wrong input
		static SQUARE_API std::vector<unsigned char> encode_png
		(
			const unsigned char* pixels,
			unsigned long width,
			unsigned long height,
			unsigned int  channels
		);

		//encode a TGA file with RLE compression (type 10, TGA 2.0 footer): pixels in the order of
		//TGA, bytes_per_pixel 2 (A1R5G5B5 little endian), 3 (BGR) or 4 (BGRA); rows from the top
		//(top_down) or from the bottom; empty on a wrong input
		static SQUARE_API std::vector<unsigned char> encode_tga_rle
		(
			const unsigned char* pixels,
			unsigned long width,
			unsigned long height,
			unsigned int  bytes_per_pixel,
			bool          top_down
		);

		//////////////////////////////////////////////////////////////////////
		// Compressed textures (ImageCompressed.cpp): BC1/BC3/BC4/BC5 in DDS files, ASTC 4x4 (and
		// BC) in KTX 1 files; the levels one after the other from the largest, rows of 4x4 blocks
		// (Render::texture_level_bytes)

		//a DDS or KTX file (from its first bytes)
		static SQUARE_API bool is_compressed(const std::vector<unsigned char>& data_file);

		//the levels of a DDS or KTX file of a compressed format the engine has
		static SQUARE_API bool load_compressed
		(
			const std::vector<unsigned char>& data_file,
			std::vector<unsigned char>& out_levels,
			unsigned long&  image_width,
			unsigned long&  image_height,
			unsigned int&   image_levels,
			Render::TextureFormat& image_format
		);

		//compress a level to BC1 (RGB), BC3 (RGBA), BC4 (R) or BC5 (RG): pixels RGBA (4 bytes),
		//row after row from the top; the borders of the last blocks repeat the last pixels;
		//empty on a wrong input
		static SQUARE_API std::vector<unsigned char> compress_bc
		(
			const unsigned char* rgba,
			unsigned long width,
			unsigned long height,
			Render::TextureFormat format
		);

		//compress a level to ASTC 4x4 (LDR, medium quality, the threads of the CPU): pixels RGBA
		//(4 bytes), row after row from the top; empty on a wrong input
		static SQUARE_API std::vector<unsigned char> compress_astc
		(
			const unsigned char* rgba,
			unsigned long width,
			unsigned long height
		);

		//encode the levels of a BC texture in a DDS file; empty on a wrong input
		static SQUARE_API std::vector<unsigned char> encode_dds
		(
			const unsigned char* levels_data,
			unsigned long width,
			unsigned long height,
			unsigned int  levels,
			Render::TextureFormat format
		);

		//encode the levels of a compressed texture in a KTX 1 file; empty on a wrong input
		static SQUARE_API std::vector<unsigned char> encode_ktx
		(
			const unsigned char* levels_data,
			unsigned long width,
			unsigned long height,
			unsigned int  levels,
			Render::TextureFormat format
		);

	protected:

		enum class ImageTypeFormat
		{
			ITF_TGA,
			ITF_PNG,
			ITF_JPG,
			ITF_BMP,
			ITF_UNKNOWN
		};

		static ImageTypeFormat get_image_type_format_from_ext(const std::string& path);
		static ImageTypeFormat get_image_type_format_from_data(const std::vector<unsigned char>& in_data);

		//load image from raw data
		static bool load
		(
			ImageTypeFormat image_type_format,
			const std::vector<unsigned char>& data_file,
			std::vector<unsigned char>& out_image,
			unsigned long& image_width,
			unsigned long& image_height,
			Render::TextureFormat& image_format,
			Render::TextureType& image_type
		);
		unsigned long m_width { 0 };
		unsigned long m_height{ 0 };
		std::vector < ImagePixel > m_buffer;
    
	};

}
}
