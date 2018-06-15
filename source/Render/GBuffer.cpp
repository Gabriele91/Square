//
//  GBuffer.cpp
//  Square
//
//  Created by Gabriele Di Bari on 15/06/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Context.h"
#include "Square/Render/GBuffer.h"

namespace Square
{
namespace Render
{
    GBuffer::GBuffer(Square::Context& context)
    : Object(context) {}
    
    GBuffer::GBuffer(Square::Context& context, const IVec2& size, const std::vector<BufferFormat>& buffer_list)
    : Object(context)
    {
        build(size, buffer_list);
    }
    
    GBuffer::~GBuffer(){ destoy(); }
    
    bool GBuffer::build( const IVec2& size, const std::vector<BufferFormat>& buffer_list )
    {
        //delete if target
        if(m_target) destoy();
        //
        if(auto render = context().render())
        {
            //save size
            m_size = size;
            //type of texture
            std::vector<TargetField> targets;
            //create textures
            for (unsigned int i = 0; i != buffer_list.size(); i++)
            {
                m_textures.push_back(render->create_texture(
                 {
                     buffer_list[i].m_format,
                     width(),
                     height(),
                     nullptr,
                     buffer_list[i].m_type,
                     buffer_list[i].m_type_format
                 },
                 {
                     TMIN_NEAREST,
                     TMAG_NEAREST,
                     TEDGE_CLAMP,
                     TEDGE_CLAMP,
                     false
                 }
                 ));
                targets.push_back({m_textures.back(), buffer_list[i].m_type_target});
            }
            //target
            m_target = render->create_render_target(targets);
        }
        return m_target != nullptr;
    }
    void GBuffer::destoy()
    {
        if(auto render = context().render())
        {
            if (m_target) render->delete_render_target(m_target);
            
            for (unsigned int i = 0; i != m_textures.size(); i++)
            {
                if (m_textures[i]) render->delete_texture(m_textures[i]);
            }
            //to null
            m_size = {0,0};
        }

    }
    
    Render::Target* GBuffer::target() const
    {
        return m_target;
    }
    
    Render::Texture* GBuffer::texture(size_t idtexture) const
    {
        return m_textures[idtexture];
    }
    
    unsigned int GBuffer::width() const
    {
        return m_size.x;
    }
    
    unsigned int GBuffer::height() const
    {
        return m_size.y;
    }
    
    const IVec2& GBuffer::size() const
    {
        return m_size;
    }
    
    size_t GBuffer::number_of_buffers() const
    {
        return m_textures.size();
    }
}
}
