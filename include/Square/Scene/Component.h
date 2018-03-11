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
	using ComponentList = std::vector< Shared<Component> >;
    //..................
    struct SQUARE_API Message
    {
        Actor* m_sender;
        Variant m_value;
        
        Message()
        {
            m_sender = nullptr;
        }
        
        Message(const Variant& value)
        {
            m_sender = nullptr;
            m_value = value;
        }
        
        Message(Actor* sender, const Variant& value)
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
        Component();
        
        //utils
        void remove_from_parent();
        
        //actor
        Actor* actor();
                
        //all events
        virtual void on_attach(Actor& entity)      {}
        virtual void on_deattch()                  {}
        virtual void on_transform()                {}
        virtual void on_message(const Message& msg){}
        
    protected:
        //parent
        Actor* m_parent { nullptr };
        
        //internal
        void submit_add(Actor* actor);
        
        void submit_remove();
        
        void submit_message(const Message& msg);
        
        //access
        friend class Actor;
    };
}
}
