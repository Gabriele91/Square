////
//  Square
//
//  Created by Gabriele on 14/11/17.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Variant.h"
#include "Square/Core/Object.h"
#include "Square/Core/Context.h"
#include "Square/Data/Archive.h"
#include "Square/Core/Application.h"

namespace Square
{
namespace  Data
{
    SQUARE_API bool attribute_serialize(Archive& archivie, const Object* object, const std::vector < Attribute >* attrbutes);
    SQUARE_API bool attribute_deserialize(Archive& archivie, Object* object, const std::vector < Attribute >* attrbutes);
    
    //from context
    template < class T >
    inline bool serialize(Archive& archivie,const Shared< T > object)
    {
        return attribute_serialize(archivie, object.get(), archivie.context().attributes(object->object_id()));
    }
    inline bool serialize(Archive& archivie,const Object* object)
    {
        return attribute_serialize(archivie, object, archivie.context().attributes(object->object_id()));
    }
    template < class T >
    inline bool serialize(Archive& archivie,const std::vector< Shared < T > >& objects)
    {
        uint64 size = uint64(objects.size());
        archivie % size;
        bool success = true;
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        for(Shared < T >& object : objects)
        {
            success &= attribute_serialize(archivie, object, archivie.context().attributes(object->object_id()));
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        return success;
    }
    
    template < class T >
    inline bool deserialize(Archive& archivie,const Shared< T > object)
    {
        return attribute_deserialize(archivie, object.get(), archivie.context().attributes(object->object_id()));
    }
    inline bool deserialize(Archive& archivie, Object* object)
    {
        return attribute_deserialize(archivie, object, archivie.context().attributes(object->object_id()));
    }
}
}
