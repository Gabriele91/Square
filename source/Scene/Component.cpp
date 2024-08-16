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
    Component::Component(Context& context) : Object(context), SharedObject_t(context.allocator())
    {
    }
    
    // utils
    void Component::remove_from_parent()
    {
        if(auto shared_actor = actor().lock()) 
			shared_actor->remove(shared_from_this());
    }
    
    //actor
    Weak<Actor> Component::actor()
    {
        return m_parent;
    }
    
    const Weak<Actor> Component::actor() const
    {
        return m_parent;
    }
    
    //internal
    void Component::submit_add(Weak<Actor> actor)
    {
		m_parent = actor;
        on_attach(*actor.lock());
    }
    
    void Component::submit_remove()
    {
        on_deattch();
		m_parent = Weak<Actor>();
    }
    
    void Component::submit_message(const Message& msg)
    {
        on_message(msg);
    }
}
}
