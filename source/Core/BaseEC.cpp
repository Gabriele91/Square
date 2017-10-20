//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/BaseEC.h"


namespace Square
{
    //Component
    void BaseComponent::remove_from_entity()
    {
        if(m_parent)
        {
            m_parent->remove(shared_from_this());
        }
    }
    
    BaseEntity* BaseComponent::entity()
    {
        return m_parent;
    }
    
    
    void BaseComponent::submit_add(BaseEntity* entity)
    {
        on_attach(entity);
        m_parent = entity;
    }
    
    void BaseComponent::submit_remove()
    {
        on_deattch();
        m_parent = nullptr;
    }
    
    void BaseComponent::submit_message(const Message& msg)
    {
        on_message(msg);
    }
    
    //Entity
    void BaseEntity::add(BaseComponent::SPtr component)
    {
        //Olready added
        if(component->entity() == this) return;
        //Remove
        component->remove_from_entity();
        //Add
        m_components.push_back(component);
        //submit
        component->submit_add(this);
    }
    
    void BaseEntity::remove(BaseComponent::SPtr component)
    {
        //Is your own
        if(component->entity() != this) return;
        //event
        component->submit_remove();
        //find and remove
        auto it = std::find(m_components.begin(), m_components.end(), component);
        if(it != m_components.end()) m_components.erase(it);
    }
    
    bool BaseEntity::contains(BaseComponent::SPtr component) const
    {
        return  std::find(m_components.begin(), m_components.end(), component) != m_components.end();
    }
    
    void BaseEntity::send_message(const Variant& variant)
    {
        //create message
        Message msg(this,variant);
        //send
        for(auto component : m_components)
        {
            component->submit_message(msg);
        }
    }
}
