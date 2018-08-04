//
//  Light.h
//  Square
//
//  Created by Gabriele Di Bari on 10/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Math/Linear.h"

namespace Square
{
    namespace Render
    {
        class Material;
        class Transform;
        class Camera;
    }
    namespace Geometry
    {
        class Sphere;
        class Frustum;
    }
}

namespace Square
{
namespace Render
{
	enum class LightType
	{
        NONE,
		SPOT,
		POINT,
		DIRECTION
	};

	//uniform buffers
    ConstantBufferStruct UniformDirectionLight
    {
		Vec3 m_direction; float __PADDING0__; //v4
        Vec3 m_diffuse;   float __PADDING1__; //v4
        Vec3 m_specular;  float __PADDING2__; //v4
    };

    ConstantBufferStruct UniformPointLight
    {
	    Vec3  m_position;      float __PADDING0__; //v4
        Vec3  m_diffuse;       float __PADDING1__; //v4
        Vec3  m_specular;      
        float m_constant;      
        float m_radius;        
        float m_inside_radius; 
    };

    ConstantBufferStruct UniformSpotLight
    {
	    Vec3  m_position;      float __PADDING0__; //v4
		Vec3  m_direction;     float __PADDING1__; //v4
        Vec3  m_diffuse;       float __PADDING2__; //v4
        Vec3  m_specular;      
        float m_constant;      
        float m_radius;        
        float m_inside_radius; 
        float m_inner_cut_off; 
        float m_outer_cut_off; 
    };

	//lights
    class SQUARE_API Light : public BaseObject
    {
	public:
		
		SQUARE_OBJECT(Light)

        virtual const Geometry::Sphere& bounding_sphere() const = 0;
        
        virtual const Geometry::Frustum& frustum() const = 0;

        virtual Weak<Transform> transform() const = 0;
        
        virtual Weak<Camera> camera() const = 0;

		bool visible() const;
        
		void visible(bool enable);

		bool shadow_caster() const;
        
		void shadow_caster(bool enable);

		LightType type() const;

		virtual void set(UniformDirectionLight* data) const;

		virtual void set(UniformPointLight* data) const;

		virtual void set(UniformSpotLight* data) const;

    protected:
        
		void type(LightType type);
        
	private:
        
        LightType m_type{ LightType::NONE };
		bool m_visible{ true };
        bool m_shadow_caster{ false };
	};
	
    class SQUARE_API DirectionLight : public Light
    {
    public:
        
		DirectionLight();
        
		virtual Vec3 diffuse()  const;
		virtual Vec3 specular() const;

		virtual void diffuse(const Vec3& diffuse);
		virtual void specular(const Vec3& specular);

		virtual void set(UniformDirectionLight* data) const override;
        
    public:
        Vec3  m_diffuse;
        Vec3  m_specular;
    };
    
    class SQUARE_API PointLight : public Light
    {
    public:
     
		PointLight();
        
		virtual float inside_radius() const;
		virtual float radius()        const;
		virtual Vec3  diffuse()       const;
		virtual Vec3  specular()      const;
		virtual float constant()      const;
        
		virtual void radius(const float radius, const float inside);
		virtual void inside_radius(const float inside_radius);
		virtual void radius(const float radius);
		virtual void diffuse(const Vec3& diffuse);
		virtual void specular(const Vec3& specular);
		virtual void constant(const float constant);
        
		virtual void set(UniformPointLight* data) const override;
        
    public:
        Vec3  m_diffuse;
        Vec3  m_specular;
        float m_constant;
        float m_radius;
        float m_inside_radius;
    };
        
    class SQUARE_API SpotLight : public Light
    {
    public:
        
		SpotLight();
        
		virtual float inner_cut_off() const;
		virtual float outer_cut_off() const;
		virtual float inside_radius() const;
		virtual float radius()        const;
		virtual Vec3  diffuse()       const;
		virtual Vec3  specular()      const;
		virtual float constant()      const;
        
		virtual void cut_off(const float outer_coff, const float inner_coff);
		virtual void inner_cut_off(const float coff);
		virtual void outer_cut_off(const float coff);
		virtual void radius(const float radius, const float inside);
		virtual void inside_radius(const float inside_radius);
		virtual void radius(const float radius);
		virtual void diffuse(const Vec3& diffuse);
		virtual void specular(const Vec3& specular);
		virtual void constant(const float constant);
        
		virtual void set(UniformSpotLight* data) const override;
    public:
        Vec3  m_diffuse;
        Vec3  m_specular;
        float m_constant;
        float m_radius;
        float m_inside_radius;
        float m_inner_cut_off;
        float m_outer_cut_off;
    };
}
}
