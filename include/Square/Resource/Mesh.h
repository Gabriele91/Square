//
//  Material.h
//  Square
//
//  Created by Gabriele Di Bari on 07/01/2024.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

#include "Square/Config.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/Resource.h"
#include "Square/Render/Queue.h"
#include "Square/Render/Mesh.h"

namespace Square
{
    namespace Resource
    {
        class SQUARE_API Mesh : public ResourceObject
                              , public Render::Mesh
                              , public SharedObject<Mesh>
                              , public Uncopyable
        {

        public:

            //Init object
            SQUARE_OBJECT(Mesh)

            //Registration in context
            static void object_registration(Context& ctx);

            //constructor
            Mesh(Context& context);
            Mesh(Context& context, const std::string& path);

            //load shader
            bool load(const std::string& path) override;

        };

    }
}