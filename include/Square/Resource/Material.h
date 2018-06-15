//
//  Effect.h
//  Square
//
//  Created by Gabriele Di Bari on 09/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

#include "Square/Config.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/Resource.h"
#include "Square/Render/Queue.h"
#include "Square/Render/Material.h"

namespace Square
{
namespace Resource
{
    class SQUARE_API Material :   public ResourceObject
								, public SharedObject<Material>
								, public Uncopyable
								, public Render::Material
		{
        
    public:
        
        //Init object
        SQUARE_OBJECT(Material)

        //Registration in context
        static void object_registration(Context& ctx);
        
        //constructor
		Material(Context& context);
		Material(Context& context, const std::string& path);
        
        //load shader
        bool load(const std::string& path) override;    
        
    };

}
}