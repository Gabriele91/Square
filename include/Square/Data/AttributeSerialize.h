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
#include "Square/Data/Json.h"
#include "Square/Core/Application.h"

namespace Square
{
namespace  Data
{
    SQUARE_API bool attribute_serialize(Archive& archive, const Object* object, const std::vector < Attribute >* attrbutes);
    SQUARE_API bool attribute_deserialize(Archive& archive, Object* object, const std::vector < Attribute >* attrbutes);
    
    //from context
    template < class T >
    inline bool serialize(Archive& archive,const Shared< T > object)
    {
        return attribute_serialize(archive, object.get(), archive.context().attributes(object->object_id()));
    }
    inline bool serialize(Archive& archive,const Object* object)
    {
        return attribute_serialize(archive, object, archive.context().attributes(object->object_id()));
    }
    
    template < class T >
    inline bool deserialize(Archive& archive,const Shared< T > object)
    {
        return attribute_deserialize(archive, object.get(), archive.context().attributes(object->object_id()));
    }
    inline bool deserialize(Archive& archive, Object* object)
    {
        return attribute_deserialize(archive, object, archive.context().attributes(object->object_id()));
    }

    SQUARE_API bool attribute_serialize_json(JsonValue& archive, const Object* object, const std::vector < Attribute >* attrbutes);
    SQUARE_API bool attribute_deserialize_json(JsonValue& archive, Object* object, const std::vector < Attribute >* attrbutes);
    
    //from context
    template < class T >
    inline bool serialize_json(JsonValue& archive,const Shared< T > object)
    {
        return attribute_serialize_json(archive, object.get(), object->context().attributes(object->object_id()));
    }
    inline bool serialize_json(JsonValue& archive,const Object* object)
    {
        return attribute_serialize_json(archive, object, object->context().attributes(object->object_id()));
    }
    
    template < class T >
    inline bool deserialize_json(JsonValue& archive, const Shared< T > object)
    {
        return attribute_deserialize_json(archive, object.get(), object->context().attributes(object->object_id()));
    }
    inline bool deserialize_json(JsonValue& archive, Object* object)
    {
        return attribute_deserialize_json(archive, object, object->context().attributes(object->object_id()));
    }
}
}
