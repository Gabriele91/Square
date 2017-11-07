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
	//Class used by all classes with serializable attributes,
    class Serializable;
    
	//Access to attribute
	class AttributeAccess : public SharedObject<AttributeAccess>
	{
	public:
		virtual void Get(const Serializable* serializable, VariantRef& ret) const =0;
		virtual void Set(Serializable* serializable, const VariantRef& set) =0;
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

		//attribute of a field
		Attribute
		(
			const std::string& name,
			VariantType     value_type,
			const Variant&  default_value,
			size_t          offset,
			Type			type = DEFAULT
		)
		{
			m_name = name;
			m_type = type;
			m_value_type = value_type;
			m_default = default_value;
			m_offset = offset;
			m_wrapper = nullptr;
			m_enum_names = nullptr;
		}

		//attribute by wrapper
		Attribute
		(
			const std::string&      name,
			VariantType             value_type,
			const Variant&          default_value,
			Shared<AttributeAccess> access,
			Type			        type = DEFAULT
		)
		{
			m_name = name;
			m_type = type;
			m_value_type = value_type;
			m_default = default_value;
			m_offset = 0;
			m_wrapper = access;
			m_enum_names = nullptr;
		}


		//attribute is a enum
		Attribute
		(
			const std::string& name,
			VariantType     value_type,
			const Variant&  default_value,
			const char**    enum_names,
			Type			type = DEFAULT
		)
		{
			m_name = name;
			m_type = type;
			m_value_type = value_type;
			m_default = default_value;
			m_offset = 0;
			m_wrapper = nullptr;
			m_enum_names = enum_names;
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
		virtual void Get(const Serializable* serializable, VariantRef& ret) const override
		{
			assert(serializable);
			const T* self = static_cast<const T*>(serializable);
			ret = (self->*m_get)();
		}

		// Invoke setter function.
		virtual void Set(Serializable* serializable, const VariantRef& value) override
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
	template <typename T, typename U, typename Trait = AttributeTrain<U> >
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
		virtual void Get(const Serializable* serializable, VariantRef& ret) const override
		{
			assert(serializable);
			const T* self = static_cast<const T*>(serializable);
			ret = (self->*m_get)();
		}

		// Invoke setter function.
		virtual void Set(Serializable* serializable, const VariantRef& value) override
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
			, variant_traits<T>()
			, default_value
			, std::shared_ptr<AttributeAccess>(
				(AttributeAccess*)new AttributeAccessMethod< T, U, Trait >(get_method, set_method)
			)
			, type
		);
	}

	template<typename T, typename U, typename Trait = AttributeTrain<U> >
	static Attribute attribute_function
	(
		  const std::string& attribute_name
		, const Variant& default_value
		, typename AttributeAccessFunction< T, U, Trait >::GetFunction get_function
		, typename AttributeAccessFunction< T, U, Trait >::SetFunction set_function
		, Attribute::Type type = Attribute::DEFAULT
	)
	{
		return Attribute(
			  attribute_name
			, variant_traits<T>()
			, default_value
			, std::shared_ptr<AttributeAccess>(
				(AttributeAccess*)new AttributeAccessMethod< T, U, Trait >(get_function, set_function)
			)
			, type
		);
	}

	template<typename T, typename U >
	static Attribute attribute_field
	(
		  const std::string& field_name
		, const Variant& default_value
		, U T::*member
		, Attribute::Type type = Attribute::DEFAULT
	)
	{
		return Attribute(
			  field_name
			, variant_traits<T>()
			, default_value
			, (char*)&((T*)nullptr->*member) - (char*)nullptr
			, type
		);
	}
}