//
//  Square
//
//  Created by Gabriele on 09/09/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Core/Allocator.h"
#include "Square/Data/Image.h"

namespace Square
{
namespace  Data
{

	Shared<Image> Image::from_rgb565(Allocator* allocator, const unsigned char* buffer,unsigned long width,unsigned long height)
	{
		//alloc conteiner
		auto o_image = MakeShared<Image>(allocator, allocator);
		//alloc buffer
		o_image->m_buffer.resize(width*height);
		//save size
		o_image->m_width = width;
		o_image->m_height= height;
		//copy into buffer
		o_image->apply_kernel([=](Image& thiz,ImagePixel& pixel,unsigned long x,unsigned long y)
		{
			// R R R R R G G G bytes[i*2+0]
			// G G G B B B B B bytes[i*2+1]
			pixel.m_r = ((buffer[x*2+width*2*y+0] & 0xF8) >> 3);
			pixel.m_g = ((buffer[x*2+width*2*y+0] & 0x07) << 3) | ((buffer[x*2+width*2*y+1] & 0xE0) >> 5);
			pixel.m_b =  (buffer[x*2+width*2*y+1] & 0x1F);
			pixel.m_a = 255;
			//normalize
			pixel.m_r = (unsigned char)(float(pixel.m_r) / 32.0f * 255.0f);
			pixel.m_g = (unsigned char)(float(pixel.m_g) / 64.0f * 255.0f);
			pixel.m_b = (unsigned char)(float(pixel.m_b) / 32.0f * 255.0f);
		});
    
		return o_image;
	}

	Shared<Image> Image::from_rgb5a1(Allocator* allocator, const unsigned char* buffer, unsigned long width, unsigned long height)
	{
		//alloc conteiner
		auto o_image = MakeShared<Image>(allocator, allocator);
		//alloc buffer
		o_image->m_buffer.resize(width*height);
		//save size
		o_image->m_width = width;
		o_image->m_height = height;
		//copy into buffer
		o_image->apply_kernel([=](Image& thiz, ImagePixel& pixel, unsigned long x, unsigned long y)
		{
			// R R R R R G G G bytes[i*2+0]
			// G G G B B B B B bytes[i*2+1]
			pixel.m_r = ((buffer[x * 2 + width * 2 * y + 0] & 0xF8) >> 3);
			pixel.m_g = ((buffer[x * 2 + width * 2 * y + 0] & 0x07) << 3) | ((buffer[x * 2 + width * 2 * y + 1] & 0xfb) >> 6);
			pixel.m_b = ((buffer[x * 2 + width * 2 * y + 1] & 0x3e) >> 1);
			pixel.m_a = (buffer[x * 2 + width * 2 * y + 1]  & 0x01 ) ? 255 : 0;
			//normalize
			pixel.m_r = (unsigned char)(float(pixel.m_r) / 32.0f * 255.0f);
			pixel.m_g = (unsigned char)(float(pixel.m_g) / 64.0f * 255.0f);
			pixel.m_b = (unsigned char)(float(pixel.m_b) / 32.0f * 255.0f);
		});

		return o_image;
	}

	Shared<Image> Image::from_r (Allocator* allocator, const unsigned char* buffer,unsigned long width,unsigned long height)
	{
		//alloc conteiner
		auto o_image = MakeShared<Image>(allocator, allocator);
		//alloc buffer
		o_image->m_buffer.resize(width*height);
		//save size
		o_image->m_width = width;
		o_image->m_height= height;
		//copy into buffer
		o_image->apply_kernel([=](Image& thiz,ImagePixel& pixel,unsigned long x,unsigned long y)
							  {
								  pixel.m_r = buffer[x+width*y];
								  pixel.m_g = 0;
								  pixel.m_b = 0;
								  pixel.m_a = 255;
							  });
		//
		return o_image;
	}

	Shared<Image> Image::from_rg (Allocator* allocator, const unsigned char* buffer,unsigned long width,unsigned long height)
	{
		//alloc conteiner
		auto o_image = MakeShared<Image>(allocator, allocator);
		//alloc buffer
		o_image->m_buffer.resize(width*height);
		//save size
		o_image->m_width = width;
		o_image->m_height= height;
		//copy into buffer
		o_image->apply_kernel([=](Image& thiz,ImagePixel& pixel,unsigned long x,unsigned long y)
							  {
								  pixel.m_r = buffer[x*2+width*2*y+0];
								  pixel.m_g = buffer[x*2+width*2*y+1];
								  pixel.m_b = 0;
								  pixel.m_a = 255;
							  });
		//
		return o_image;
	}

