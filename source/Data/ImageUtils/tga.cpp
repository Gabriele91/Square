//
//  Square
//
//  Created by Gabriele on 05/07/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Driver/Render.h"
#include <cstring>
#include <assert.h>

namespace Square
{
namespace Data
{

	enum tga_image_type
	{
		TGA_NONE = 0,
		TGA_INDEXED = 1,
		TGA_RGB = 2,
		TGA_GREY = 3,
		TGA_INDEXED_RLE = 9,
		TGA_RGB_RLE = 10,
		TGA_GREY_RLE = 11,
	};

	PACKED(struct TGAHeader
	{
		unsigned char  m_identsize;          // size of ID field that follows 18 byte header (0 usually)
		unsigned char  m_colourmaptype;      // type of colour map 0=none, 1=has palette
		unsigned char  m_imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

		short m_colourmapstart;            // first colour map entry in palette
		short m_colourmaplength;           // number of colours in palette
		unsigned char  m_colourmapbits;    // number of bits per palette entry 15,16,24,32

		short m_xstart;                    // image x origin
		short m_ystart;				       // image y origin
		short m_width;				       // image width in pixels
		short m_height;					   // image height in pixels
		unsigned char  m_bits;             // image bits per pixel 8,16,24,32
		unsigned char  m_descriptor;       // image descriptor bits (vh flip bits)
	});
	#define VERTICAL_FLIP(tga_h)   ( tga_h->m_descriptor & 0b00100000)
	#define HORIZONTAL_FLIP(tga_h) ( tga_h->m_descriptor & 0b00010000)

	static void decoder_rle
	(
		unsigned long width,
		unsigned long height,
		unsigned long image_bytes_pixel,
		const unsigned char* buffer_in,
		std::vector<unsigned char>& out_image
	)
	{
		//bytes
		const unsigned char* c_ptr = buffer_in;
		//new alloc
		out_image.resize(width * height * image_bytes_pixel);
		//ptr to buffer out
		const unsigned char* buffer_out = out_image.data();
		//length of chunk
		size_t i = 0;
		//length of chunk
		unsigned char length_chunk = 0;
		// Decode
		while (i < out_image.size())
		{
			// 1XXX XXXX RLE chunk [ LEN | PIXEL ]
			if (*c_ptr & 0x80)
			{
				//length
				length_chunk = *c_ptr - 127;
				//pixel
				++c_ptr;
				//copy
				for (; length_chunk; i += image_bytes_pixel, --length_chunk)
					std::memcpy((void*)&buffer_out[i], c_ptr, image_bytes_pixel);
				//next chunk
				c_ptr += image_bytes_pixel;
			}
			// RAW chunk  [ LEN | PIXEL | PIXEL |... ]
			else
			{
				//length
				length_chunk = *c_ptr + 1;
				//pixel
				++c_ptr;
				//copy
				std::memcpy((void*)&buffer_out[i], c_ptr, image_bytes_pixel*length_chunk);
				//offset
				c_ptr += image_bytes_pixel*length_chunk;
				i += image_bytes_pixel*length_chunk;
				//length to 0
				length_chunk = 0;
			}
		}
	}

	static void read_rgba5551
	(
		const unsigned char* data,
		unsigned char& r,
		unsigned char& g,
		unsigned char& b,
		unsigned char& a
	)
	{
		r = (data[0] & 0xf8) >> 3;
		g = ((data[0] & 0x07) << 2) | ((data[1] & 0xfb) >> 6);
		b = (data[1] & 0x3e) >> 1;
		a = data[1] & 0x01;
	}

	static void write_rgba5551
	(
		unsigned char* data,
		unsigned char r,
		unsigned char g,
		unsigned char b,
		unsigned char a
	)
	{
		data[0] = (r << 3) | (data[0] & 0x07);
		data[0] = ((g & 0x1c) >> 2) | (data[0] & 0xf8);
		data[1] = ((g & 0x03) << 6) | (data[1] & 0xf8);
		data[1] = (b << 1) | (data[1] & 0xc1);
		data[1] = a | (data[1] & 0xfe);
	}

	static void rga_swap_r_and_b_16
	(
		unsigned char* bytes,
		unsigned long width,
		unsigned long height
	)
	{
		size_t image_size = width * height;
		//16 image
		for (unsigned long i = 0; i != image_size; ++i)
		{
			// R R R R R G G G bytes[i*2+0]
			// G G G B B B B B bytes[i*2+1]
			unsigned char r, g, b, a;
			//read
			read_rgba5551(&bytes[i * 2 + 0], r, g, b, a);
			//swap
			std::swap(r,b);
			//write
			write_rgba5551(&bytes[i * 2 + 0], r, g, b, a);
		}
	}

