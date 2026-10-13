//  Square
//
//  Created by Gabriele on 15/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/System/RenderSystem.h"
#include "Square/Core/Filesystem.h"
#include "Square/Data/Image.h"
#include "Square/Data/ParserTexture.h"
#include "Square/Core/Context.h"
#include "Square/Core/Application.h"
#include "Square/Resource/Texture.h"
#include "Square/Core/ClassObjectRegistration.h"

namespace Square
{
namespace Resource
{
    //////////////////////////////////////////////////////////////
    //Add element to objects
    SQUARE_CLASS_OBJECT_REGISTRATION(Texture);
    //Registration in context
    void Texture::object_registration(Context& ctx)
    {
        //factory
        ctx.add_resource<Texture>({ ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".dds", ".ktx", ".sqtex", ".sampler" });
        //attributes
		#if 0
        ctx.add_attribute_function<Texture, unsigned long>
                                  ("width"
                                   , (unsigned long)(0)
                                   , [](const Texture* tex) -> unsigned long{ return tex->get_width(); }
                                   , [](Texture* tex, const unsigned long& ){ /*none*/ }
                                   );
        ctx.add_attribute_function<Texture, unsigned long>
                                    ("height"
                                     , (unsigned long)(0)
                                     , [](const Texture* tex) -> unsigned long { return tex->get_height(); }
                                     , [](Texture* actor, const unsigned long& ){ /*none*/ }
                                     );
		#endif
        //end
    }
    
	//////////////////////////////////////////////////////////////
	// Attributes

	Texture::Attributes::Attributes
	(
		Render::TextureMinFilterType  min_filter,
		Render::TextureMagFilterType  mag_filter,
		Render::TextureEdgeType  wrap_s,
		Render::TextureEdgeType  wrap_t,
		Render::TextureEdgeType  wrap_r,
		bool build_mipmap,
		int anisotropic
	)
	{
		m_min_filter = min_filter;
		m_mag_filter = mag_filter;
		m_wrap_s = wrap_s;
		m_wrap_t = wrap_t;
		m_wrap_r = wrap_r;
		m_build_mipmap = build_mipmap;
		m_anisotropic = anisotropic;
	}
	//////////////////////////////////////////////////////////////
	// Texture
	Texture::Texture(Context& context) : ResourceObject(context), BaseInheritableSharedObject(context.allocator())
	{
	}

	Texture::~Texture()
	{
		destoy();
	}

	Texture::Texture(Context& context, const std::string& path) : ResourceObject(context), BaseInheritableSharedObject(context.allocator())

	{
		load(path);
	}

	Texture::Texture
	(
		Context& context,
		const Attributes& attr,
		const std::string& path
	) : ResourceObject(context), BaseInheritableSharedObject(context.allocator())
	{
		load(attr, path);
	}

	Texture::Texture
	(
		Context& context,
		const Attributes& attr,
		const unsigned char* buffer,
		unsigned long width,
		unsigned long height,
		Render::TextureFormat format,
		Render::TextureType   type
	) : ResourceObject(context), BaseInheritableSharedObject(context.allocator())
	{
		build(attr, buffer, width, height, format, type);
	}

	Texture::Texture
	(   
		Context& context,
		const Attributes& attr,
		const std::vector< unsigned char >& buffer,
		unsigned long width,
		unsigned long height,
		Render::TextureFormat format,
		Render::TextureType   type
	) : ResourceObject(context), BaseInheritableSharedObject(context.allocator())
	{
		build(attr, buffer, width, height, format, type);
	}
	
	bool Texture::load(const std::string& path)
	{
		//get ext
		std::string ext = Filesystem::get_extension(path);
		// sqtex
		if (ext == ".sqtex" || ext == ".sampler")
		{
			Parser::Texture::Context tex_context;
			std::vector<unsigned char> data = Filesystem::binary_file_read_all(path);
			// The parser reads it as a C string
			data.push_back('\0');
			// Set default attrs
			tex_context.m_attributes =
			{
				Render::TMIN_LINEAR_MIPMAP_LINEAR,
				Render::TMAG_LINEAR,
				Render::TEDGE_REPEAT,
				Render::TEDGE_REPEAT,
				Render::TEDGE_REPEAT,
				true,
				1
			};
			// Parse texture data
			if (Parser::Texture::parse(tex_context, reinterpret_cast<const char*>(data.data()), data.size() - 1))
			{
				if (std::holds_alternative<std::string>(tex_context.m_image))
				{
					auto image_path = Filesystem::join(Filesystem::get_directory(path), std::get<std::string>(tex_context.m_image));
					// Load default texture
					if (load(tex_context.m_attributes, image_path))
					{ 
						return true;
					}
					// Fallback: an image for a GPU without the format of the first (DDS/KTX)
					if (tex_context.m_fallback.empty())
					{ 
						return false;
					}
					auto fallback_path = Filesystem::join(Filesystem::get_directory(path), tex_context.m_fallback);
					context().logger()->info("Texture: " + path + ", the GPU uses " + tex_context.m_fallback);
					// Load fallback
					return load(tex_context.m_attributes, fallback_path);
				}
				else if (std::holds_alternative< std::vector<unsigned char> >(tex_context.m_image))
				{
					return load(tex_context.m_attributes, std::get< std::vector<unsigned char> >(tex_context.m_image));
				}
				else
				{
					context().logger()->warning("Texture: " + path + "\nInvalid image");
					return false;
				}
			}
			else
			{
				context().logger()->warning("Texture: " + path + "\n" + tex_context.errors_to_string());
				return false;
			}
		}
		else
		{
			return load(
			{
				Render::TMIN_LINEAR_MIPMAP_LINEAR,
				Render::TMAG_LINEAR,
				Render::TEDGE_REPEAT,
				Render::TEDGE_REPEAT,
				Render::TEDGE_REPEAT,
				true,
				1
			}, path);
		}
	}
	
