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
    
    class SQUARE_API Component : public SmartPointers< Component >, public Uncopyable
    {
    public:
        
        //Init
        Component(const std::string& name, size_t id);
        
        //utils
        void remove_from_parent();
        
        //actor
        Actor* actor();
        
        //component info
        const std::string& name() const;
        const size_t       id()   const;
        
        //all events
        virtual void on_attach(Actor& entity)      {}
        virtual void on_deattch()                  {}
        virtual void on_transform()                {}
        virtual void on_message(const Message& msg){}
        
    public:
        //parent
        Actor* m_parent { nullptr };
        
        //info about component
        std::string m_name;
        size_t m_id;
        
        //internal
        void submit_add(Actor* actor);
        
        void submit_remove();
        
        void submit_message(const Message& msg);
        
        //access
        friend class Actor;
    };
}
}