	static void rga_swap_r_and_b_24_32(unsigned char* bytes,
		unsigned long image_bytes_pixel,
		unsigned long width,
		unsigned long height)
	{
		size_t image_size = width * height;
		//24/32 image
		for (unsigned long i = 0; i != image_size; ++i)
		{
			unsigned char tmp;
			tmp = bytes[i * image_bytes_pixel + 0];
			bytes[i * image_bytes_pixel + 0] = bytes[i * image_bytes_pixel + 2];
			bytes[i * image_bytes_pixel + 2] = tmp;
		}
	}



#if 1
	static void image_y_flip
	(
		std::vector<unsigned char>& bytes,
		unsigned long image_bytes_pixel,
		unsigned long image_width,
		unsigned long image_height
	)
	{
		//line size
		unsigned long line_len = image_width * image_bytes_pixel * sizeof(unsigned char);
		//alloc
		unsigned char* line = new unsigned char[line_len];
		//for size
		unsigned long half_height = image_height / 2;
		//for all lines
		for (unsigned long y = 0; y != half_height; ++y)
		{
			std::memcpy(line, &bytes[line_len*y], line_len);
			std::memcpy(&bytes[line_len*y], &bytes[line_len*(image_height - y - 1)], line_len);
			std::memcpy(&bytes[line_len*(image_height - y - 1)], line, line_len);
		}
		//dealloc
		delete[] line;
	}

	static void image_x_flip
	(
		std::vector<unsigned char>& bytes,
		unsigned long image_bytes_pixel,
		unsigned long image_width,
		unsigned long image_height
	)
	{
		assert(image_bytes_pixel <= 4);
		//line size
		unsigned long line_width = image_width * image_bytes_pixel;
		//half size
		unsigned long half_width = image_width / 2;
		//buffer
		unsigned char temp_pixel[4];
		//..
		for (unsigned long y = 0; y != image_height; ++y)
		for (unsigned long x = 0; x != half_width; ++x)
		{
			unsigned char* left_pixel  = &bytes[x + line_width*y];
			unsigned char* right_pixel = &bytes[(line_width - x - 1) + line_width*y];
			//swap
			std::memcpy(temp_pixel, left_pixel, image_bytes_pixel);
			std::memcpy(left_pixel, right_pixel, image_bytes_pixel);
			std::memcpy(right_pixel, temp_pixel, image_bytes_pixel);
		}
	}
#endif

	bool decode_tga
	(
		std::vector<unsigned char>& out_image,
		unsigned long& image_width,
		unsigned long& image_height,
		Render::TextureFormat& image_format,
		Render::TextureType&   image_type,
		const unsigned char* in_tga,
		size_t in_size
    )
	{
		//ptr to data
		TGAHeader*		     header = (TGAHeader*)in_tga;
		const unsigned char* data = in_tga + sizeof(TGAHeader);
		//only rgb supported
		if (header->m_imagetype != TGA_RGB &&
			header->m_imagetype != TGA_RGB_RLE) return false;
		//return size
		image_width = header->m_width;
		image_height = header->m_height;
		//opengl type
		switch (header->m_bits)
		{
		case 8:   image_format = Render::TF_R8;      image_type = Render::TT_R;    break;
		case 16:  image_format = Render::TF_RGB5A1;  image_type = Render::TT_RGBA; break;
		case 24:  image_format = Render::TF_RGB8;    image_type = Render::TT_RGB;  break;
		case 32:  image_format = Render::TF_RGBA8;   image_type = Render::TT_RGBA; break;
		default: return false; break;
		}
		//copy data
		if (header->m_imagetype == TGA_RGB_RLE)
		{
			decoder_rle(image_width, image_height, header->m_bits / 8, data, out_image);
		}
		else
		{
			//resize
			out_image.resize(in_size - sizeof(TGAHeader));
			//copy raw data
			std::memcpy(&out_image[0], data, out_image.size());
		}
		//swap r/b
		switch (header->m_bits)
		{
			//2 bytes
		case 16:
			rga_swap_r_and_b_16(out_image.data(), image_width, image_height);
			break;
			//3/4 bytes
		case 24:
		case 32:
			rga_swap_r_and_b_24_32(out_image.data(), header->m_bits / 8, image_width, image_height);
			break;
		default:
			break;
		}
		//flip
		if VERTICAL_FLIP(header)
		{
			image_y_flip(out_image, header->m_bits / 8, image_width, image_height);
		}
		if HORIZONTAL_FLIP(header)
		{
			image_x_flip(out_image, header->m_bits / 8, image_width, image_height);
		}
		//success
		return true;
	}
}
}