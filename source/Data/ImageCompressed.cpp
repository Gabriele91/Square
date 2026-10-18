//
//  ImageCompressed.cpp
//  Square
//
//  Compressed textures: the encoders, BC (ImageUtils/stb_dxt.cpp) and ASTC (dependencies/
//  astc-encoder), and the files of the levels,
//  DDS (BC1/BC3/BC4/BC5: the FourCC header, read also with the DX10 one) and KTX 1 (the GL
//  internal formats: ASTC 4x4 and BC). Every level is rows of 4x4 blocks, the levels one after
//  the other from the largest (Render::texture_level_bytes).
//
#include <cstring>
#include <algorithm>
#include <thread>
#include "Square/Config.h"
#include "Square/Data/Image.h"
#include "astcenc.h"

extern "C"
{
	void stb_compress_dxt_block(unsigned char *dest, const unsigned char *src_rgba_four_bytes_per_pixel, int alpha, int mode);
	void stb_compress_bc4_block(unsigned char *dest, const unsigned char *src_r_one_byte_per_pixel);
	void stb_compress_bc5_block(unsigned char *dest, const unsigned char *src_rg_two_byte_per_pixel);
}

namespace Square
{
namespace Data
{
	//////////////////////////////////////////////////////////////////////
	// little endian values of the files
	static unsigned int read_u32(const unsigned char* bytes)
	{
		return (unsigned int)bytes[0]
			| ((unsigned int)bytes[1] << 8)
			| ((unsigned int)bytes[2] << 16)
			| ((unsigned int)bytes[3] << 24);
	}

	static void write_u32(Vector<unsigned char>& out, unsigned int value)
	{
		out.push_back((unsigned char)(value & 0xFF));
		out.push_back((unsigned char)((value >> 8) & 0xFF));
		out.push_back((unsigned char)((value >> 16) & 0xFF));
		out.push_back((unsigned char)((value >> 24) & 0xFF));
	}

	static constexpr unsigned int four_cc(char a, char b, char c, char d)
	{
		return (unsigned int)(unsigned char)a
			| ((unsigned int)(unsigned char)b << 8)
			| ((unsigned int)(unsigned char)c << 16)
			| ((unsigned int)(unsigned char)d << 24);
	}

	//bytes of all the levels
	static size_t levels_bytes(Render::TextureFormat format, unsigned long width, unsigned long height, unsigned int levels)
	{
		size_t size = 0;
		for (unsigned int level = 0; level < levels; ++level)
		{
			size += Render::texture_level_bytes(format, Render::texture_level_size((unsigned int)width, level), Render::texture_level_size((unsigned int)height, level));
		}
		return size;
	}

	//levels a texture can have (down to 1x1)
	static unsigned int max_levels(unsigned long width, unsigned long height)
	{
		unsigned int levels = 1;
		for (unsigned long size = (width > height ? width : height); size > 1; size >>= 1) ++levels;
		return levels;
	}

	//////////////////////////////////////////////////////////////////////
	// DDS
	static constexpr unsigned int    DDS_MAGIC = four_cc('D', 'D', 'S', ' ');
	static constexpr unsigned int    DDS_HEADER_SIZE = 124;
	static constexpr unsigned int    DDS_PIXEL_FORMAT_SIZE = 32;
	static constexpr unsigned int    DDS_DX10_HEADER_SIZE = 20;
	//header flags: caps, height, width, pixel format, mipmap count, linear size
	static constexpr unsigned int    DDSD_FLAGS = 0x1 | 0x2 | 0x4 | 0x1000 | 0x20000 | 0x80000;
	//pixel format flags: FourCC
	static constexpr unsigned int    DDPF_FOURCC = 0x4;
	//caps: texture, mipmap, complex (more levels)
	static constexpr unsigned int    DDSCAPS_TEXTURE = 0x1000;
	static constexpr unsigned int    DDSCAPS_MIPMAP = 0x400000;
	static constexpr unsigned int    DDSCAPS_COMPLEX = 0x8;

	static Render::TextureFormat dds_format_from_four_cc(unsigned int code)
	{
		switch (code)
		{
		case four_cc('D', 'X', 'T', '1'): return Render::TF_BC1;
		case four_cc('D', 'X', 'T', '5'): return Render::TF_BC3;
		case four_cc('A', 'T', 'I', '1'):
		case four_cc('B', 'C', '4', 'U'): return Render::TF_BC4;
		case four_cc('A', 'T', 'I', '2'):
		case four_cc('B', 'C', '5', 'U'): return Render::TF_BC5;
		default:                          return Render::TF_INVALID;
		}
	}

