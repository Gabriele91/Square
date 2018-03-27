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
#include "Square/Resource/Geometry.h"

namespace Square
{
namespace Resource
{
	class SQUARE_API Mesh : public ResourceObject
                          , public SharedObject<Mesh>
	{
		//vector of surface
		std::vector< Shared< Geometry > > m_surfaces;

	public:

		//init
		Mesh(Context& context);

		//load		
		bool load(const std::string& path) override;

		//draw
		void draw();
	};
}
}