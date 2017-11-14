//
//  AttributeSerialize.cpp
//  Square
//
//  Created by Gabriele Di Bari on 14/11/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Data/AttributeSerialize.h"

namespace Square
{
namespace  Data
{
    SQUARE_API bool attribute_serialize(Archive& archivie, const Object* object, const std::vector < Attribute >* attrbutes)
    {
        //serialize
        for(const Attribute& attr : *attrbutes)
        if (attr.type() & Attribute::FILE)
        {
            //get
            VariantRef value;
            attr.get(object, value);
            //serialize
            archivie % value;
        }
        return true;
    }
    
    SQUARE_API bool attribute_deserialize(Archive& archivie, Object* object, const std::vector < Attribute >* attributes)
    {
        //deserialize
        for(const Attribute& attr : *attributes)
        if (attr.type() & Attribute::FILE)
        {
            //deserialize
            Variant value(attr.value_type());
            archivie % value.as_variant_ref();
            //set
            attr.set(object, value.as_variant_ref());
        }
        return true;
    }
}
}