	static Render::TextureFormat dds_format_from_dxgi(unsigned int dxgi_format)
	{
		switch (dxgi_format)
		{
		case 71: case 72: return Render::TF_BC1; //BC1_UNORM(_SRGB)
		case 77: case 78: return Render::TF_BC3; //BC3_UNORM(_SRGB)
		case 80:          return Render::TF_BC4; //BC4_UNORM
		case 83:          return Render::TF_BC5; //BC5_UNORM
		default:          return Render::TF_INVALID;
		}
	}

	static unsigned int dds_four_cc_from_format(Render::TextureFormat format)
	{
		switch (format)
		{
		case Render::TF_BC1: return four_cc('D', 'X', 'T', '1');
		case Render::TF_BC3: return four_cc('D', 'X', 'T', '5');
		case Render::TF_BC4: return four_cc('A', 'T', 'I', '1');
		case Render::TF_BC5: return four_cc('A', 'T', 'I', '2');
		default:             return 0;
		}
	}

	static bool load_dds
	(
		const Vector<unsigned char>& data_file,
		Vector<unsigned char>& out_levels,
		unsigned long& image_width,
		unsigned long& image_height,
		unsigned int& image_levels,
		Render::TextureFormat& image_format
	)
	{
		//magic + header
		if (data_file.size() < 4 + DDS_HEADER_SIZE) return false;
		const unsigned char* header = data_file.data() + 4;
		if (read_u32(header) != DDS_HEADER_SIZE) return false;
		const unsigned int height = read_u32(header + 8);
		const unsigned int width = read_u32(header + 12);
		const unsigned int levels = read_u32(header + 24);
		//pixel format (at 72 in the header)
		const unsigned char* pixel_format = header + 72;
		if (!(read_u32(pixel_format + 4) & DDPF_FOURCC)) return false;
		const unsigned int code = read_u32(pixel_format + 8);
		size_t offset = 4 + DDS_HEADER_SIZE;
		Render::TextureFormat format = Render::TF_INVALID;
		if (code == four_cc('D', 'X', '1', '0'))
		{
			if (data_file.size() < offset + DDS_DX10_HEADER_SIZE) return false;
			format = dds_format_from_dxgi(read_u32(data_file.data() + offset));
			offset += DDS_DX10_HEADER_SIZE;
		}
		else
		{
			format = dds_format_from_four_cc(code);
		}
		if (format == Render::TF_INVALID || !width || !height) return false;
		//levels (0 or 1: only the first)
		const unsigned int count = levels ? (levels < max_levels(width, height) ? levels : max_levels(width, height)) : 1;
		const size_t size = levels_bytes(format, width, height, count);
		if (data_file.size() < offset + size) return false;
		out_levels.assign(data_file.begin() + offset, data_file.begin() + offset + size);
		image_width = width;
		image_height = height;
		image_levels = count;
		image_format = format;
		return true;
	}

