//
//  RenderCore.h
//  Square
//
//  Created by Gabriele Di Bari on 10/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Render/Queue.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Light.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/LevelOfDetail.h"
#include "Square/Render/Occluder.h"

namespace Square
{
namespace Render
{
	class Material;
	class SoftwareOcclusion;
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
        using Cameras = std::vector < Weak<Camera> >;
        using Lights = std::vector < Weak<Light> >;
		using Renderables = std::vector < Weak<Renderable> >;
		using LevelsOfDetail = std::vector < Weak<LevelOfDetail> >;
		using Occluders = std::vector < Weak<Occluder> >;
        Cameras m_cameras;
		Lights m_lights;
		Renderables m_renderables;
		//the groups of levels of detail: their level selected for each camera (before its queues)
		LevelsOfDetail m_levels_of_detail;
		//what hides what is behind it (the software occlusion of each camera)
		Occluders m_occluders;
        //clear
        void clear();        
    };
    
    class SQUARE_API CollectionQuery
    {
    public:
        //compue queues
        static void lights(const Collection& collection, PoolQueues& queues, const Geometry::Frustum& view_frustum);
        static void lights(const Collection& collection, PoolQueues& queues, const Geometry::Sphere&  in_sphere);
        static void lights(const Collection& collection, PoolQueues& queues, const Camera&  in_camera);
        
        static void renderables(const Collection& collection, PoolQueues& queues, const Vec3& position);
        static void renderables(const Collection& collection, PoolQueues& queues, const Geometry::Frustum& view_frustum);
        //the ones in the frustum not hidden by the occluders (nullptr: none hidden)
        static void renderables(const Collection& collection, PoolQueues& queues, const Geometry::Frustum& view_frustum, const SoftwareOcclusion* occlusion);
        static void renderables(const Collection& collection, PoolQueues& queues, const Geometry::Sphere& in_sphere);
        static void renderables(const Collection& collection, PoolQueues& queues, const Camera&  in_camera);
        static void renderables(const Collection& collection, PoolQueues& queues, const Camera&  in_camera, const SoftwareOcclusion* occlusion);

		static void opaque_renderables(const Collection& collection, PoolQueues& queues, const Vec3& position);
		static void opaque_renderables(const Collection& collection, PoolQueues& queues, const Geometry::Frustum& view_frustum);
		static void opaque_renderables(const Collection& collection, PoolQueues& queues, const Geometry::Sphere& in_sphere);
        static void renderables(const Collection& collection, PoolQueues& queues, const Light&  in_light);

    };
}
}
