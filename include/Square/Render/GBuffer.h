//
//  GBuffer.h
//  Square
//
//  Created by Gabriele Di Bari on 15/06/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Object.h"
#include "Square/Driver/Render.h"

namespace Square
{
namespace Render
{
    class SQUARE_API GBuffer : public Object
    {
    public:
        
        SQUARE_OBJECT(GBuffer)
        
        struct BufferFormat
        {
            TextureFormat       m_format = TF_RGBA32F;
            TextureType         m_type = TT_RGBA;
            TextureTypeFormat   m_type_format = TTF_FLOAT;
            TargetType          m_type_target = RT_COLOR;
            
            BufferFormat() {}
            
            BufferFormat(TextureFormat     format,
                         TextureType       type,
                         TextureTypeFormat type_format,
                         TargetType        type_target)
            {
                m_format     = format;
                m_type        = type;
                m_type_format = type_format;
                m_type_target = type_target;
            }
        };
        
        
        GBuffer(Square::Context& context);
        GBuffer(Square::Context& context, const IVec2& size, const std::vector<BufferFormat>& buffer_list);
        virtual ~GBuffer();
        
        bool build( const IVec2& size, const std::vector<BufferFormat>& buffer_list );
        void destoy();
        
        Render::Target* target() const;

        Render::Texture* texture(size_t idtexture) const;
        
        unsigned int width() const;
        
        unsigned int height() const;
        
        const IVec2& size() const;
        
        size_t number_of_buffers() const;

    private:
        
        IVec2 m_size;
        Render::Target*               m_target{ nullptr };
        std::vector<Render::Texture*> m_textures;
    };
}
}
