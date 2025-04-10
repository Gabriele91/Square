//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Variant.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/Object.h"
#include "Square/Core/Attribute.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Data/AttributeSerialize.h"
#include "Square/Data/Json.h"


namespace Square
{
namespace Scene
{
    //..................
    //declaretion
    class  Actor;
    class  Component;
	struct Message;
	//Alias
    using ComponentMap = std::unordered_map< uint64, Shared<Component> >;
    //..................
    struct SQUARE_API Message
    {
        Actor* m_sender;
        VariantRef m_value;
        
        Message()
        {
            m_sender = nullptr;
        }
        
        Message(const VariantRef& value)
        {
            m_sender = nullptr;
            m_value = value;
        }
        
        Message(Actor* sender, const VariantRef& value)
        {
            m_sender = sender;
            m_value  = value;
        }
    };
    
    class SQUARE_API Component : public Object
							   , public Uncopyable
							   , public SharedObject<Component>
    {
    public:

		//A square object
		SQUARE_OBJECT(Component)

        //Init
        Component(Context& context);
        virtual ~Component();
        
        //utils
        void remove_from_parent();
        
        //actor
		Weak<Actor> actor();
        const Weak<Actor> actor() const;
                
        //all events
        virtual void on_attach(Actor& entity)      {}
        virtual void on_deattch()                  {}
        virtual void on_transform()                {}
        virtual void on_message(const Message& msg){}
        
        //virtual
        virtual void serialize(Data::Archive& archive) = 0;
        virtual void serialize_json(Data::JsonValue& archive) = 0;
        //deserialize
        virtual void deserialize(Data::Archive& archive) = 0;
        virtual void deserialize_json(Data::JsonValue& archive) = 0;
        
        
    protected:
        //parent
        Weak<Actor> m_parent;
        
        //internal
        void submit_add(Weak<Actor> actor);
        
        void submit_remove();
        
        void submit_message(const Message& msg);
        
        //access
        friend class Actor;
    };
}
}
