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

    class SQUARE_API Light : public BaseObject
    {
	public:
		
		SQUARE_OBJECT(Light)

        virtual const Geometry::Sphere& bounding_sphere() const = 0;
        
        virtual const Geometry::Frustum& frustum() const = 0;

        virtual Weak<Transform> transform() const = 0;
        
        virtual Weak<Camera> camera() const = 0;

        bool visible() const { return m_visible; }
        
        void visible(bool enable) { m_visible = enable; }

        bool shadow_caster() const { return m_shadow_caster; }
        
        void shadow_caster(bool enable) { m_shadow_caster = enable; }

        LightType type() const { return m_type; }

    protected:
        
        void type(LightType type){ m_type=type; }
        
	private:
        
        LightType m_type{ LightType::NONE };
		bool m_visible{ true };
        bool m_shadow_caster{ false };
	};
    
    PACKED(ConstantBufferStruct UniformDirectionLight
    {
        Vec3 m_diffuse;  float __PADDING0__{0.0}; //v4
        Vec3 m_specular; float __PADDING1__{0.0}; //v4
    });

    class SQUARE_API DirectionLight : public Light
    {
    public:
        
        DirectionLight()
        {
            type(LightType::DIRECTION);
        }
        
        Vec3 diffuse()  const { return m_diffuse;  }
        Vec3 specular() const { return m_specular; }

        void diffure(const Vec3& diffuse)   { m_diffuse  = diffuse;  }
        void specular(const Vec3& specular) { m_specular = specular; }

        void set(UniformDirectionLight* data)
        {
            data->m_diffuse = diffuse();
            data->m_specular = specular();
        }
        
    public:
        Vec3  m_diffuse;
        Vec3  m_specular;
    };
    
    PACKED(ConstantBufferStruct UniformPointLight
    {
        Vec3  m_diffuse;       float __PADDING0__{0.0}; //v4
        Vec3  m_specular;      float __PADDING1__{0.0}; //v4
        float m_constant;      float __PADDING2__{0.0}; //v2
        float m_radius;        float __PADDING3__{0.0}; //v2
        float m_inside_radius; float __PADDING4__{0.0}; //v2
    });
    
    class SQUARE_API PointLight : public Light
    {
    public:
     
        PointLight()
        {
            type(LightType::POINT);
        }
        
        float inside_radius() const { return m_inside_radius; }
        float radius()        const { return m_radius; }
        Vec3  diffuse()       const { return m_diffuse;  }
        Vec3  specular()      const { return m_specular; }
        float constant()      const { return m_constant; }
        
        void radius(const float radius, const float inside)
        {
            m_radius  = radius;
            m_inside_radius = inside;
        }
        void inside_radius(const float inside_radius) { m_inside_radius  = inside_radius; }
        void radius(const float radius)               { m_radius  = radius;     }
        void diffure(const Vec3& diffuse)             { m_diffuse  = diffuse;   }
        void specular(const Vec3& specular)           { m_specular = specular;  }
        void constant(const float constant)           { m_constant  = constant; }
        
        void set(UniformPointLight* data)
        {
            data->m_diffuse = diffuse();
            data->m_specular = specular();
            data->m_constant = constant();
            data->m_radius = radius();
            data->m_inside_radius = inside_radius();
        }
        
    public:
        Vec3  m_diffuse;
        Vec3  m_specular;
        float m_constant;
        float m_radius;
        float m_inside_radius;
    };
    
    PACKED(ConstantBufferStruct UniformSpotLight
    {
        Vec3  m_diffuse;       float __PADDING0__{0.0}; //v4
        Vec3  m_specular;      float __PADDING1__{0.0}; //v4
        float m_constant;      float __PADDING2__{0.0}; //v2
        float m_radius;        float __PADDING3__{0.0}; //v2
        float m_inside_radius; float __PADDING4__{0.0}; //v2
        float m_inner_cut_off; float __PADDING5__{0.0}; //v2
        float m_outer_cut_off; float __PADDING6__{0.0}; //v2
    });
    
    class SQUARE_API SpotLight : public Light
    {
    public:
        
        SpotLight()
        {
            type(LightType::SPOT);
        }
        
        float inner_cut_off() const { return m_inner_cut_off; }
        float outer_cut_off() const { return m_outer_cut_off; }
        float inside_radius() const { return m_inside_radius; }
        float radius()        const { return m_radius;   }
        Vec3  diffuse()       const { return m_diffuse;  }
        Vec3  specular()      const { return m_specular; }
        float constant()      const { return m_constant; }
        
        void cut_off(const float outer_coff, const float inner_coff)
        {
            m_outer_cut_off = outer_coff;
            m_inner_cut_off = inner_coff;
        }
        void inner_cut_off(const float coff) { m_inner_cut_off = coff; }
        void outer_cut_off(const float coff) { m_outer_cut_off = coff; }
        void radius(const float radius, const float inside)
        {
            m_radius  = radius;
            m_inside_radius = inside;
        }
        void inside_radius(const float inside_radius) { m_inside_radius  = inside_radius; }
        void radius(const float radius)               { m_radius  = radius;    }
        void diffure(const Vec3& diffuse)             { m_diffuse  = diffuse;  }
        void specular(const Vec3& specular)           { m_specular = specular; }
        void constant(const float constant)           { m_constant  = constant;}
        
        void set(UniformSpotLight* data)
        {
            data->m_diffuse = diffuse();
            data->m_specular = specular();
            data->m_constant = constant();
            data->m_radius = radius();
            data->m_inside_radius = inside_radius();
            data->m_inner_cut_off = std::cos(inner_cut_off());
            data->m_outer_cut_off = std::cos(outer_cut_off());
        }
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
