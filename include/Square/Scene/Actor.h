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
#include "Square/Core/Resource.h"
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
	//..................
    class SQUARE_API Actor : public ResourceObject
                           , public InheritableSharedObject<Actor>
						   , public Render::Transform
    {
    public:

        using InheritableSharedObject<Actor>::shared_from_this;
        using InheritableSharedObject<Actor>::weak_from_this;
        
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
        void send_message(const VariantRef& value, bool brodcast = false);
        void send_message(const Message& msg, bool brodcast = false);
        
        //matrix op
        void translation(const Vec3& vector) override;
        void move(const Vec3& vector) override;
        void turn(const Quat& rot) override;
        
        void position(const Vec3& pos) override;
        void rotation(const Quat& rot) override;
        void scale(const Vec3& pos) override;
        
        Vec3 position(bool global = false) const override;
        Quat rotation(bool global = false) const override;
        Vec3 scale(bool global = false) const override;
        
        Mat4 const& local_model_matrix() const override;
        Mat4 const& global_model_matrix() const override;

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
        void   level(Weak<Level> level);
		bool   is_root_of_level() const;
		bool   remove_from_level();
        
        //set
        void set(Render::UniformBufferTransform* gpubuffer) const override;

        //load actor
        bool load(const std::string& path) override;

    protected:
		//friend class
		friend class Level;
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
		mutable Mat4  m_inv_model_global;
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
