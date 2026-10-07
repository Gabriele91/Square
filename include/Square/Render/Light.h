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
#include "Square/Render/ConstantBuffer.h"

#define DIRECTION_SHADOW_CSM_NUMBER_OF_FACES 8 // cascades of a directional light at most
#define DIRECTION_SHADOW_CSM_DEFAULT_FACES 4   // its cascades by default

namespace Square
{
    namespace Render
    {
		class Texture;
		class Target;
        class Material;
		class Transform;
		class Camera;
		class ShadowBuffer;
		class Viewport;
    }
    namespace Geometry
    {
        class Sphere;
        class Frustum;
		class AABoundingBox;
    }
	namespace Render
	{
		class Camera;
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

	//the filter of the shadow of a light (directional: forward and deferred): NONE one sample
	//(hard, aliased), PCF a fixed kernel, PCSS soft (wider far from the caster, the default)
	enum class ShadowFilter : int
	{
		NONE,
		PCF,
		PCSS
	};

	//uniform buffers
	CBStruct UniformDirectionLight
    {
		CBAlignas Vec3 m_direction;
	    CBAlignas Vec3 m_diffuse;
		CBAlignas Vec3 m_specular;
    };

    CBStruct UniformPointLight
    {
		CBAlignas Vec3  m_position;
	    CBAlignas Vec3  m_diffuse;
		CBAlignas Vec3  m_specular;
	    float m_constant;
		float m_radius;
		float m_inside_radius;
    };

	CBStruct UniformSpotLight
	{
		CBAlignas Vec3  m_position;
	    CBAlignas Vec3  m_direction;
		CBAlignas Vec3  m_diffuse;
		CBAlignas Vec3  m_specular;
        float m_constant;      
		float m_radius;
		float m_inside_radius;
		float m_inner_cut_off;
		float m_outer_cut_off;
    };

	//uniform buffers
	CBStruct UniformDirectionShadowLight
    {
		CBArrMat4        m_projection[DIRECTION_SHADOW_CSM_NUMBER_OF_FACES];
		CBArrMat4        m_view[DIRECTION_SHADOW_CSM_NUMBER_OF_FACES];
		CBArrVec3        m_data[DIRECTION_SHADOW_CSM_NUMBER_OF_FACES]; // x: the depth (view) where the cascade ends; y, z: free (> hlsl packet rule: float4 for a float in array)
		IVec4            m_options{ 0, DIRECTION_SHADOW_CSM_DEFAULT_FACES, 0, 0 }; //x: the ShadowFilter, y: the cascades (a vector: its own register after the array)
	};

	CBStruct UniformPointShadowLight
    {
		Mat4      m_projection;
	    CBArrMat4 m_view[6];
		Vec4      m_position;
		float     m_radius;
    };

	CBStruct UniformSpotShadowLight
    {
		Mat4 m_projection;
		Mat4 m_view;
    };

	//lights
	class SQUARE_API Light : public BaseObject
	{
	public:

		SQUARE_OBJECT(Light)

		virtual const Geometry::Sphere& bounding_sphere() const = 0;

		virtual const Geometry::Frustum& frustum() const = 0;

		virtual Weak<Transform> transform() const = 0;

		//shadow
		virtual const ShadowBuffer& shadow_buffer() const = 0;

		virtual bool shadow() const = 0;

		virtual Vec4 shadow_viewport() const;

		//uniform light info
		virtual void set(UniformDirectionLight* data) const;

		virtual void set(UniformPointLight* data) const;

		virtual void set(UniformSpotLight* data) const;

		//uniform shadow light info
		virtual void set(UniformDirectionShadowLight* data, const Camera* camera, bool draw_shadow_map = true) const;

		virtual void set(UniformPointShadowLight* data, bool draw_shadow_map = true) const;

		virtual void set(UniformSpotShadowLight* data, bool draw_shadow_map = true) const;

		// For CSM
		virtual void set_scene_size(const Geometry::AABoundingBox& scene);

		//get viewport shadow camera

		bool visible() const;

		void visible(bool enable);

		LightType type() const;

		ShadowFilter shadow_filter() const;

		void shadow_filter(ShadowFilter filter);

    protected:
        
		void type(LightType type);
        
	private:
        
        LightType m_type{ LightType::NONE };
		bool m_visible{ true };
		ShadowFilter m_shadow_filter{ ShadowFilter::PCSS };
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
