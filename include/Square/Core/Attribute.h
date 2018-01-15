//
//  Square
//
//  Created by Gabriele on 09/09/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#pragma once
#include <string>
#include <functional>
#include <type_traits>
#include "Square/Core/Object.h"
#include "Square/Core/Variant.h"
#include "Square/Core/SmartPointers.h"

namespace Square
{
	//Access to attribute
	class AttributeAccess : public SharedObject<AttributeAccess>
	{
	public:
		virtual void get(const Object* serializable, VariantRef& ret) const =0;
		virtual void set(Object* serializable, const VariantRef& set) =0;
	};

	//Attribute of a class
	class Attribute
	{
	public:

		//type of Attribute
		enum Type
		{
			NETWORK           = 0b0001,  //replicate on network
			FILE              = 0b0010,  //serializable/deserializable
			FILE_READONLY     = 0b0100,  //deserializable only
			EDITOR			  = 0b1000,  //used by editor (no serializable)
			//Composition
			DEFAULT = FILE | NETWORK     //default serializable/deserializable/network
		};

		//Default
		Attribute()
		{
			m_type = DEFAULT;
			m_value_type = VariantType::VR_INT;
			m_offset = 0;
			m_wrapper = nullptr;
			m_enum_names = nullptr;
		}

		//Copy
		Attribute(const Attribute& from)
		{
			m_name = from.m_name;
			m_type = from.m_type;
			m_value_type = from.m_value_type;
			m_offset = from.m_offset;
			m_wrapper = from.m_wrapper;
			m_default = from.m_default;
			m_enum_names = from.m_enum_names;
		}

		//Move
		Attribute(Attribute&& from)
		{
			m_name       = std::move(from.m_name);
			m_type       = std::move(from.m_type);
			m_value_type = std::move(from.m_value_type);
			m_offset     = std::move(from.m_offset);
			m_wrapper    = std::move(from.m_wrapper);
			m_default    = std::move(from.m_default);
			m_enum_names = std::move(from.m_enum_names);
		}

		//attribute of a field
		Attribute
		(
			const std::string& name,
			VariantType     value_type,
            Variant&&       default_value,
			size_t          offset,
			Type			type = DEFAULT
		)
        :m_default(std::move(default_value))
		{
			m_name = name;
			m_type = type;
			m_value_type = value_type;
			m_offset = offset;
			m_wrapper = nullptr;
			m_enum_names = nullptr;
		}

		//attribute by wrapper
		Attribute
		(
			const std::string&      name,
			VariantType             value_type,
            Variant&&               default_value,
			Shared<AttributeAccess> access,
			Type			        type = DEFAULT
		)
        :m_default(std::move(default_value))
		{
			m_name = name;
			m_type = type;
			m_value_type = value_type;
			m_offset = 0;
			m_wrapper = access;
			m_enum_names = nullptr;
		}


		//attribute is a enum
		Attribute
		(
			const std::string& name,
			VariantType     value_type,
            Variant&&       default_value,
			const char**    enum_names,
			Type			type = DEFAULT
		)
        :m_default(std::move(default_value))
		{
			m_name = name;
			m_type = type;
			m_value_type = value_type;
			m_offset = 0;
			m_wrapper = nullptr;
			m_enum_names = enum_names;
		}

        const std::string&      name()          const { return m_name; }
        Type                    type()          const { return m_type; }
        VariantType             value_type()    const { return m_value_type; }
        const Variant&          default_value() const { return m_default; }
        size_t                  offset()        const { return m_offset; }
        Shared<AttributeAccess> access()        const { return m_wrapper; }
        const char**            enum_name()     const { return m_enum_names; }

        
        //helper
        void get(const Object* serializable, VariantRef& retvalue) const
        {
            //wrapper?
            if(m_wrapper) m_wrapper->get(serializable, retvalue);
            //is a field
            retvalue = VariantRef(m_value_type, (void*)((const char*)serializable+m_offset));
        }
        void set(Object* serializable, const VariantRef& setvalue) const
        {
            //wrapper?
            if(m_wrapper) m_wrapper->set(serializable, setvalue);
            //cast to variant
            VariantRef field_as_variant(m_value_type, (void*)((char*)serializable+m_offset));
            //value copy
            field_as_variant.copy_from( setvalue );
        }
        
	protected:

		// Name of attribute
		std::string m_name;
		// Type of attribute
		Type m_type;
		// Type of value
		VariantType m_value_type;
		// Default
		Variant  m_default;
		// Offset attribute
		size_t   m_offset;
		// or wrapper
		Shared<AttributeAccess> m_wrapper;
		// or enum names.
		const char** m_enum_names;

	};

	/// Attribute train (default use const reference).
	template <typename T, class Fundamental = void>
	struct AttributeTrain
	{
		/// Get function return type.
		using ReturnType = const T&;
		/// Set function parameter type.
		using ParameterType = const T&;
    };
    
