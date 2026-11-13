//
//  Decal.cpp
//  Square
//
//  See Decal.h.
//
#include "Square/Core/Object.h"
#include "Square/Core/Context.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Geometry/Sphere.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/Decal.h"

namespace Square
{
namespace Scene
{
	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(Decal);

	//Registration in context
	void Decal::object_registration(Context& ctx)
	{
		//factory
		ctx.add_object<Decal>();
		//Attributes
		ctx.add_attribute_function<Decal, bool>
		("visible"
		, true
		, [](const Decal* decal) -> bool        { return decal->visible(); }
		, [](Decal* decal, const bool& visible) { decal->visible(visible); });
		ctx.add_attribute_function<Decal, Vec4>
		("color"
		, Vec4(1.0f)
		, [](const Decal* decal) -> Vec4        { return decal->color(); }
		, [](Decal* decal, const Vec4& color)   { decal->color(color); });
		ctx.add_attribute_function<Decal, Vec3>
		("size"
		, Vec3(1.0f)
		, [](const Decal* decal) -> Vec3        { return decal->size(); }
		, [](Decal* decal, const Vec3& size)    { decal->size(size); });
	}

	Decal::Decal(Context& context)
	: Component(context)
	, SharedObject<Decal>(context.allocator())
	{
	}

	void Decal::size(const Vec3& half)
	{
		m_size = half;
	}

	const Vec3& Decal::size() const
	{
		return m_size;
	}

	Mat4 Decal::box() const
	{
		Mat4 model = Constants::identity<Mat4>();
		if (auto ptr_actor = actor().lock())
		{
			model = ptr_actor->global_model_matrix();
		}
		return model * scale(Constants::identity<Mat4>(), m_size);
	}

	const Geometry::Sphere& Decal::bounding_sphere() const
	{
		//around its box where it is now (its half diagonal, as scaled)
		const Mat4 model = box();
		m_sphere.position(Vec3(model[3]));
		m_sphere.radius(length(Vec3(model[0])) + length(Vec3(model[1])) + length(Vec3(model[2])));
		return m_sphere;
	}

	//serialize
	void Decal::serialize(Data::Archive& archive)          { Data::serialize(archive, this); }
	void Decal::serialize_json(Data::JsonValue& archive)   { Data::serialize_json(archive, this); }
	//deserialize
	void Decal::deserialize(Data::Archive& archive)        { Data::deserialize(archive, this); }
	void Decal::deserialize_json(Data::JsonValue& archive) { Data::deserialize_json(archive, this); }
}
}
