//
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright ? 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Render.h"
#include "Square/Resource/Interface.h"

namespace Square
{
namespace Resource
{
	class SQUARE_API Texture : public SharedObject<Texture>, public Interface
	{

		Render::Texture*      m_ctx_texture{ nullptr };
		unsigned long         m_width{ 0 };
		unsigned long         m_height{ 0 };
		Render::TextureFormat m_format;
		Render::TextureType   m_type;

	public:

		struct SQUARE_API Attributes
		{
			Render::TextureType   m_type{ Render::TT_RGB };
			Render::TextureFormat m_format{ Render::TF_RGB8 };
			Render::TextureMinFilterType m_min_filter{ Render::TMIN_NEAREST };
			Render::TextureMagFilterType m_mag_filter{ Render::TMAG_NEAREST };
			bool m_clamp_to_border{ false };
			bool m_build_mipmap   { false };
			bool alpha_channel();

			Attributes(
				Render::TextureType   type_image,
				Render::TextureFormat format_image,
				Render::TextureMinFilterType  min_filter = Render::TMIN_NEAREST,
				Render::TextureMagFilterType  mag_filter = Render::TMAG_NEAREST,
				bool clamp_to_border = false,
				bool build_mipmap = false
			);

			static Attributes rgba(
				Render::TextureMinFilterType  min_filter = Render::TMIN_NEAREST,
				Render::TextureMagFilterType  mag_filter = Render::TMAG_NEAREST,
				bool clamp_to_border = false,
				bool build_mipmap = false
			);

			static Attributes rgba_linear(
				bool clamp_to_border = false,
				bool alpha_channel = true
			);

			static Attributes rgba_linear_mipmap_linear(
				bool clamp_to_border = false,
				bool alpha_channel = true
			);
		};

		Texture();

		Texture(const std::string& path);

		Texture(const Attributes& attr,
			    const std::string& path);

		Texture
		(
			const Attributes& attr,
			const unsigned char* buffer,
			unsigned long  width,
			unsigned long  height,
			Render::TextureFormat format,
			Render::TextureType   type
		);

		Texture
		(
			const Attributes& attr,
			const std::vector< unsigned char >& buffer,
			unsigned long width,
			unsigned long height,
			Render::TextureFormat format,
			Render::TextureType   type
		);

		virtual ~Texture();

		bool load(Manager& resources, const std::string& path);

		bool load(const std::string& path);

		bool load(const Attributes& attr, const std::string& path);


		bool build
		(
			const Attributes& attr,
			const unsigned char* buffer,
			unsigned long width,
			unsigned long height,
			Render::TextureFormat format,
			Render::TextureType   type
		);

		bool build
		(
			const Attributes& attr,
			const std::vector< unsigned char >& buffer,
			unsigned long width,
			unsigned long height,
			Render::TextureFormat format,
			Render::TextureType   type
		);

		Render::Texture* get_context_texture() const;
		unsigned long    get_width() const;
		unsigned long    get_height() const;
		Render::TextureFormat get_format() const;
		Render::TextureType	  get_type() const;

		void destoy();

	};
}
}