    /// Attribute train function (use const reference as parameter and copy value as return)
    template <typename T>
    struct AttributeFunctionTrain
    {
        /// Get function return type.
        using ReturnType = T;
        /// Set function parameter type.
        using ParameterType = const T&;
    };

	/// Attribute train (for fundamental types use copy value).
	template <class T>
	struct AttributeTrain<T, std::enable_if_t<std::is_fundamental<T>::value>>
	{
		using ReturnType = T;
		using ParameterType = T;
	};
	
	// Template implementation of the attribute accessor invoke helper class.
	template <typename T, typename U, typename Trait = AttributeTrain<U> >
	class AttributeAccessMethod : public AttributeAccess
	{
	public:
		// Ptr
		using GetFunction = typename Trait::ReturnType(T::*)() const;
		using SetFunction = void (T::*)(typename Trait::ParameterType);

		// Construct with function pointers.
		AttributeAccessMethod(GetFunction get_function, SetFunction set_function)
		: m_get(get_function)
		, m_set(set_function)
		{
			assert(m_get);
			assert(m_set);
		}

		// Invoke getter function.
		virtual void get(const Object* serializable, VariantRef& ret) const override
		{
			assert(serializable);
			const T* self = static_cast<const T*>(serializable);
			ret = (self->*m_get)();
		}

		// Invoke setter function.
		virtual void set(Object* serializable, const VariantRef& value) override
		{
			assert(serializable);
			T* self = static_cast<T*>(serializable);
			(self->*m_set)(value.get<U>());
		}

		// Pointer to getter function.
		GetFunction m_get;

		// Pointer to setter function.
		SetFunction m_set;
	};
	
	// Template implementation of the attribute accessor invoke helper class.
	template <typename T, typename U, typename Trait = AttributeFunctionTrain<U> >
	class AttributeAccessFunction : public AttributeAccess
	{
	public:
		// Ptr
		using GetFunction = typename Trait::ReturnType(*)(const T*);
		using SetFunction = void(*)(T*, typename Trait::ParameterType);

		// Construct with function pointers.
		AttributeAccessFunction(GetFunction get_function, SetFunction set_function)
			: m_get(get_function)
			, m_set(set_function)
		{
			assert(m_get);
			assert(m_set);
		}

		// Invoke getter function.
		virtual void get(const Object* serializable, VariantRef& ret) const override
		{
			assert(serializable);
			const T* self = static_cast<const T*>(serializable);
			ret = (*m_get)(self);
		}

		// Invoke setter function.
		virtual void set(Object* serializable, const VariantRef& value) override
		{
			assert(serializable);
			T* self = static_cast<T*>(serializable);
			(*m_set)(self, value.get<U>());
		}

		// Pointer to getter function.
		GetFunction m_get;

		// Pointer to setter function.
		SetFunction m_set;
	};

	//helper
	template<typename T, typename U, typename Trait = AttributeTrain<U> >
	static Attribute attribute_method(
		  const std::string& attribute_name
		, const U& default_value
		, typename AttributeAccessMethod< T, U, Trait >::GetFunction get_method
		, typename AttributeAccessMethod< T, U, Trait >::SetFunction set_method
		, Attribute::Type type = Attribute::DEFAULT
	)
	{
		return Attribute(
			  attribute_name
			, variant_traits<U>()
			, default_value
			, Shared<AttributeAccess>(
				(AttributeAccess*)new AttributeAccessMethod< T, U, Trait >(get_method, set_method)
			)
			, type
		);
	}

	template<typename T, typename U, typename Trait = AttributeFunctionTrain<U> >
	static Attribute attribute_function
	(
		  const std::string& attribute_name
		, const U& default_value
		, typename AttributeAccessFunction< T, U, Trait >::GetFunction get_function
		, typename AttributeAccessFunction< T, U, Trait >::SetFunction set_function
		, Attribute::Type type = Attribute::DEFAULT
	)
	{
		return Attribute(
			  attribute_name
			, variant_traits<U>()
			, default_value
			, Shared<AttributeAccess>(
				(AttributeAccess*)new AttributeAccessFunction< T, U, Trait >(get_function, set_function)
			)
			, type
		);
	}

	template<typename T, typename U >
	static Attribute attribute_field
	(
		  const std::string& field_name
		, const U& default_value
		, U T::*member
		, Attribute::Type type = Attribute::DEFAULT
	)
	{
		return Attribute(
			  field_name
			, variant_traits<U>()
			, default_value
			, //(char*)&((T*)nullptr->*member) - (char*)nullptr
			 (size_t)((char*)&(((T*)nullptr)->*member) - (char*)nullptr)
			, type
		);
	}
}
