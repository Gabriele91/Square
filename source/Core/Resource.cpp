//
//  Square
//
//  Created by Gabriele on 12/07/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Square/Core/Resource.h"
#include "Square/Core/Context.h"


namespace Square
{
	ResourceObject::ResourceObject(Context& context) : Object(context), BaseInheritableSharedObject(context.allocator()) {}
}
