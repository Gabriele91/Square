//  Square
//
//  Created by Gabriele on 15/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Core/Filesystem.h"
#include "Square/Data/Image.h"
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
        ctx.add_resource<Texture>({ ".png", ".jpg", ".jpeg", ".tga" });
        //attributes
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
        //end
    }
    
	//////////////////////////////////////////////////////////////
	// Attributes
	Texture::Attributes::Attributes(
		Render::TextureType   type_image,
		Render::TextureFormat format_image,
		Render::TextureMinFilterType  min_filter,
		Render::TextureMagFilterType  mag_filter,
		bool clamp_to_border,
		bool build_mipmap
	)
	{
		m_type = type_image;
		m_format = format_image;
		m_min_filter = min_filter;
		m_mag_filter = mag_filter;
		m_clamp_to_border = clamp_to_border;
		m_build_mipmap = build_mipmap;
	}

	bool Texture::Attributes::alpha_channel()
	{
		return m_type == Render::TT_RGBA;
	}

	Texture::Attributes Texture::Attributes::rgba(
		Render::TextureMinFilterType  min_filter,
		Render::TextureMagFilterType  mag_filter,
		bool clamp_to_border,
		bool build_mipmap
	)
	{
		return
		{
			Render::TT_RGBA,
			Render::TF_RGBA8,
			min_filter,
			mag_filter,
			clamp_to_border,
			build_mipmap
		};
	}

	Texture::Attributes Texture::Attributes::rgba_linear(
		bool clamp_to_border,
		bool alpha_channel
	)
	{
		return
		{
			Render::TT_RGBA,
			Render::TF_RGBA8,
			Render::TMIN_LINEAR,
			Render::TMAG_LINEAR,
			clamp_to_border,
			false
		};
	}

	Texture::Attributes Texture::Attributes::rgba_linear_mipmap_linear(
		bool clamp_to_border,
		bool alpha_channel
	)
	{
		return
		{
			Render::TT_RGBA,
			Render::TF_RGBA8,
			Render::TMIN_LINEAR_MIPMAP_LINEAR,
			Render::TMAG_LINEAR,
			clamp_to_border,
			true
		};
	}
	//////////////////////////////////////////////////////////////
	// Texture
	Texture::Texture(Context& context) : Object(context), ResourceObject(context), SharedObject<Texture>(context.allocator())
	{
	}

	Texture::~Texture()
	{
		destoy();
	}

	Texture::Texture(Context& context, const std::string& path) : Object(context), ResourceObject(context), SharedObject<Texture>(context.allocator())

	{
		load(path);
	}

	Texture::Texture
	(
		Context& context,
		const Attributes& attr,
		const std::string& path
	) : Object(context), ResourceObject(context), SharedObject<Texture>(context.allocator())
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
	) : Object(context), ResourceObject(context), SharedObject<Texture>(context.allocator())
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
	) : Object(context), ResourceObject(context), SharedObject<Texture>(context.allocator())
	{
		build(attr, buffer, width, height, format, type);
	}
	
	bool Texture::load(const std::string& path)
	{
		return load(Attributes::rgba_linear_mipmap_linear(), path);
	}
	
	bool Texture::load
	(
		const Attributes& attr,
		const std::string& path
	)
	{
		//get ext
		std::string ext = Filesystem::get_extension(path);
		//test
		if (ext != ".png"
		&&  ext != ".jpg"
		&&  ext != ".jpeg"
		&&  ext != ".tga") return false;
		//decode
		std::vector<unsigned char> image;
		unsigned long image_width = 0;
		unsigned long image_height = 0;
		Render::TextureFormat image_format;
		Render::TextureType   image_type;
		//load
		if (!Data::Image::load(path, image, image_width, image_height, image_format, image_type))
			return false;
		//delete alpha channel from attributes (if needed) :TODO:
		Attributes modattr = attr;
		modattr.m_type = image_type;
		modattr.m_format = image_format;
		//build
		return build(modattr, image.data(), image_width, image_height, image_format, image_type);
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
		//create texture
		if (auto render = context().render())
		{
			m_ctx_texture =
			render->create_texture
			({
				format,
				(unsigned int)m_width,
				(unsigned int)m_height,
				buffer,
				type,
				Render::TTF_UNSIGNED_BYTE
			},
			{
				attr.m_min_filter,
				attr.m_mag_filter,
				attr.m_clamp_to_border ? Render::TEDGE_CLAMP : Render::TEDGE_REPEAT,
				attr.m_clamp_to_border ? Render::TEDGE_CLAMP : Render::TEDGE_REPEAT,
				attr.m_clamp_to_border ? Render::TEDGE_CLAMP : Render::TEDGE_REPEAT,
				attr.m_build_mipmap
			});
		}

		//ok
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

	void Texture::destoy()
	{
		if (m_ctx_texture)
			if (auto render = context().render())
				render->delete_texture(m_ctx_texture);
		m_ctx_texture = nullptr;
	}
}
}
