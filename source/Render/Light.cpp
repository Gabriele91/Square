#include "Square/Render/Light.h"

namespace Square
{
namespace Render
{
	//////////////
	//  LIGHTS  //
	//////////////
	bool Light::visible() const { return m_visible; }

	void Light::visible(bool enable) { m_visible = enable; }

	bool Light::shadow_caster() const { return m_shadow_caster; }

	void Light::shadow_caster(bool enable) { m_shadow_caster = enable; }

	LightType Light::type() const { return m_type; }

	void Light::set(UniformDirectionLight* data) const {};

	void Light::set(UniformPointLight* data) const {};

	void Light::set(UniformSpotLight* data) const {};

	void Light::type(LightType type) { m_type = type; }

	//////////////////////
	// DIRECTION LIGHTS //
	//////////////////////
	DirectionLight::DirectionLight() 
	{ 
		type(LightType::DIRECTION);
		diffuse(Vec3{ 1.0,1.0,1.0 });
		specular(Vec3{ 1.0,1.0,1.0 });
	}

	Vec3 DirectionLight::diffuse()  const { return m_diffuse; }
	Vec3 DirectionLight::specular() const { return m_specular; }

	void DirectionLight::diffuse(const Vec3& diffuse) { m_diffuse = diffuse; }
	void DirectionLight::specular(const Vec3& specular) { m_specular = specular; }

	void DirectionLight::set(UniformDirectionLight* data) const
	{
		data->m_diffuse = diffuse();
		data->m_specular = specular();
	}
	
	//////////////////
	// POINT LIGHTS //
	//////////////////
	PointLight::PointLight()
	{ 
		type(LightType::POINT); 
		radius(1.0);
		inside_radius(1.0);
		diffuse(Vec3{ 1.0,1.0,1.0 });
		specular(Vec3{ 1.0,1.0,1.0 });
		constant(1.0);
	}

	float PointLight::inside_radius() const { return m_inside_radius; }
	float PointLight::radius()        const { return m_radius; }
	Vec3  PointLight::diffuse()       const { return m_diffuse; }
	Vec3  PointLight::specular()      const { return m_specular; }
	float PointLight::constant()      const { return m_constant; }

	void PointLight::radius(const float light_radius, const float light_inside)
	{
		radius(light_radius);
		inside_radius(light_inside);
	}
	void PointLight::inside_radius(const float inside_radius) { m_inside_radius = inside_radius; }
	void PointLight::radius(const float radius) { m_radius = radius; }
	void PointLight::diffuse(const Vec3& diffuse) { m_diffuse = diffuse; }
	void PointLight::specular(const Vec3& specular) { m_specular = specular; }
	void PointLight::constant(const float constant) { m_constant = constant; }

	void PointLight::set(UniformPointLight* data) const
	{
		data->m_diffuse = diffuse();
		data->m_specular = specular();
		data->m_constant = constant();
		data->m_radius = radius();
		data->m_inside_radius = inside_radius();
	}

	//////////////////
	// POINT LIGHTS //
	//////////////////
	SpotLight::SpotLight()
	{ 
		type(LightType::SPOT); 
		radius(1.0);
		inside_radius(1.0);
		diffuse(Vec3{ 1.0,1.0,1.0 });
		specular(Vec3{ 1.0,1.0,1.0 });
		constant(1.0);
		inner_cut_off(radians(45.0));
		outer_cut_off(radians(90.0));
	}

	float SpotLight::inner_cut_off() const { return m_inner_cut_off; }
	float SpotLight::outer_cut_off() const { return m_outer_cut_off; }
	float SpotLight::inside_radius() const { return m_inside_radius; }
	float SpotLight::radius()        const { return m_radius; }
	Vec3  SpotLight::diffuse()       const { return m_diffuse; }
	Vec3  SpotLight::specular()      const { return m_specular; }
	float SpotLight::constant()      const { return m_constant; }

	void SpotLight::cut_off(const float outer_coff, const float inner_coff)
	{
		m_outer_cut_off = outer_coff;
		m_inner_cut_off = inner_coff;
	}
	void SpotLight::inner_cut_off(const float coff) { m_inner_cut_off = coff; }
	void SpotLight::outer_cut_off(const float coff) { m_outer_cut_off = coff; }
	void SpotLight::radius(const float light_radius, const float light_inside)
	{
		radius(light_radius);
		inside_radius(light_inside);
	}
	void SpotLight::inside_radius(const float inside_radius) { m_inside_radius = inside_radius; }
	void SpotLight::radius(const float radius) { m_radius = radius; }
	void SpotLight::diffuse(const Vec3& diffuse) { m_diffuse = diffuse; }
	void SpotLight::specular(const Vec3& specular) { m_specular = specular; }
	void SpotLight::constant(const float constant) { m_constant = constant; }

	void SpotLight::set(UniformSpotLight* data) const
	{
		data->m_diffuse = diffuse();
		data->m_specular = specular();
		data->m_constant = constant();
		data->m_radius = radius();
		data->m_inside_radius = inside_radius();
		data->m_inner_cut_off = std::cos(inner_cut_off());
		data->m_outer_cut_off = std::cos(outer_cut_off());
	}

	/////////////////
	// AREA LIGHTS //
	/////////////////
}
}