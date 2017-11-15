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
#include "Square/Data/AttributeSerialize.h"
#include "Square/Data/Json.h"

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
                           , public SharedObject<Actor>
                           , public Uncopyable
    {
    public:
        
		//Init object
		SQUARE_OBJECT(Actor)
		//Registration in context
		static void object_registration(Context& ctx);

        //constructor
        Actor() {}
        Actor(const std::string& name) : m_name(name){}
        
        //add a child
        void add(Shared<Actor> child);
        void add(Shared<Component> component);

        //remove a child
        void remove(Shared<Actor> child);
        void remove(Shared<Component> component);
        void remove_from_parent();
        
        //get parent
        Shared<Actor> parent() const;
        
        //contains an actor
        bool contains(Shared<Actor> child) const;
        bool contains(Shared<Component> component) const;
		
		//get by type
		template< class T >
		Shared< T > component()
		{
			return StaticPointerCast<T>(component(T::static_object_id()));
		}
		Shared<Component> component(const std::string& name);
		Shared<Component> component(size_t id);
        
        //name        
        const std::string& name() const;
        void name(const std::string&);

        
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
        
        //serialize
        void serialize(Data::Archive& archivie);
        void serialize_json(Data::Json& archivie);
        //deserialize
        void deserialize(Data::Archive& archivie);
        void deserialize_json(Data::Json& archivie);

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
        //node name
        std::string m_name;
        //parent
        Shared<Actor> m_parent;
        //child list
        std::vector< Shared<Actor> > m_childs;
        std::vector< Shared<Component> > m_components;
    };
}
}