	bool Texture::load
	(
		const Attributes& attr,
		const std::string& path
	)
	{
		//compressed: DDS (BC), KTX (ASTC, BC); false when the GPU has not its format (a .sqtex
		//can give a fallback image)
		const std::string ext = Filesystem::get_extension(path);
		if (ext == ".dds" || ext == ".ktx")
		{
			if (load_compressed(attr, Filesystem::binary_file_read_all(path))) return true;
			context().logger()->warning("Texture: " + path + "\nFormat not supported by the GPU");
			return false;
		}
		//decode
		std::vector<unsigned char> image;
		unsigned long image_width = 0;
		unsigned long image_height = 0;
		Render::TextureFormat image_format;
		Render::TextureType   image_type;
		//load
		if (!Data::Image::load(path, image, image_width, image_height, image_format, image_type))
			return false;
		//build
		return build(attr, image.data(), image_width, image_height, image_format, image_type);
	}

	bool Texture::load
	(
		const Attributes& attr,
		const std::vector< unsigned char >& data_file
	)
	{
		//compressed: DDS (BC), KTX (ASTC, BC)
		if (Data::Image::is_compressed(data_file)) return load_compressed(attr, data_file);
		//decode
		std::vector<unsigned char> image;
		unsigned long image_width = 0;
		unsigned long image_height = 0;
		Render::TextureFormat image_format;
		Render::TextureType   image_type;
		//load
		if (!Data::Image::load(data_file, image, image_width, image_height, image_format, image_type))
			return false;
		//build
		return build(attr, image.data(), image_width, image_height, image_format, image_type);
	}

	bool Texture::build
	(
		const Attributes& attr,
		const std::vector< unsigned char >& buffer,
		unsigned long width,
		unsigned long height,
		Render::TextureFormat format,
		Render::TextureType   type
	)
	{
		return build(attr, buffer.data(), width, height, format, type);
	}

	static size_t get_pixel_size_from_format(Render::TextureFormat format)
	{
		switch (format)
		{
		case Square::Render::TF_RGBA8:     return 4;

		case Square::Render::TF_RGBA16F:
		case Square::Render::TF_RGBA16I:
		case Square::Render::TF_RGBA16UI:  return 4 * 2;

		case Square::Render::TF_RGBA32F:
		case Square::Render::TF_RGBA32I:
		case Square::Render::TF_RGBA32UI:  return 4 * 4;

		case Square::Render::TF_RGB8:      return 3;

		case Square::Render::TF_RGB16F:
		case Square::Render::TF_RGB16I:
		case Square::Render::TF_RGB16UI:   return 3 * 2;

		case Square::Render::TF_RGB32F:
		case Square::Render::TF_RGB32I:
		case Square::Render::TF_RGB32UI:   return 3 * 4;

		case Square::Render::TF_RG8:	   return 2;


		case Square::Render::TF_RG16F:
		case Square::Render::TF_RG16I:
		case Square::Render::TF_RG16UI:	   return 2 * 2;

		case Square::Render::TF_RG32F:
		case Square::Render::TF_RG32I:
		case Square::Render::TF_RG32UI:	   return 2 * 4;

		case Square::Render::TF_R8:		   return 1;

		case Square::Render::TF_R16F:
		case Square::Render::TF_R16I:
		case Square::Render::TF_R16UI:	   return 1 * 2;

		case Square::Render::TF_R32F:
		case Square::Render::TF_R32I:
		case Square::Render::TF_R32UI:	   return 1 * 4;

		case Square::Render::TF_RGBA4:
		case Square::Render::TF_RGB5A1:
		case Square::Render::TF_RGB565:	   return 2;


		case Square::Render::TF_DEPTH16_STENCIL8:  return 3;
		case Square::Render::TF_DEPTH24_STENCIL8:  return 4;
		case Square::Render::TF_DEPTH32_STENCIL8:  return 5;

		case Square::Render::TF_DEPTH_COMPONENT16:  return 2;
		case Square::Render::TF_DEPTH_COMPONENT24:  return 3;
		case Square::Render::TF_DEPTH_COMPONENT32:  return 4;

		default: return 0;
		}
	}

