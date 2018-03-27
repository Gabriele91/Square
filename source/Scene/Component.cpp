//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Scene/Actor.h"
#include "Square/Scene/Component.h"

namespace Square
{
namespace Scene
{
    //init
    Component::Component(Context& context) : Object(context)
    {
    }
    
    // utils
    void Component::remove_from_parent()
    {
        if(actor()) actor()->remove(shared_from_this());
    }
    
    //actor
    Actor* Component::actor()
    {
        return m_parent;
    }
    
    //internal
    void Component::submit_add(Actor* actor)
    {
        on_attach(*actor);
        m_parent = actor;
    }
    
    void Component::submit_remove()
    {
        on_deattch();
        m_parent = nullptr;
    }
    
    void Component::submit_message(const Message& msg)
    {
        on_message(msg);
    }
}
}
