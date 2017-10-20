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
    struct Message;
    class  BaseComponent;
    class  BaseEntity;
    
    struct SQUARE_API Message
    {
        BaseEntity* m_sender;
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
        
        Message(BaseEntity* sender, const Variant& value)
        {
            m_sender = sender;
            m_value  = value;
        }
    };
    
    class SQUARE_API BaseComponent : public SmartPointers< BaseComponent >, public Uncopyable
    {
    public:
        
        //utltils
        void remove_from_entity();
        
        //get
        BaseEntity* entity();
        
        //all events
        virtual void on_attach(BaseEntity* entity) = 0;
        virtual void on_deattch()                  = 0;
        virtual void on_message(const Message& msg)= 0;
        
    protected:
        //entity
        BaseEntity* m_parent { nullptr };
        //internal
        void submit_add(BaseEntity* entity);
        void submit_remove();
        void submit_message(const Message& msg);
        //friends
        friend class BaseEntity;
    };
    
    class SQUARE_API BaseEntity
    {
    public:
        
        void add(BaseComponent::SPtr component);
        void remove(BaseComponent::SPtr component);
        bool contains(BaseComponent::SPtr component) const;
        void send_message(const Variant& variant);
        
    protected:
        //list of components
        std::vector < BaseComponent::SPtr > m_components;
    };
}
