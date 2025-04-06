//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Object.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/Context.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Core/StringUtilities.h"
#include "Square/Math/Transformation.h"
#include "Square/Core/Application.h"
#include "Square/Scene/Component.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/Level.h"
#include <algorithm>
#include <fstream>


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
        ctx.add_resource<Actor>({ ".ac", ".acgz", ".acj", ".acjgz" });
		//factory
		ctx.add_object<Actor>();
		//Attributes
        ctx.add_attribute_field<Actor, std::string>("name", std::string(), offsetof(Actor,m_name));
		ctx.add_attribute_function<Actor, Vec3>
		("position"
		, Vec3(0)
		, [](const Actor* actor) -> Vec3   { return actor->position(); }
		, [](Actor* actor, const Vec3& pos){ actor->position(pos);     });

		ctx.add_attribute_function<Actor, Vec3>
		("scale"
		, Vec3(0)
		, [](const Actor* actor) -> Vec3  { return actor->scale(); }
		, [](Actor* actor, const Vec3& sc){ actor->scale(sc);      });

		ctx.add_attribute_function<Actor, Quat>
		("rotation"
		, Quat()
		, [](const Actor* actor) -> Quat  { return actor->rotation(); }
		, [](Actor* actor,const Quat& rot){ actor->rotation(rot);     });
	}

	Actor::Actor(Context& context) : ResourceObject(context), BaseInheritableSharedObject(context.allocator()) {}
	Actor::Actor(Context& context, const std::string& name) : ResourceObject(context), BaseInheritableSharedObject(context.allocator()), m_name(name) {}
    Actor::~Actor()
    {
    }

    // Load child node from resource
    Shared<Actor> Actor::load_child(const std::string& resource_name)
    {
        Shared<Actor> new_actor = context().resource<Actor>(resource_name);
        if (new_actor)
        {
            add(new_actor);
        }
        return new_actor;
    }

    //serialize
    void Actor::serialize(Data::Archive& archive)
    {
        //serialize this
        Data::serialize(archive, this);
        //serialize components
        {
            uint64 size = m_components.size();
            archive % size;
            for(auto component : m_components)
            {
                //serialize name
                archive % component.second->object_name();
                //serialize component
                component.second->serialize(archive);
            }
        }
        //serialize childs
        {
            uint64 size = m_childs.size();
            archive % size;
            for(auto child : m_childs)
            {
                child->serialize(archive);
            }
        }
    }
    void Actor::serialize_json(Data::JsonValue& archive)
    {
         Data::JsonValue json_data = Data::JsonObject();
         Data::serialize_json(json_data, this);
         archive["data"] = std::move(json_data);
         // Components
         auto json_components = Data::JsonArray();
         json_components.reserve(m_components.size());
         for (auto component : m_components)
         {
             Data::JsonValue json_component = Data::JsonObject();
             //serialize component name
             json_component["name"] = component.second->object_name();
             //serialize component data 
             Data::Json json_component_data = Data::JsonObject();
             component.second->serialize_json(json_component_data);
             json_component["data"] = std::move(json_component_data);
             // add component
             json_components.emplace_back(std::move(json_component));
         }
         archive["components"] = std::move(json_components);
         // Children
         auto json_children = Data::JsonArray();
         json_children.reserve(m_childs.size());
         for (auto child : m_childs)
         {
             Data::Json json_child = Data::JsonObject();
             child->serialize_json(json_child);
             json_children.emplace_back(std::move(json_child));
         }
         archive["children"] = std::move(json_children);

    }
    //deserialize
    void Actor::deserialize(Data::Archive& archive)
    {
        ///clear
        m_components.clear(); //todo: call events
        m_childs.clear();     //todo: call events
        //deserialize this
        Data::deserialize(archive, this);
        //deserialize components
        {
            uint64 size = 0;
            archive % size;
            for(uint64 i = 0; i!=size; ++i)
            {
                //name of component
                std::string name;
                //serialize name
                archive % name;
                //new component
                Shared<Component> component = this->component(name);
                //deserialize component
                component->deserialize(archive);
            }
        }
        //deserialize childs
        {
            uint64 size = 0;
            archive % size;
            for(uint64 i = 0; i!=size; ++i)
            {
                child()->deserialize(archive);
            }
        }
    }
    void Actor::deserialize_json(Data::JsonValue& archive)
    {
        ///clear
        m_components.clear(); //todo: call events
        m_childs.clear();     //todo: call events
        //Data
        if (archive.contains("data"))
        {
            Data::JsonValue& jdata = archive["data"];
            //deserialize this
            Data::deserialize_json(jdata, this);
        }
        //Components
        if (archive.contains("components"))
        {
            Data::JsonValue& jcomponents = archive["components"];
            // Get name
            if (jcomponents.is_array())
            for(auto& jcomponent : jcomponents.array())
            if (jcomponent.contains("name") && jcomponent["name"].is_string())
            {
                //new component
                Shared<Component> component = this->component(jcomponent["name"].string());
                if (jcomponent.contains("data") && jcomponent["data"].is_object())
                {
                    //deserialize component
                    component->deserialize_json(jcomponent["data"]);
                }
            }
        }
        //Components
        if (archive.contains("children"))
        {
            Data::JsonValue& jchildren = archive["children"];
            // Get name
            if (jchildren.is_array())
            for(auto& jchild : jchildren.array())
            if (jchild.is_object())
            {
                child()->deserialize_json(jchild);
            }
        }
    }

    void Actor::visit(const std::function<bool(Shared<Actor>)>& callback)
    {
        if (callback(shared_from_this()))
        {
            for (auto child : childs())
            {
                child->visit(callback);
            }
        }
    }
    
    //add a child
    void Actor::add(Shared<Actor> child)
    {
		if (!child) return;
        child->remove_from_parent();
        child->m_parent = shared_from_this();
        m_childs.push_back(child);
		child->level(level());
		child->dirty();
    }
    void Actor::add(Shared<Component> component)
    {
		if (!component) return;
        //Olready added
        if(component->actor().lock().get() == this) return;
        //Remove
        component->remove_from_parent();
        //Add
        m_components.insert({ component->object_id(), component });
        //submit
        component->submit_add(shared_from_this());
		//send event to level
		if (auto shared_level = level().lock())
			shared_level->on_add_a_component(shared_from_this(),component);
    }
    
    //remove a child
    void Actor::remove(Shared<Actor> child)
    {
        if (child)
        if(auto parent = child->m_parent.lock(); parent.get() == this)
        {
            //remove ref to parent
            child->m_parent.reset();
            //remove child from list
            auto it = std::find(m_childs.begin(), m_childs.end(), child);
			if (it != m_childs.end())
			{
				//remove from level
				child->level(Weak<Level>());
				//ok
				m_childs.erase(it);
			}
        }
    }
    void Actor::remove(Shared<Component> component)
    {
        if (!component) return;
        //Is your own
        if(component->actor().lock().get() != this) return;
		//find 
		auto component_it = m_components.find(component->object_id());
        //remove
		if (component_it != m_components.end() && component_it->second == component)
		{
			//event
			component->submit_remove();
			//send event to level
			if (auto shared_level = level().lock())
				shared_level->on_add_a_component(shared_from_this(), component);
			//remove
			m_components.erase(component_it);
		}
    }
    void Actor::remove_from_parent()
    {
        if (auto parent = m_parent.lock())
        {
            parent->remove(shared_from_this());
        }
    }
    
    //get parent
    Weak<Actor> Actor::parent() const
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
        if (!component) return false;
        auto component_it = m_components.find(component->object_id());
        if (component_it == m_components.end()) return false;
        return  component_it->second == component;
    }

	Shared<Component> Actor::component(const std::string& name)
	{
		return component(ObjectInfo::compute_id(name));
	}
	Shared<Component> Actor::component(uint64 id)
	{
        if(auto component_it = m_components.find(id);  component_it != m_components.end())
        {
            return component_it->second;
        }
		//create
		Shared<Component> new_component = StaticPointerCast<Component>(context().create(id));
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
	const ComponentMap& Actor::components() const
	{
		return m_components;
	}

    //get/create child
    Shared<Actor> Actor::child()
    {
        //create
        auto actor = MakeShared<Actor>(context());
        //add
        add(actor);
        //return
        return actor;
    }
    Shared<Actor> Actor::child(size_t index)
    {
        return m_childs.size() ? m_childs[index] : nullptr;
    }
    Shared<Actor> Actor::child(const std::string& name)
    {
        //search
        for(auto child : m_childs) 
            if(child->name() == name) 
                return child;
        //create
        auto actor = MakeShared<Actor>(context(), name);
        //add
        add(actor);
        //return
        return actor;
    }
	const ActorList& Actor::childs() const
	{
		return m_childs;
	}

    //message to components
    void Actor::send_message(const VariantRef& variant, bool brodcast)
    {
        //create message
        Message msg(this, variant);
        //send
        send_message(msg, brodcast);
    }
    void Actor::send_message(const Message& msg, bool brodcast)
    {
        //send
        for(auto& component : m_components)
        {
            component.second->submit_message(msg);
        }
        //brodcast
        if(brodcast) for(auto& child : m_childs)
        {
            child->send_message(msg, brodcast);
        }
    }

	//level
	Weak<Level> Actor::level() const
	{
		if (m_level.lock()) return m_level;
		else return Weak<Level>();
	}	

	bool Actor::is_root_of_level() const
	{
		return !parent().lock() && m_level.lock();
	}

	bool Actor::remove_from_level()
	{
		if (auto current_level = m_level.lock())
		{
			current_level->remove(shared_from_this());
			level(Weak<Level>());
			dirty();
			return true;
		}
		return false;
	}


	void Actor::level(Weak<Level> level)
	{
		auto current_level = m_level.lock();
		auto new_level = level.lock();
		//cases
		if (current_level == new_level) return;
		//ref to level
		//..
		if (current_level)
			current_level->on_remove_a_actor(shared_from_this());
		//..
		m_level = level;
		//..
		if (new_level)
			new_level->on_add_a_actor(shared_from_this());
		//event
		dirty();
		//same for each chils
		for (Shared<Actor> actor : m_childs)
			actor->level(level);
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
    
    Mat4 const& Actor::local_model_matrix() const
    {
        compute_matrix();
        return m_model_local;
    }
    
    Mat4 const& Actor::global_model_matrix() const
    {
        compute_matrix();
        return m_model_global;
    }
    
    Mat4 const& Actor::model_matrix() const
    {
        compute_matrix();
        return m_model_global;
    }

	//force to recompute all matrix
	void Actor::dirty()
	{
		set_dirty();
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
        for(auto component : m_components) 
            component.second->on_transform();
        //to childs
        for (auto child : m_childs) child->set_dirty();
    }
    
    void Actor::compute_matrix() const
    {
        if (m_tranform.m_dirty)
        {
            //T*R*S
            m_model_local = Constants::identity<Mat4>();
            m_model_local = Square::translate(m_model_local, m_tranform.m_position);
            m_model_local *= Square::to_mat4(m_tranform.m_rotation);
            m_model_local = Square::scale(m_model_local, m_tranform.m_scale);
            //global
            if (auto parent = m_parent.lock(); parent)
            {
                m_model_global = parent->model_matrix() * m_model_local;
            }
            else
            {
                m_model_global = m_model_local;
            }
			m_inv_model_global = inverse(m_model_global);
            m_tranform.m_dirty = false;
        }
    }
    
    //set uniform buffer
    void Actor::set(Render::UniformBufferTransform* gpubuffer) const
    {
		gpubuffer->m_model     = global_model_matrix();
		gpubuffer->m_inv_model = m_inv_model_global;
		gpubuffer->m_rotation  = mat4_cast(rotation(true));
        gpubuffer->m_position  = position(true);
        gpubuffer->m_scale     = scale(true);
    }

    //Actor format
    enum class ActorFormat
    {
        SQ_BIN,
        SQ_BIN_GZ,
        SQ_JSON,
        SQ_JSON_GZ
    };

    static ActorFormat get_actor_format_from_extension(const std::string& extension)
    {
        ActorFormat actor_format{ ActorFormat::SQ_BIN };
        if (Square::case_insensitive_equal(extension, ".ac"))
        {
            actor_format = ActorFormat::SQ_BIN;
        }
        else if (Square::case_insensitive_equal(extension, ".acgz"))
        {
            actor_format = ActorFormat::SQ_BIN_GZ;
        }
        else if (Square::case_insensitive_equal(extension, ".acj"))
        {
            actor_format = ActorFormat::SQ_JSON;
        }
        else if (Square::case_insensitive_equal(extension, ".acjgz"))
        {
            actor_format = ActorFormat::SQ_JSON_GZ;
        }
        return actor_format;
    }

    //load actor
    bool Actor::load(const std::string& path)
    {
        using namespace Square;
        using namespace Square::Data;
        using namespace Square::Filesystem::Stream;
        // Deserialize, per each format
        switch (get_actor_format_from_extension(Filesystem::get_extension(path)))
        {
        default:
        case ActorFormat::SQ_BIN:
        {
            std::ifstream in_file_stream(path);
            if (in_file_stream.good())
            {
                ArchiveBinRead in_archive(Object::context(), in_file_stream);
                deserialize(in_archive);
                return true;
            }
            else
            {
                context().logger()->warning("Fail to read data file: " + path);
            }
        }
        break;
        case ActorFormat::SQ_BIN_GZ:
        {
            GZIStream in_file_stream(path);
            if (in_file_stream.good())
            {
                ArchiveBinRead in_archive(Object::context(), in_file_stream);
                deserialize(in_archive);
                return true;
            }
            else
            {
                context().logger()->warning("Fail to read data file: " + path);
            }
        }
        break;
        case ActorFormat::SQ_JSON:
        {
            using namespace Square;
            using namespace Square::Data;
            Json jin;
            if (jin.parser(Square::Filesystem::text_file_read_all(path)))
            {
                deserialize_json(jin);
                return true;
            }
            else
            {
                context().logger()->warnings(
                { { "Fail to parse json: " + path },
                    jin.errors()
                });
            }
        }
        break;
        case ActorFormat::SQ_JSON_GZ:
        {
            using namespace Square;
            using namespace Square::Data;
            Json jin;
            // Decompress string
            auto bin_vector = Square::Filesystem::binary_compress_file_read_all(path);
            if (bin_vector.empty())
            {
                return false;
            }
            else
            {
                context().logger()->warning("Fail to read json: " + path);
            }
            // Get string
            const char* json_cppstr_begin = (const char*)(bin_vector.data());
            const char* json_cppstr_end = (const char*)(bin_vector.data() + bin_vector.size());
            std::string json_string(json_cppstr_begin, json_cppstr_end);
            // Parsing
            if (jin.parser(json_string))
            {
                deserialize_json(jin);
                return true;
            }
            else
            {
                context().logger()->warnings(
                { { "Fail to parse json: " + path },
                    jin.errors()
                });
            }
        }
        break;
        }
        return false;
    }
}
}