	Shared<Image> Image::from_rgb (Allocator* allocator, const unsigned char* buffer,unsigned long width,unsigned long height)
	{
		//alloc conteiner
		auto o_image = MakeShared<Image>(allocator, allocator);
		//alloc buffer
		o_image->m_buffer.resize(width*height);
		//save size
		o_image->m_width = width;
		o_image->m_height= height;
		//copy into buffer
		o_image->apply_kernel([=](Image& thiz,ImagePixel& pixel,unsigned long x,unsigned long y)
		{
			pixel.m_r = buffer[x*3+width*3*y+0];
			pixel.m_g = buffer[x*3+width*3*y+1];
			pixel.m_b = buffer[x*3+width*3*y+2];
			pixel.m_a = 255;
		});
		//
		return o_image;
	}
	
	Shared<Image> Image::from_rgba (Allocator* allocator, const unsigned char* buffer,unsigned long width,unsigned long height)
	{
		//alloc conteiner
		auto o_image = MakeShared<Image>(allocator, allocator);
		//alloc buffer
		o_image->m_buffer.resize(width*height);
		//save size
		o_image->m_width = width;
		o_image->m_height= height;
		//copy
		std::memcpy(o_image->m_buffer.data(),buffer,width*height*sizeof(ImagePixel));
		//return
		return o_image;
	}

	void Image::apply_kernel(ImageKernel kernel)
	{
		for(unsigned long y = 0; y != m_height; ++y)
		for(unsigned long x = 0; x != m_width;  ++x)
		{
			kernel(*this,m_buffer[x+y*m_width],x,y);
		}
	}

	void Image::flip_horizontal()
	{
		//half size
		size_t half_width = m_width / 2;
		//..
		for(unsigned long y = 0; y != m_height; ++y)
		for(unsigned long x = 0; x != half_width;  ++x)
		{
			std::swap(m_buffer[x+m_width*y], m_buffer[(m_width - x -1)+m_width*y]);
		}
	}

	void Image::flip_vertical()
	{
		//line size
		unsigned long line_byte_size = m_width * sizeof(ImagePixel);
		//alloc
		ImagePixel* line = new ImagePixel[m_width];
		//for size
		unsigned long half_height = m_height / 2;
		//for all lines
		for (unsigned long y = 0; y != half_height; ++y)
		{
			std::memcpy(line,                                  &m_buffer[m_width*y],                  line_byte_size);
			std::memcpy(&m_buffer[m_width*y],                  &m_buffer[m_width*(m_height - y - 1)], line_byte_size);
			std::memcpy(&m_buffer[m_width*(m_height - y - 1)], line,                                  line_byte_size);
		}
		//dealloc
		delete[] line;
	}
	
	std::vector< unsigned char > Image::to_rgb()
	{
		//buffer
		std::vector< unsigned char > output;
		//init
		output.resize(m_width*m_height*3);
		//copy
		for(unsigned long y = 0; y != m_height; ++y)
		for(unsigned long x = 0; x != m_width;  ++x)
		{
			output[x*3+m_width*3*y+0] = m_buffer[x+m_width*y].m_r;
			output[x*3+m_width*3*y+1] = m_buffer[x+m_width*y].m_g;
			output[x*3+m_width*3*y+2] = m_buffer[x+m_width*y].m_b;
		}
		//return
		return output;
	}

	std::vector< unsigned char > Image::to_rgb16()
	{
		//buffer
		std::vector< unsigned char > output;
		//init
		output.resize(m_width*m_height*2);
    
		for(unsigned long y = 0; y != m_height; ++y)
		for(unsigned long x = 0; x != m_width;  ++x)
		{
			//normalize
			unsigned char r = (unsigned char)(float(m_buffer[x+m_width*y].m_r) / 255.0f * 32.0f); // 0 0 0 R R R R R
			unsigned char g = (unsigned char)(float(m_buffer[x+m_width*y].m_g) / 255.0f * 64.0f); // 0 0 G G G G G G
			unsigned char b = (unsigned char)(float(m_buffer[x+m_width*y].m_b) / 255.0f * 32.0f); // 0 0 0 B B B B B
			// R R R R R G G G bytes[i*2+0]
			// G G G B B B B B bytes[i*2+1]
			output[x*2+m_width*2*y+0] = (r << 3) | ((g >> 3) & 0x03);
			output[x*2+m_width*2*y+1] = (g << 5) | ( b       & 0x1F);
		}
		//return
		return output;
	}

	std::vector< unsigned char > Image::to_rgba()
	{
    
		//buffer
		std::vector< unsigned char > output;
		//init
		output.resize(m_width*m_height*4);
		//copy
		std::memcpy(output.data(), m_buffer.data(),m_width*m_height*sizeof(ImagePixel));
		//return
		return output;

	}

}
}
