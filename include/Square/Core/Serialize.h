//
//  Square
//
//  Created by Gabriele on 09/09/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#pragma once
#include <string>
#include <unordered_map>
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/Attribute.h"

namespace Square
{
    //Class used by all classes with serializable attributes,
    class Serializable : public Object
    {
    public:
        SQUARE_OBJECT(Serializable)
    };
  
    //context
    class SerializeContext
    {
    public:
        
        //add an attrbute
        void add(uint64 object_id,const Attribute& attribute)
        {
            m_context[object_id].push_back(attribute);
        }
        void add(const Object& object,const Attribute& attribute)
        {
            m_context[object.object_id()].push_back(attribute);
        }
        void add(const ObjectInfo& info,const Attribute& attribute)
        {
            m_context[info.id()].push_back(attribute);
        }
        template < class T >
        void add(const Attribute& attribute)
        {
            m_context[T::static_object_id()].push_back(attribute);
        }
        
        //get attributes
        std::vector < Attribute >& get(uint64 object_id)
        {
            return m_context[object_id];
        }
        
        //clear
        void claer()
        {
            m_context.clear();
        }
        
    private:
        //context
        std::unordered_map< uint64, std::vector < Attribute > > m_context;
        
    };
}

