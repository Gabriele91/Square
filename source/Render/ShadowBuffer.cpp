//
//  GBuffer.cpp
//  Square
//
//  Created by Gabriele Di Bari on 15/06/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/System/RenderSystem.h"
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
    
    bool ShadowBuffer::build( const IVec2& size, const Type& type, unsigned int layers)
    {
        //delete if target
        if(m_target) destoy();
        //
        if(auto render = System::get<RenderSystem>(context())->render())
        {
            //save size
            m_size = size;
            m_type = type;
            m_layers = std::max(layers, 1u);
			//texture data
			Render::TextureRawDataInformation raw_data
			{
				TF_DEPTH_COMPONENT32,
				width(),
				height(),
				nullptr,
				TT_DEPTH,
				TTF_FLOAT,
				false
			};
			Render::TextureGpuDataInformation gpu_data
			{
				TMIN_NEAREST, 
				TMAG_NEAREST,
				//========
				TEDGE_CLAMP,
				TEDGE_CLAMP,
				TEDGE_CLAMP,
				//========
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
				m_texture = render->create_texture_array(raw_data, gpu_data, m_layers);
			break;
			default: return false;
			}
			//fail to build texture
			if (!m_texture) return false;
            //target: all of it; each of its layers, a driver that draws them one by one
			m_target = render->create_render_target({ Render::TargetField{ m_texture, Render::RT_DEPTH } });
			if (System::get<RenderSystem>(context())->layers_by_target() && m_type != SB_TEXTURE_2D)
			{
				m_layer_targets.reserve(target_layers());
				for (unsigned int layer = 0; layer != target_layers(); ++layer)
				{
					m_layer_targets.push_back(render->create_render_target({ Render::TargetField{ m_texture, Render::RT_DEPTH, int(layer) } }));
				}
			}
        }
        return m_target != nullptr;
    }
    void ShadowBuffer::destoy()
    {
        if(auto render = System::get<RenderSystem>(context())->render())
        {
            if (m_target)  render->delete_render_target(m_target);
			for (Render::Target*& layer_target : m_layer_targets)
			{
				if (layer_target)
				{
					render->delete_render_target(layer_target);
				}
			}
			if (m_texture) render->delete_texture(m_texture);
        }
        //to null
        m_target = nullptr;
        m_texture = nullptr;
        m_layer_targets.clear();
        m_size = {0,0};
    }
    
    Render::Target* ShadowBuffer::target() const
    {
        return m_target;
    }

    Render::Target* ShadowBuffer::target(unsigned int layer) const
    {
        Render::Target* out = m_target;
        if (layer < m_layer_targets.size())
        {
            out = m_layer_targets[layer];
        }
        return out;
    }

    unsigned int ShadowBuffer::target_layers() const
    {
        switch (m_type)
        {
        case SB_TEXTURE_CUBE: return 6;
        case SB_TEXTURE_CSM:  return m_layers;
        case SB_TEXTURE_2D:
        default:              return 1;
        }
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

    unsigned int ShadowBuffer::layers() const
    {
    	return m_layers;
    }

    ShadowBuffer::Type ShadowBuffer::type() const
    {
    	return m_type;
    }
}
}
