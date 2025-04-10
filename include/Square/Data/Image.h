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

		//load image from file raw
		static bool load
		(
			const std::vector<unsigned char>& data_file,
			std::vector<unsigned char>& out_image,
			unsigned long&  image_width,
			unsigned long&  image_height,
			Render::TextureFormat& image_format,
			Render::TextureType&   image_type
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
