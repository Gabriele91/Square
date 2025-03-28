//
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright ? 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Resource.h"
#include "Square/Driver/Render.h"

namespace Square
{
namespace Resource
{
	class SQUARE_API Texture : public ResourceObject
                             , public InheritableSharedObject<Texture>
	{
	public:
	
		struct SQUARE_API Attributes
		{
			Render::TextureMinFilterType m_min_filter{ Render::TMIN_NEAREST };
			Render::TextureMagFilterType m_mag_filter{ Render::TMAG_NEAREST };
			Render::TextureEdgeType m_wrap_s{ Render::TEDGE_CLAMP };
			Render::TextureEdgeType m_wrap_t{ Render::TEDGE_CLAMP };
			Render::TextureEdgeType m_wrap_r{ Render::TEDGE_CLAMP };
			bool m_build_mipmap { false };
			int  m_anisotropic{ 1 };

			Attributes(
				Render::TextureMinFilterType  min_filter = Render::TMIN_NEAREST,
				Render::TextureMagFilterType  mag_filter = Render::TMAG_NEAREST,
				Render::TextureEdgeType  wrap_s = Render::TEDGE_CLAMP,
				Render::TextureEdgeType  wrap_t = Render::TEDGE_CLAMP,
				Render::TextureEdgeType  wrap_r = Render::TEDGE_CLAMP,
				bool build_mipmap = false,
				int anisotropic = 1
			);
		};

        //Init object
        SQUARE_OBJECT(Texture)

        //Registration in context
        static void object_registration(Context& ctx);
        
        //Contructor
		Texture(Context& context);

		Texture(Context& context, const std::string& path);

		Texture(Context& context, 
			    const Attributes& attr,
			    const std::string& path);

		Texture
		(
			Context& context,
			const Attributes& attr,
			const unsigned char* buffer,
			unsigned long  width,
			unsigned long  height,
			Render::TextureFormat format,
			Render::TextureType   type
		);

		Texture
		(
			Context& context,
			const Attributes& attr,
			const std::vector< unsigned char >& buffer,
			unsigned long width,
			unsigned long height,
			Render::TextureFormat format,
			Render::TextureType   type
		);

		virtual ~Texture();

		bool load(const std::string& path) override;

		bool load(const Attributes& attr, const std::string& path);

		bool load(const Attributes& attr, const std::vector< unsigned char >& data_file);

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
		const Attributes& get_attributes() const;

		void destoy();

	protected:
		
		Render::Texture*      m_ctx_texture{ nullptr };
		unsigned long         m_width{ 0 };
		unsigned long         m_height{ 0 };
		Render::TextureFormat m_format;
		Render::TextureType   m_type;
		Attributes		      m_attributes;
	};
}
}
