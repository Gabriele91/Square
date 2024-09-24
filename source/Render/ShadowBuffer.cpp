//
//  GBuffer.cpp
//  Square
//
//  Created by Gabriele Di Bari on 15/06/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Context.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Light.h"
#include "Square/Render/ShadowBuffer.h"

namespace Square
{
namespace Render
{
	ShadowBuffer::ShadowBuffer(Square::Context& context)
    : Object(context) 
	{
		//to null
		m_target = nullptr;
		m_texture = nullptr;
		m_size = { 0,0 };
	}
    
	ShadowBuffer::ShadowBuffer(
		   Square::Context& context
		, const IVec2& size
		, const ShadowBuffer::Type& type
	)
    : Object(context)
    {
        build(size, type);
    }
    
	ShadowBuffer::~ShadowBuffer(){ destoy(); }
    
    bool ShadowBuffer::build( const IVec2& size, const Type& type)
    {
        //delete if target
        if(m_target) destoy();
        //
        if(auto render = context().render())
        {
            //save size
            m_size = size;
			//texture data
			Render::TextureRawDataInformation raw_data
			{
				TF_DEPTH_COMPONENT32,
				width(),
				height(),
				nullptr,
				TT_DEPTH,
				TTF_FLOAT
			};
			Render::TextureGpuDataInformation gpu_data
			{
				//TMIN_NEAREST, 
				TMIN_LINEAR,
				//TMAG_NEAREST,
				TMAG_LINEAR,
				//========
				TEDGE_CLAMP,
				TEDGE_CLAMP,
				TEDGE_CLAMP,
				false,//no mipmap
				1,	  //mipmap_min
				1,    //mipmap_max
				0,
				false //no cpu access
			};
			//create textures
			switch (type)
			{
			case SB_TEXTURE_2D:
				m_texture = render->create_texture(raw_data, gpu_data);
			break;
			case SB_TEXTURE_CUBE:
			{
				Render::TextureRawDataInformation raw_cube[6]
				{
					raw_data, raw_data, raw_data,
					raw_data, raw_data, raw_data
				};
				m_texture = render->create_cube_texture(raw_cube, gpu_data);
			}
			break;
			case SB_TEXTURE_CSM:
				m_texture = render->create_texture_array(raw_data, gpu_data, DIRECTION_SHADOW_CSM_NUMBER_OF_FACES);
			break;
			default: return false;
			}
			//fail to build texture
			if (!m_texture) return false;
            //target
			m_target = render->create_render_target({ Render::TargetField{ m_texture, Render::RT_DEPTH } });
        }
        return m_target != nullptr;
    }
    void ShadowBuffer::destoy()
    {
        if(auto render = context().render())
        {
            if (m_target)  render->delete_render_target(m_target);
			if (m_texture) render->delete_texture(m_texture);
            //to null
            m_size = {0,0};
        }

    }
    
    Render::Target* ShadowBuffer::target() const
    {
        return m_target;
    }
    
    Render::Texture* ShadowBuffer::texture() const
    {
        return m_texture;
    }
    
    unsigned int ShadowBuffer::width() const
    {
        return m_size.x;
    }
    
    unsigned int ShadowBuffer::height() const
    {
        return m_size.y;
    }
    
    const IVec2& ShadowBuffer::size() const
    {
        return m_size;
    }
}
}
