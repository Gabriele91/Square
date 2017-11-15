//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Application.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/Component.h"
#include "Square/Core/Object.h"
#include "Square/Core/Context.h"
#include "Square/Core/ClassObjectRegistration.h"
#include <algorithm>

namespace Square
{
namespace Scene
{
    //Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(Actor);

	//Registration in context
	void Actor::object_registration(Context& ctx)
	{
		//factory
		ctx.add<Actor>();
		//Attributes
        ctx.add<Actor>(attribute_field("name", std::string(), &Actor::m_name));
		ctx.add<Actor>(attribute_function<Actor, Vec3>
		("position"
		, Vec3(0)
		, [](const Actor* actor) -> Vec3   { return actor->position(); }
		, [](Actor* actor, const Vec3& pos){ actor->position(pos);     }));

		ctx.add<Actor>(attribute_function<Actor, Vec3>
		("scale"
		, Vec3(0)
		, [](const Actor* actor) -> Vec3  { return actor->scale(); }
		, [](Actor* actor, const Vec3& sc){ actor->scale(sc);      }));

		ctx.add<Actor>(attribute_function<Actor, Quat>
		("rotation"
		, Quat()
		, [](const Actor* actor) -> Quat  { return actor->rotation(); }
		, [](Actor* actor,const Quat& rot){ actor->rotation(rot);     }));
	}
    
    //serialize
    void Actor::serialize(Data::Archive& archivie)
    {
        //serialize this
        Data::serialize(archivie, this);
        //serialize components
        for(auto component : m_components)
        {
            //serialize name
            archivie % component->object_name();
            //serialize component
            Data::serialize<Object>(archivie, component);
        }
        //serialize childs
        uint64 size = m_childs.size();
        archivie % size;
        for(auto child : m_childs)
        {
            child->serialize(archivie);
        }
    }
    void Actor::serialize_json(Data::Json& archivie)
    {
        
    }
    //deserialize
    void Actor::deserialize(Data::Archive& archivie)
    {
        
    }
    void Actor::deserialize_json(Data::Json& archivie)
    {
        
    }
    
    //add a child
    void Actor::add(Shared<Actor> child)
    {
		if (!child) return;
        child->remove_from_parent();
        child->m_parent = shared_from_this();
        m_childs.push_back(child);
    }
    void Actor::add(Shared<Component> component)
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
    void Actor::remove(Shared<Actor> child)
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
    void Actor::remove(Shared<Component> component)
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
    Shared<Actor> Actor::parent() const
    {
        return m_parent;
    }
    
    //name
    const std::string& Actor::name() const
    {
        return m_name;
    }
    void Actor::name(const std::string& name)
    {
        m_name = name;
    }
    
    //contains an actor
    bool Actor::contains(Shared<Actor> child) const
    {
        //local
        if(std::find(m_childs.begin(), m_childs.end(), child) != m_childs.end()) return true;
        //deph
        for(auto child : m_childs) if(contains(child)) return true;
        //fail
        return false;
    }
    bool Actor::contains(Shared<Component> component) const
    {
        return  std::find(m_components.begin(), m_components.end(), component) != m_components.end();
    }

	Shared<Component> Actor::component(const std::string& name)
	{
		return component(ObjectInfo::compute_id(name));
	}
	Shared<Component> Actor::component(size_t id)
	{
		for (auto& component : m_components)
		{
			if (component->object_id() == id)
			{
				return component;
			}
		}
		//Context?
		if (!Application::context()) return nullptr;
		//create
		Shared<Component> new_component = StaticPointerCast<Component>(Application::context()->create(id));
		//test
		if (!new_component)
		{
			return nullptr;
		}
		//add
		add(new_component);
		//return
		return new_component;
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
