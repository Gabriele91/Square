//
//  ShadowBuffer.h
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
    class SQUARE_API ShadowBuffer : public Object
	{

	public:

		SQUARE_OBJECT(ShadowBuffer)
		
		enum Type
		{
			SB_TEXTURE_2D,
			SB_TEXTURE_CUBE,
			SB_TEXTURE_CSM
		};

		ShadowBuffer(Square::Context& context);
		ShadowBuffer
		(
			  Square::Context& context
			, const IVec2& size
			, const Type& type
		);
        virtual ~ShadowBuffer();
		
		bool build(
			  const IVec2& size
			, const Type& type
		);
        void destoy();
        
        Render::Target* target() const;

		Render::Texture* texture() const;

		unsigned int width() const;

		unsigned int height() const;

		const IVec2& size() const;

	protected:

		IVec2 m_size;
		Render::Target*  m_target{ nullptr };
		Render::Texture* m_texture{ nullptr };

	};
}
}