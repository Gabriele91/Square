//
//  Properties.cpp
//  Square
//
//  See Properties.h for the high level description.
//
#include <algorithm>
#include <sstream>
#include "Square/Config.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/Properties.h"
#include "Square/Core/ClassObjectRegistration.h"

namespace Square
{
namespace Scene
{
	SQUARE_CLASS_OBJECT_REGISTRATION(Properties);

	template < typename T, T Properties::Packed::* Field >
	void Properties::packed_attribute(Square::Context& ctx, const char* name)
	{
		ctx.add_attribute_function<Properties, T>
			(name
			, T()
			, [](const Properties* properties) -> T { return properties->pack().*Field; }
			, [](Properties* properties, const T& value) { properties->m_packed.*Field = value; });
	}

	//regs
	void Properties::object_registration(Square::Context& ctx)
	{
		ctx.add_object<Properties>();
		packed_attribute< std::vector<std::string>, &Packed::m_names >(ctx, "names");
		packed_attribute< std::vector<int>, &Packed::m_types >(ctx, "types");
		packed_attribute< std::vector<Vec4>, &Packed::m_numbers >(ctx, "numbers");
		packed_attribute< std::vector<std::string>, &Packed::m_strings >(ctx, "strings");
	}

	Properties::Properties(Square::Context& context)
	: Component(context)
	{
	}

	Properties::~Properties()
	{
	}

	void Properties::set(const std::string& name, bool value)
	{
		Value& out = m_values[name];
		out = Value();
		out.m_type = P_BOOLEAN;
		out.m_numbers.x = value ? 1.0f : 0.0f;
	}

	void Properties::set(const std::string& name, float value)
	{
		Value& out = m_values[name];
		out = Value();
		out.m_type = P_NUMBER;
		out.m_numbers.x = value;
	}

	void Properties::set(const std::string& name, const Vec4& value, int components)
	{
		Value& out = m_values[name];
		out = Value();
		out.m_type = P_VECTOR;
		out.m_numbers = value;
		out.m_components = std::clamp(components, 1, 4);
	}

	void Properties::set(const std::string& name, const std::string& value)
	{
		Value& out = m_values[name];
		out = Value();
		out.m_type = P_STRING;
		out.m_string = value;
	}

	void Properties::remove(const std::string& name)
	{
		m_values.erase(name);
	}

	bool Properties::has(const std::string& name) const
	{
		return m_values.find(name) != m_values.end();
	}

	bool Properties::boolean(const std::string& name, bool fallback) const
	{
		bool out = fallback;
		auto found = m_values.find(name);
		if (found != m_values.end())
		{
			const Value& value = found->second;
			switch (value.m_type)
			{
			case P_STRING:
				if (value.m_string == "true")  out = true;
				if (value.m_string == "false") out = false;
				break;
			default:
				out = value.m_numbers.x != 0.0f;
				break;
			}
		}
		return out;
	}

	float Properties::number(const std::string& name, float fallback) const
	{
		float out = fallback;
		auto found = m_values.find(name);
		if (found != m_values.end() && found->second.m_type != P_STRING)
		{
			out = found->second.m_numbers.x;
		}
		return out;
	}

	Vec4 Properties::vector(const std::string& name, const Vec4& fallback) const
	{
		Vec4 out = fallback;
		auto found = m_values.find(name);
		if (found != m_values.end() && found->second.m_type != P_STRING)
		{
			out = found->second.m_numbers;
		}
		return out;
	}

	std::string Properties::string(const std::string& name, const std::string& fallback) const
	{
		std::string out = fallback;
		auto found = m_values.find(name);
		if (found != m_values.end())
		{
			const Value& value = found->second;
			std::ostringstream text;
			switch (value.m_type)
			{
			case P_STRING:  out = value.m_string; break;
			case P_BOOLEAN: out = value.m_numbers.x != 0.0f ? "true" : "false"; break;
			case P_NUMBER:  text << value.m_numbers.x; out = text.str(); break;
			default:
				for (int i = 0; i < value.m_components; ++i)
				{
					text << (i ? " " : "") << value.m_numbers[i];
				}
				out = text.str();
				break;
			}
		}
		return out;
	}

	//packed
	Properties::Packed Properties::pack() const
	{
		Packed packed;
		for (const auto& [name, value] : m_values)
		{
			packed.m_names.push_back(name);
			packed.m_types.push_back(int(value.m_type) * 8 + value.m_components);
			packed.m_numbers.push_back(value.m_numbers);
			packed.m_strings.push_back(value.m_string);
		}
		return packed;
	}

	void Properties::unpack(const Packed& packed)
	{
		m_values.clear();
		const size_t count = std::min({ packed.m_names.size(), packed.m_types.size(), packed.m_numbers.size(), packed.m_strings.size() });
		for (size_t i = 0; i < count; ++i)
		{
			Value value;
			value.m_type = Type(packed.m_types[i] / 8);
			value.m_components = std::clamp(packed.m_types[i] % 8, 1, 4);
			value.m_numbers = packed.m_numbers[i];
			value.m_string = packed.m_strings[i];
			m_values[packed.m_names[i]] = value;
		}
	}

	//serialize
	void Properties::serialize(Data::Archive& archive)
	{
		Data::serialize(archive, this);
	}

	void Properties::serialize_json(Data::JsonValue& archive)
	{
		Data::serialize_json(archive, this);
	}

	void Properties::deserialize(Data::Archive& archive)
	{
		Data::deserialize(archive, this);
		unpack(m_packed);
		m_packed = Packed();
	}

	void Properties::deserialize_json(Data::JsonValue& archive)
	{
		Data::deserialize_json(archive, this);
		unpack(m_packed);
		m_packed = Packed();
	}
}
}
