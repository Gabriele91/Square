//
//  RenderCore.h
//  Square
//
//  Created by Gabriele Di Bari on 10/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Render/Queue.h"
#include "Square/Render/Light.h"
#include "Square/Render/Renderable.h"

namespace Square
{
namespace Render
{
	class Material;
}
namespace Geometry
{
	class Sphere;
	class Frustum;
}
}

namespace Square
{
namespace Render
{
    class SQUARE_API Collection
    {
    public:
        //all objects
		using Lights = std::vector < Weak<Light> >;
		using Renderables = std::vector < Weak<Renderable> >;
		Lights m_lights;
		Renderables m_renderables;
        //compue queues
        void compute_lights_queues(PoolQueues& queues, const Geometry::Frustum& view_frustum);
        //compue queues
        void compute_no_lights_queues(PoolQueues& queues, const Geometry::Frustum& view_frustum);
        void compute_no_lights_queues(PoolQueues& queues, const Geometry::Sphere& in_sphere);
        //clear
        void clear();        
    };
}
}
