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
#include "Square/Core/SmartPointers.h"
#include "Square/Math/Linear.h"
#include "Square/Geometry/Frustum.h"
#include "Square/Geometry/Sphere.h"
#include "Square/Render/Queue.h"

namespace Square
{
namespace Render
{
    class SQUARE_API Collection
    {
    public:
        //all objects
        using Objects = std::vector < Weak<Object> >;
        Objects m_lights;
        Objects m_renderable;
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
