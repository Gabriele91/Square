//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Scene/Actor.h"
#include <algorithm>

namespace Square
{
namespace Scene
{
    //add a child
    void Actor::add(Actor::SPtr child)
    {
		if (!child) return;
        child->remove_from_parent();
        child->m_parent = shared_from_this();
        m_childs.push_back(child);
    }
    void Actor::add(Component::SPtr component)
    {
		if (!component) return;
        //Olready added
        if(component->actor() == this) return;
        //Remove
        component->remove_from_parent();
        //Add
        m_components.push_back(component);
        //submit
        component->submit_add(this);
    }
    
    //remove a child
    void Actor::remove(Actor::SPtr child)
    {
        if(child->m_parent.get() == this)
        {
            //remove ref to parent
            child->m_parent = nullptr;
            //remove child from list
            auto it = std::find(m_childs.begin(), m_childs.end(), child);
            if(it != m_childs.end()) m_childs.erase(it);
        }
    }
    void Actor::remove(Component::SPtr component)
    {
        //Is your own
        if(component->actor() != this) return;
        //event
        component->submit_remove();
        //find and remove
        auto it = std::find(m_components.begin(), m_components.end(), component);
        if(it != m_components.end()) m_components.erase(it);
    }
    void Actor::remove_from_parent()
    {
        if(m_parent) m_parent->remove(shared_from_this());
    }
    
    //get parent
    Actor::SPtr Actor::parent() const
    {
        return m_parent;
    }
    
    //contains an actor
    bool Actor::contains(Actor::SPtr child) const
    {
        //local
        if(std::find(m_childs.begin(), m_childs.end(), child) != m_childs.end()) return true;
        //deph
        for(auto child : m_childs) if(contains(child)) return true;
        //fail
        return false;
    }
    bool Actor::contains(Component::SPtr component) const
    {
        return  std::find(m_components.begin(), m_components.end(), component) != m_components.end();
    }

	Component::SPtr Actor::component(int id)
	{
		for (auto& components : m_components)
		{
			if (components->id() == id)
			{
				return components;
			}
		}
		return nullptr;
	}
	Component::SPtr Actor::component(const std::string& name)
	{
		for (auto& components : m_components)
		{
			if (components->name() == name)
			{
				return components;
			}
		}
		return nullptr;
	}

    //message to components
    void Actor::send_message(const Variant& variant, bool brodcast)
    {
        //create message
        Message msg(this,variant);
        //send
        send_message(msg, brodcast);
    }
    void Actor::send_message(const Message& msg, bool brodcast)
    {
        //send
        for(auto component : m_components)
        {
            component->submit_message(msg);
        }
        //brodcast
        if(brodcast) for(auto child : m_childs)
        {
            child->send_message(msg, brodcast);
        }
    }
    

    
    //matrix op
    void Actor::translation(const Vec3& vector)
    {
        m_tranform.m_position += vector;
        set_dirty();
    }
    
    void Actor::move(const Vec3& vector)
    {
        m_tranform.m_position += rotation(true) * vector;
        set_dirty();
    }
    
    void Actor::turn(const Quat& rot)
    {
        m_tranform.m_rotation = rot * m_tranform.m_rotation;
        set_dirty();
    }
    
    void Actor::position(const Vec3& pos)
    {
        m_tranform.m_position = pos;
        set_dirty();
    }
    
    void Actor::rotation(const Quat& rot)
    {
        m_tranform.m_rotation = rot;
        set_dirty();
    }
    
    void Actor::scale(const Vec3& scale)
    {
        m_tranform.m_scale = scale;
        set_dirty();
    }
    
    Vec3 Actor::position(bool global) const
    {
        if(global)
        {
            return Vec3(model_matrix()[3]);
        }
        return  m_tranform.m_position;
    }
    
    Quat Actor::rotation(bool global) const
    {
        if(global)
        {
            Mat3 rot_scale(model_matrix());
            //len
            Vec3 scale(length(rot_scale[0]),
                       length(rot_scale[1]),
                       length(rot_scale[2]));
            //normalize
            rot_scale[0] /= scale[0];
            rot_scale[1] /= scale[1];
            rot_scale[2] /= scale[2];
            //must to be a ortogonal matrix
            return  quat_cast(traspose(inverse(rot_scale)));
        }
        return m_tranform.m_rotation;
    }
    
    Vec3 Actor::scale(bool global) const
    {
        if(global)
        {
            Mat3 rot_scale(model_matrix());
            return Vec3(length(rot_scale[0]),
                        length(rot_scale[1]),
                        length(rot_scale[2]));
        }
        return m_tranform.m_scale;
    }
    
    Mat4 const& Actor::local_model_matrix()
    {
        compute_matrix();
        return m_model_local;
    }
    
    Mat4 const& Actor::global_model_matrix()
    {
        compute_matrix();
        return m_model_global;
    }
    
    Mat4 const& Actor::model_matrix() const
    {
        compute_matrix();
        return m_model_global;
    }
    
    
    void Actor::set_dirty()
    {
        if (!m_tranform.m_dirty)
        {
            m_tranform.m_dirty = true;
            send_dirty();
        }
    }
    
    void Actor::send_dirty()
    {
        //to components
        for(auto component : m_components) component->on_transform();
        //to childs
        for (auto child : m_childs) child->set_dirty();
    }
    
    void Actor::compute_matrix() const
    {
        if (m_tranform.m_dirty)
        {
            //T*R*S
            m_model_local  = Square::translate(Mat4(1.0f), m_tranform.m_position);
            m_model_local *= Square::to_mat4(  m_tranform.m_rotation );
            m_model_local  = Square::scale(m_model_local, m_tranform.m_scale);
            //global
            if (parent())
            {
                m_model_global = parent()->model_matrix() * m_model_local;
            }
            else
            {
                m_model_global = m_model_local;
            }
            m_tranform.m_dirty = false;
        }
    }
}
}
