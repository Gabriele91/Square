//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright � 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/SmartPointers.h"

namespace Square
{
	//Declaretion
	class Context;
	class Object;
	class ObjectInfo;
	class ObjectFactory;

	//Info about object
	class SQUARE_API ObjectInfo
	{
	public:
		//object info
		ObjectInfo(const std::string& name);

		//compute id
		static uint64 compute_id(const std::string& name);

		//object id
		uint64 id() const;

		//object name
		const std::string& name() const;


	protected:

		uint64		m_id;
		std::string m_name;

	};

	//Define object
	#define SQUARE_OBJECT(Class)\
	    static const ::Square::ObjectInfo& static_object_info()\
			{ static ::Square::ObjectInfo obj_info(#Class); return obj_info; }\
		static ::Square::uint64 static_object_id()\
			{ return static_object_info().id(); }\
		static const std::string& static_object_name()\
			{ return static_object_info().name(); }\
		::Square::uint64 object_id() const override\
			{ return Class::static_object_id(); }\
		const std::string& object_name() const override\
			{ return Class::static_object_name(); }\
		const ::Square::ObjectInfo& object_info() const override\
			{ return Class::static_object_info(); }

	//Define Object
    class SQUARE_API Object
	{
	public:

		virtual uint64 object_id() const = 0;
		virtual const std::string& object_name() const = 0;
		virtual const ObjectInfo& object_info() const = 0;

	};

	//Factory of a object
	class SQUARE_API ObjectFactory
	{
	public:

		ObjectFactory(const ObjectInfo& info) : m_info(info) { }

		const ObjectInfo& info() { return m_info; }

		virtual Shared<Object> create() = 0;

	protected:

		const ObjectInfo& m_info;

	};

	template< typename T >
	//Implementation of a factory
	class SQUARE_API ObjectFactoryItem : public ObjectFactory
	{
	public:

		//Object factory
		ObjectFactoryItem() : ObjectFactory(T::static_object_info()) { }
		//Create
		virtual Shared<Object> create()
		{
			return StaticPointerCast<Object>(std::make_shared<T>());
		}

	protected:

		const ObjectInfo* m_info;

	};
}
