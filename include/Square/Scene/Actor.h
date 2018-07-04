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
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"
#include "Square/Data/AttributeSerialize.h"
#include "Square/Data/Json.h"
#include "Square/Render/Transform.h"

namespace Square
{
namespace Scene
{
    //..................
    //declaretion
	class Actor;
	class Level;
	class World;
	using ActorList = std::vector< Shared<Actor> >;
	using LevelList = std::vector< Shared<Level> >;
	//Transform uniform buffer
	PACKED(ConstantBufferStruct UniformBufferTransform
    {
		Mat4 m_model;
	    Vec3 m_position; float __PADDING0__; //alignas(16) 
		Vec3 m_scale;    float __PADDING1__; //alignas(16) 
		Mat4 m_rotation;
    });
	//..................
    class SQUARE_API Actor : public Object
                           , public SharedObject<Actor>
                           , public Uncopyable
						   , public Render::Transform
    {
    public:
        
		//Init object
		SQUARE_OBJECT(Actor)
		//Registration in context
		static void object_registration(Context& ctx);

        //constructor
		Actor(Context& context);
		Actor(Context& context, const std::string& name);
        
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
		Shared<Component> component(uint64 id);		
		const ComponentList& components() const;
        
        //get child
        Shared<Actor> child();
        Shared<Actor> child(size_t index);
        Shared<Actor> child(const std::string& name);        
		const ActorList& childs() const;

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
        
        Mat4 const& local_model_matrix() const;
        Mat4 const& global_model_matrix() const;

		//force to recompute all matrix
		void dirty();
        
        //serialize
        void serialize(Data::Archive& archivie);
        void serialize_json(Data::Json& archivie);
        //deserialize
        void deserialize(Data::Archive& archivie);
        void deserialize_json(Data::Json& archivie);

		//get level
		Weak<Level> level() const;
		bool   is_root_of_level() const;
		bool   remove_from_level();
        
        //set
        void set(UniformBufferTransform* gpubuffer) const;

    protected:
		//friend class
		friend class Level;
		//set a level
		void level(Weak<Level> level);
		//level events
		Weak<Level> m_level;
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
        ActorList     m_childs;
		ComponentList m_components;
    };
}
}
