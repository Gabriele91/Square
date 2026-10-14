//
//  ShadowBuffer.h
//  Square
//
//  Created by Gabriele Di Bari on 15/06/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <vector>
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
		
		//layers: of a CSM (its cascades)
		bool build(
			  const IVec2& size
			, const Type& type
			, unsigned int layers = 1
		);
        void destoy();
        
        Render::Target* target() const;
		//the target of one of its layers (a face of a cube, a cascade): made when the driver draws
		//the layers one by one (RenderSystem::layers_by_target), else the one of all of them
		Render::Target* target(unsigned int layer) const;
		//its layers as targets: the faces of a cube, the cascades of a CSM, one of a 2D
		unsigned int target_layers() const;

		Render::Texture* texture() const;

		unsigned int width() const;

		unsigned int height() const;

		const IVec2& size() const;

		unsigned int layers() const;

		Type type() const;

	protected:

		IVec2 m_size;
		Type m_type{ SB_TEXTURE_2D };
		unsigned int m_layers{ 1 };
		Render::Target*  m_target{ nullptr };
		Vector<Render::Target*> m_layer_targets;
		Render::Texture* m_texture{ nullptr };

	};
}
}