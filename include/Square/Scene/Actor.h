//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Core/Object.h"
#include "Square/Scene/Component.h"
#include "Square/Scene/ComponentFactory.h"

namespace Square
{
namespace Scene
{
    //..................
    //declaretion
    class Actor;
    class Component;
    struct Message;
    //..................
    
    class SQUARE_API Actor : public Object
                           , public SmartPointers<Actor>
                           , public Uncopyable
    {
    public:
        
		//Init object
		SQUARE_OBJECT(Actor)

        //add a child
        void add(Actor::SPtr child);
        void add(Component::SPtr component);

        //remove a child
        void remove(Actor::SPtr child);
        void remove(Component::SPtr component);
        void remove_from_parent();
        
        //get parent
        Actor::SPtr parent() const;
        
        //contains an actor
        bool contains(Actor::SPtr child) const;
        bool contains(Component::SPtr component) const;
		
		//get by type
		template< class T >
		std::shared_ptr< T > component()
		{
			for (auto& components : m_components)
			{
				if (dynamic_cast<T*>(components.get()))
				{
					return std::static_pointer_cast<T>(components);
				}
			}
			//create
			Component::SPtr new_component = ComponentFactory::create(T::static_object_id());
			//add
			add(new_component);
			//return
			return std::static_pointer_cast<T>(new_component);
		}
		Component::SPtr component(int id);
		Component::SPtr component(const std::string& name);
        
        //message
        void send_message(const Variant& value, bool brodcast = false);
        void send_message(const Message& msg, bool brodcast = false);
        
        //matrix op
        void translation(const Vec3& vector);
        void move(const Vec3& vector);
        void turn(const Quat& rot);
        
        void position(const Vec3& pos);
        void rotation(const Quat& rot);
        void scale(const Vec3& pos);
        
        Vec3 position(bool global = false) const;
        Quat rotation(bool global = false) const;
        Vec3 scale(bool global = false) const;
        
        Mat4 const& local_model_matrix();
        Mat4 const& global_model_matrix();
        
    protected:
        //fake const
        Mat4 const& model_matrix() const;
        //local
        struct Tranform
        {
            bool m_dirty   { false                  };
            Vec3 m_position{ 0.0f, 0.0f, 0.0f       };
            Quat m_rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
            Vec3 m_scale   { 1.0f, 1.0f, 1.0f       };
        };
        //Local tranform
        mutable Tranform m_tranform;
        //matrix
        mutable Mat4  m_model_local;
        mutable Mat4  m_model_global;
        //compute
        void compute_matrix() const;
        //send dirty
        void    set_dirty();
        void    send_dirty();
        //parent
        Actor::SPtr m_parent;
        //child list
        std::vector< Actor::SPtr > m_childs;
        std::vector< Component::SPtr > m_components;
    };
}
}