	//////////////////////////////////////////////////////////////////////
	// KTX 1
	static const unsigned char KTX_IDENTIFIER[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	static constexpr unsigned int KTX_HEADER_SIZE = 64;
	static constexpr unsigned int KTX_ENDIANNESS = 0x04030201;
	//GL base internal formats
	static constexpr unsigned int GL_BASE_RED = 0x1903;
	static constexpr unsigned int GL_BASE_RG = 0x8227;
	static constexpr unsigned int GL_BASE_RGB = 0x1907;
	static constexpr unsigned int GL_BASE_RGBA = 0x1908;

	static Render::TextureFormat ktx_format_from_gl(unsigned int gl_internal_format)
	{
		switch (gl_internal_format)
		{
		case 0x83F0: case 0x83F1: case 0x8C4C: case 0x8C4D: return Render::TF_BC1;      //S3TC DXT1 (RGB, RGBA, sRGB)
		case 0x83F3: case 0x8C4F:                           return Render::TF_BC3;      //S3TC DXT5 (sRGB)
		case 0x8DBB:                                        return Render::TF_BC4;      //RGTC1 red
		case 0x8DBD:                                        return Render::TF_BC5;      //RGTC2 RG
		case 0x93B0: case 0x93D0:                           return Render::TF_ASTC_4x4; //ASTC 4x4 (sRGB)
		default:                                            return Render::TF_INVALID;
		}
	}

	static void ktx_gl_from_format(Render::TextureFormat format, unsigned int& gl_internal_format, unsigned int& gl_base_format)
	{
		switch (format)
		{
		case Render::TF_BC1:      gl_internal_format = 0x83F0; gl_base_format = GL_BASE_RGB;  break;
		case Render::TF_BC3:      gl_internal_format = 0x83F3; gl_base_format = GL_BASE_RGBA; break;
		case Render::TF_BC4:      gl_internal_format = 0x8DBB; gl_base_format = GL_BASE_RED;  break;
		case Render::TF_BC5:      gl_internal_format = 0x8DBD; gl_base_format = GL_BASE_RG;   break;
		case Render::TF_ASTC_4x4: gl_internal_format = 0x93B0; gl_base_format = GL_BASE_RGBA; break;
		default:                  gl_internal_format = 0;      gl_base_format = 0;            break;
		}
	}

	static bool load_ktx
	(
		const Vector<unsigned char>& data_file,
		Vector<unsigned char>& out_levels,
		unsigned long& image_width,
		unsigned long& image_height,
		unsigned int& image_levels,
		Render::TextureFormat& image_format
	)
	{
		if (data_file.size() < KTX_HEADER_SIZE) return false;
		const unsigned char* header = data_file.data();
		//little endian files only (the ones of the tools)
		if (read_u32(header + 12) != KTX_ENDIANNESS) return false;
		const Render::TextureFormat format = ktx_format_from_gl(read_u32(header + 28));
		const unsigned int width = read_u32(header + 36);
		const unsigned int height = read_u32(header + 40);
		const unsigned int depth = read_u32(header + 44);
		const unsigned int array_elements = read_u32(header + 48);
		const unsigned int faces = read_u32(header + 52);
		const unsigned int levels = read_u32(header + 56);
		const unsigned int key_values_size = read_u32(header + 60);
		//a 2D texture
		if (format == Render::TF_INVALID || !width || !height || depth > 1 || array_elements > 1 || faces != 1) return false;
		const unsigned int count = levels ? (levels < max_levels(width, height) ? levels : max_levels(width, height)) : 1;
		//levels: the size, the blocks (4 bytes aligned: the blocks are 8 or 16 bytes)
		size_t offset = KTX_HEADER_SIZE + key_values_size;
		out_levels.clear();
		for (unsigned int level = 0; level < count; ++level)
		{
			if (data_file.size() < offset + 4) return false;
			const size_t level_size = read_u32(data_file.data() + offset);
			const size_t expected = Render::texture_level_bytes(format, Render::texture_level_size(width, level), Render::texture_level_size(height, level));
			offset += 4;
			if (level_size != expected || data_file.size() < offset + level_size) return false;
			out_levels.insert(out_levels.end(), data_file.begin() + offset, data_file.begin() + offset + level_size);
			offset += (level_size + 3) & ~size_t(3);
		}
		image_width = width;
		image_height = height;
		image_levels = count;
		image_format = format;
		return true;
	}

	//////////////////////////////////////////////////////////////////////
	// Image
	bool Image::is_compressed(const Vector<unsigned char>& data_file)
	{
		if (data_file.size() >= 4 && read_u32(data_file.data()) == DDS_MAGIC) return true;
		if (data_file.size() >= sizeof(KTX_IDENTIFIER) && std::memcmp(data_file.data(), KTX_IDENTIFIER, sizeof(KTX_IDENTIFIER)) == 0) return true;
		return false;
	}

	bool Image::load_compressed
	(
		const Vector<unsigned char>& data_file,
		Vector<unsigned char>& out_levels,
		unsigned long& image_width,
		unsigned long& image_height,
		unsigned int& image_levels,
		Render::TextureFormat& image_format
	)
	{
		if (data_file.size() >= 4 && read_u32(data_file.data()) == DDS_MAGIC)
		{
			return load_dds(data_file, out_levels, image_width, image_height, image_levels, image_format);
		}
		if (data_file.size() >= sizeof(KTX_IDENTIFIER) && std::memcmp(data_file.data(), KTX_IDENTIFIER, sizeof(KTX_IDENTIFIER)) == 0)
		{
			return load_ktx(data_file, out_levels, image_width, image_height, image_levels, image_format);
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////
	// BC decoding: the 16 pixels of a block
	//a color of 5:6:5 bits to 8 bits
	static void decode_565(unsigned int color, unsigned char out[4])
	{
		const unsigned int r = (color >> 11) & 0x1F;
		const unsigned int g = (color >> 5) & 0x3F;
		const unsigned int b = color & 0x1F;
		out[0] = (unsigned char)((r << 3) | (r >> 2));
		out[1] = (unsigned char)((g << 2) | (g >> 4));
		out[2] = (unsigned char)((b << 3) | (b >> 2));
		out[3] = 255;
	}

	//the colors of BC1/BC3 (8 bytes): RGBA of the 16 pixels; BC1 with its transparent black
	static void decode_color_block(const unsigned char* block, bool bc1, unsigned char pixels[16 * 4])
	{
		const unsigned int color0 = block[0] | (block[1] << 8);
		const unsigned int color1 = block[2] | (block[3] << 8);
		unsigned char palette[4][4];
		decode_565(color0, palette[0]);
		decode_565(color1, palette[1]);
		const bool four_colors = !bc1 || color0 > color1;
		for (int channel = 0; channel != 3; ++channel)
		{
			const int c0 = palette[0][channel];
			const int c1 = palette[1][channel];
			palette[2][channel] = (unsigned char)(four_colors ? (2 * c0 + c1) / 3 : (c0 + c1) / 2);
			palette[3][channel] = (unsigned char)(four_colors ? (c0 + 2 * c1) / 3 : 0);
		}
		palette[2][3] = 255;
		palette[3][3] = four_colors ? 255 : 0;
		const unsigned int indices = read_u32(block + 4);
		for (int pixel = 0; pixel != 16; ++pixel)
		{
			const unsigned int index = (indices >> (2 * pixel)) & 0x3;
			std::memcpy(pixels + pixel * 4, palette[index], 4);
		}
	}

	//a channel of BC3 (alpha), BC4, BC5 (8 bytes): the 16 values
	static void decode_channel_block(const unsigned char* block, unsigned char values[16])
	{
		const int v0 = block[0];
		const int v1 = block[1];
		int palette[8];
		palette[0] = v0;
		palette[1] = v1;
		if (v0 > v1)
		{
			for (int i = 1; i != 7; ++i)
			{
				palette[i + 1] = ((7 - i) * v0 + i * v1) / 7;
			}
		}
		else
		{
			for (int i = 1; i != 5; ++i)
			{
				palette[i + 1] = ((5 - i) * v0 + i * v1) / 5;
			}
			palette[6] = 0;
			palette[7] = 255;
		}
		//48 bits of indices, 3 a value
		unsigned long long bits = 0;
		for (int byte = 0; byte != 6; ++byte)
		{
			bits |= (unsigned long long)block[2 + byte] << (8 * byte);
		}
		for (int pixel = 0; pixel != 16; ++pixel)
		{
			values[pixel] = (unsigned char)palette[(bits >> (3 * pixel)) & 0x7];
		}
	}

	//the pixels of a block (its channels) by format
	static void decode_block(const unsigned char* block, Render::TextureFormat format, unsigned char* pixels, unsigned int channels)
	{
		unsigned char values[16];
		switch (format)
		{
		case Render::TF_BC1:
			decode_color_block(block, true, pixels);
			break;
		case Render::TF_BC3:
			decode_color_block(block + 8, false, pixels);
			decode_channel_block(block, values);
			for (int pixel = 0; pixel != 16; ++pixel)
			{
				pixels[pixel * 4 + 3] = values[pixel];
			}
			break;
		case Render::TF_BC4:
		case Render::TF_BC5:
			for (unsigned int channel = 0; channel != channels; ++channel)
			{
				decode_channel_block(block + 8 * channel, values);
				for (int pixel = 0; pixel != 16; ++pixel)
				{
					pixels[pixel * channels + channel] = values[pixel];
				}
			}
			break;
		default:
			break;
		}
	}

	Vector<unsigned char> Image::decompress_bc
	(
		const unsigned char* blocks,
		unsigned long width,
		unsigned long height,
		Render::TextureFormat format
	)
	{
		Vector<unsigned char> out;
		const unsigned int block_bytes = Render::texture_block_bytes(format);
		if (!blocks || !width || !height || !block_bytes || format == Render::TF_ASTC_4x4)
		{
			return out;
		}
		const unsigned int channels = format == Render::TF_BC4 ? 1 : format == Render::TF_BC5 ? 2 : 4;
		out.resize(size_t(width) * height * channels);
		unsigned char pixels[16 * 4];
		const unsigned char* block = blocks;
		for (unsigned long block_y = 0; block_y < height; block_y += 4)
		{
			for (unsigned long block_x = 0; block_x < width; block_x += 4)
			{
				decode_block(block, format, pixels, channels);
				block += block_bytes;
				//the pixels of the block inside the image
				for (unsigned long y = 0; y != 4 && block_y + y < height; ++y)
				{
					const unsigned long columns = std::min<unsigned long>(4, width - block_x);
					std::memcpy(out.data() + ((block_y + y) * width + block_x) * channels, pixels + y * 4 * channels, columns * channels);
				}
			}
		}
		return out;
	}

	Vector<unsigned char> Image::compress_bc
	(
		const unsigned char* rgba,
		unsigned long width,
		unsigned long height,
		Render::TextureFormat format
	)
	{
		Vector<unsigned char> out;
		const unsigned int block_bytes = Render::texture_block_bytes(format);
		if (!rgba || !width || !height || !block_bytes || format == Render::TF_ASTC_4x4) return out;
		out.resize(Render::texture_level_bytes(format, (unsigned int)width, (unsigned int)height));
		unsigned char* block = out.data();
		//the pixels of a block (RGBA), clamped to the image
		unsigned char pixels[16 * 4];
		unsigned char channels[16 * 2];
		for (unsigned long block_y = 0; block_y < height; block_y += 4)
		for (unsigned long block_x = 0; block_x < width; block_x += 4)
		{
			for (unsigned long y = 0; y < 4; ++y)
			for (unsigned long x = 0; x < 4; ++x)
			{
				const unsigned long image_x = (block_x + x < width) ? block_x + x : width - 1;
				const unsigned long image_y = (block_y + y < height) ? block_y + y : height - 1;
				std::memcpy(pixels + (y * 4 + x) * 4, rgba + (image_y * width + image_x) * 4, 4);
			}
			switch (format)
			{
			//2: STB_DXT_HIGHQUAL
			case Render::TF_BC1: stb_compress_dxt_block(block, pixels, 0, 2); break;
			case Render::TF_BC3: stb_compress_dxt_block(block, pixels, 1, 2); break;
			case Render::TF_BC4:
				for (int i = 0; i < 16; ++i) channels[i] = pixels[i * 4];
				stb_compress_bc4_block(block, channels);
				break;
			case Render::TF_BC5:
				for (int i = 0; i < 16; ++i)
				{
					channels[i * 2 + 0] = pixels[i * 4 + 0];
					channels[i * 2 + 1] = pixels[i * 4 + 1];
				}
				stb_compress_bc5_block(block, channels);
				break;
			default: break;
			}
			block += block_bytes;
		}
		return out;
	}

	Vector<unsigned char> Image::compress_astc
	(
		const unsigned char* rgba,
		unsigned long width,
		unsigned long height
	)
	{
		Vector<unsigned char> out;
		if (!rgba || !width || !height) return out;
		//the encoder, for the threads of the CPU (each one compresses blocks of the image)
		const unsigned int threads = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 1;
		astcenc_config config;
		astcenc_context* context = nullptr;
		if (astcenc_config_init(ASTCENC_PRF_LDR, 4, 4, 1, ASTCENC_PRE_MEDIUM, 0, &config) != ASTCENC_SUCCESS
		||  astcenc_context_alloc(&config, threads, &context, nullptr) != ASTCENC_SUCCESS)
		{
			return out;
		}
		//the image (the encoder reads it only)
		void* slices[1] = { const_cast<unsigned char*>(rgba) };
		astcenc_image image;
		image.dim_x = (unsigned int)width;
		image.dim_y = (unsigned int)height;
		image.dim_z = 1;
		image.data_type = ASTCENC_TYPE_U8;
		image.data = slices;
		const astcenc_swizzle swizzle{ ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A };
		out.resize(Render::texture_level_bytes(Render::TF_ASTC_4x4, image.dim_x, image.dim_y));
		Vector<astcenc_error> errors(threads, ASTCENC_SUCCESS);
		Vector<std::thread> workers;
		for (unsigned int thread_index = 0; thread_index != threads; ++thread_index)
		{
			workers.emplace_back([&, thread_index]()
			{
				errors[thread_index] = astcenc_compress_image(context, &image, &swizzle, out.data(), out.size(), thread_index);
			});
		}
		for (auto& worker : workers) worker.join();
		astcenc_context_free(context);
		for (astcenc_error error : errors)
		{
			if (error != ASTCENC_SUCCESS) return Vector<unsigned char>();
		}
		return out;
	}

	Vector<unsigned char> Image::encode_dds
	(
		const unsigned char* levels_data,
		unsigned long width,
		unsigned long height,
		unsigned int  levels,
		Render::TextureFormat format
	)
	{
		Vector<unsigned char> out;
		const unsigned int code = dds_four_cc_from_format(format);
		if (!levels_data || !width || !height || !levels || !code) return out;
		const size_t size = levels_bytes(format, width, height, levels);
		out.reserve(4 + DDS_HEADER_SIZE + size);
		write_u32(out, DDS_MAGIC);
		//header
		write_u32(out, DDS_HEADER_SIZE);
		write_u32(out, DDSD_FLAGS);
		write_u32(out, (unsigned int)height);
		write_u32(out, (unsigned int)width);
		write_u32(out, Render::texture_level_bytes(format, (unsigned int)width, (unsigned int)height)); //linear size
		write_u32(out, 0);      //depth
		write_u32(out, levels); //mipmap count
		for (int i = 0; i < 11; ++i) write_u32(out, 0); //reserved
		//pixel format
		write_u32(out, DDS_PIXEL_FORMAT_SIZE);
		write_u32(out, DDPF_FOURCC);
		write_u32(out, code);
		for (int i = 0; i < 5; ++i) write_u32(out, 0); //bit count, masks
		//caps
		write_u32(out, DDSCAPS_TEXTURE | (levels > 1 ? DDSCAPS_MIPMAP | DDSCAPS_COMPLEX : 0));
		for (int i = 0; i < 4; ++i) write_u32(out, 0); //caps 2, 3, 4, reserved
		//levels
		out.insert(out.end(), levels_data, levels_data + size);
		return out;
	}

	Vector<unsigned char> Image::encode_ktx
	(
		const unsigned char* levels_data,
		unsigned long width,
		unsigned long height,
		unsigned int  levels,
		Render::TextureFormat format
	)
	{
		Vector<unsigned char> out;
		unsigned int gl_internal_format = 0, gl_base_format = 0;
		ktx_gl_from_format(format, gl_internal_format, gl_base_format);
		if (!levels_data || !width || !height || !levels || !gl_internal_format) return out;
		out.reserve(KTX_HEADER_SIZE + levels_bytes(format, width, height, levels) + levels * 4);
		out.insert(out.end(), KTX_IDENTIFIER, KTX_IDENTIFIER + sizeof(KTX_IDENTIFIER));
		write_u32(out, KTX_ENDIANNESS);
		write_u32(out, 0);                  //gl type (compressed: 0)
		write_u32(out, 1);                  //gl type size
		write_u32(out, 0);                  //gl format (compressed: 0)
		write_u32(out, gl_internal_format);
		write_u32(out, gl_base_format);
		write_u32(out, (unsigned int)width);
		write_u32(out, (unsigned int)height);
		write_u32(out, 0);                  //depth
		write_u32(out, 0);                  //array elements
		write_u32(out, 1);                  //faces
		write_u32(out, levels);
		write_u32(out, 0);                  //key/value data
		//levels: the size, the blocks (8 or 16 bytes: always 4 bytes aligned)
		const unsigned char* level_data = levels_data;
		for (unsigned int level = 0; level < levels; ++level)
		{
			const unsigned int level_size = Render::texture_level_bytes(format, Render::texture_level_size((unsigned int)width, level), Render::texture_level_size((unsigned int)height, level));
			write_u32(out, level_size);
			out.insert(out.end(), level_data, level_data + level_size);
			level_data += level_size;
		}
		return out;
	}
}
}