	bool Texture::build
	(
		const Attributes& attr,
		const unsigned char* buffer,
		unsigned long width,
		unsigned long height,
		Render::TextureFormat format,
		Render::TextureType   type
	)
	{
		//save size
		m_width = width;
		m_height = height;
		m_format = format;
		m_type = type;
		m_attributes = attr;
		//create texture
		if (auto render = System::get<RenderSystem>(context())->render())
		{
			m_ctx_texture =
			render->create_texture
			({
				format,
				(unsigned int)m_width,
				(unsigned int)m_height,
				buffer,
				type,
				Render::TTF_UNSIGNED_BYTE,
				false
			},
			{
				attr.m_min_filter,           // min_type
				attr.m_mag_filter,           // mag_type
				attr.m_wrap_s,               // edge_s
				attr.m_wrap_t,               // edge_t
				attr.m_wrap_r,               // edge_r
				attr.m_build_mipmap,         // build_mipmap
				0,                           // mipmap_min
				attr.m_build_mipmap ? 10:0,  // mipmap_max
				attr.m_anisotropic,          // anisotropy
				false                        // read_from_cpu
			});
		}

		//ok
		return m_ctx_texture != nullptr;
	}

	bool Texture::update(const unsigned char* buffer)
	{
		if (!m_ctx_texture || !buffer) return false;
		auto render = System::get<RenderSystem>(context())->render();
		if (!render) return false;
		render->update_texture
		(
			m_ctx_texture,
			{ m_format, (unsigned int)m_width, (unsigned int)m_height, buffer, m_type, Render::TTF_UNSIGNED_BYTE, false }
		);
		return true;
	}

	bool Texture::load_compressed
	(
		const Attributes& attr,
		const std::vector< unsigned char >& data_file
	)
	{
		std::vector<unsigned char> levels_data;
		unsigned long image_width = 0;
		unsigned long image_height = 0;
		unsigned int  image_levels = 0;
		Render::TextureFormat image_format = Render::TF_INVALID;
		if (!Data::Image::load_compressed(data_file, levels_data, image_width, image_height, image_levels, image_format))
			return false;
		return build_compressed(attr, levels_data.data(), image_width, image_height, image_levels, image_format);
	}

	bool Texture::build_compressed
	(
		const Attributes& attr,
		const unsigned char* levels_data,
		unsigned long width,
		unsigned long height,
		unsigned int  levels,
		Render::TextureFormat format
	)
	{
		auto render = System::get<RenderSystem>(context())->render();
		if (!render || !Render::is_compressed_format(format) || !levels) return false;
		//the GPU samples it?
		const Render::RenderDriverInfo& info = render->get_render_driver_info();
		if (format == Render::TF_ASTC_4x4 ? !info.m_texture_astc : !info.m_texture_bc) return false;
		//a new texture
		destoy();
		m_width = width;
		m_height = height;
		m_format = format;
		m_type = format == Render::TF_BC4 ? Render::TT_R : format == Render::TF_BC5 ? Render::TT_RG : Render::TT_RGBA;
		m_attributes = attr;
		//the levels of the file (no mipmaps built: without levels, a mipmap filter samples the first)
		Render::TextureRawDataInformation data
		{
			format,
			(unsigned int)m_width,
			(unsigned int)m_height,
			levels_data,
			m_type,
			Render::TTF_UNSIGNED_BYTE,
			false
		};
		data.m_levels = levels;
		m_ctx_texture = render->create_texture
		(
			data,
			{
				attr.m_min_filter,           // min_type
				attr.m_mag_filter,           // mag_type
				attr.m_wrap_s,               // edge_s
				attr.m_wrap_t,               // edge_t
				attr.m_wrap_r,               // edge_r
				false,                       // build_mipmap
				0,                           // mipmap_min
				int(levels) - 1,             // mipmap_max
				attr.m_anisotropic,          // anisotropy
				false                        // read_from_cpu
			}
		);
		return m_ctx_texture != nullptr;
	}

	Render::Texture* Texture::get_context_texture() const
	{
		return m_ctx_texture;
	}

	unsigned long Texture::get_width() const
	{
		return m_width;
	}

	unsigned long Texture::get_height() const
	{
		return m_height;
	}

	Render::TextureFormat Texture::get_format() const
	{
		return m_format;
	}

	Render::TextureType Texture::get_type() const
	{
		return m_type;
	}

	const Texture::Attributes& Texture::get_attributes() const
	{
		return m_attributes;
	}

	void Texture::destoy()
	{
		if (m_ctx_texture)
			if (auto render = System::get<RenderSystem>(context())->render())
				render->delete_texture(m_ctx_texture);
		m_ctx_texture = nullptr;
	}

}
}
