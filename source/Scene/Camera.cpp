//
//  Camera.cpp
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Object.h"
#include "Square/Core/Context.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/Camera.h"

namespace Square
{
namespace Scene
{
    //Add element to objects
    SQUARE_CLASS_OBJECT_REGISTRATION(Camera);
    
    //Registration in context
    void Camera::object_registration(Context& ctx)
    {
        //factory
        ctx.add_object<Camera>();
        //Attributes
		ctx.add_attributes<Camera>(attribute_function<Camera, bool>
		("enable"
		, bool(0)
		, [](const Camera* plight) -> bool      { return plight->enable(); }
		, [](Camera* plight, const bool& enable){ plight->enable(enable); }));

        ctx.add_attributes<Camera>(attribute_function<Camera, Vec4>
        ("viewport"
        , Vec4(0,0,1,1)
        , [](const Camera* camera) -> Vec4   { return camera->viewport(); }
        , [](Camera* camera, const Vec4& viewport){ camera->viewport(viewport); }));
        
        ctx.add_attributes<Camera>(attribute_function<Camera, Mat4>
        ("projection"
        , Mat4(1)
        , [](const Camera* camera) -> Mat4  { return camera->viewport().projection(); }
        , [](Camera* camera, const Mat4& sc){ camera->projection(sc); }));
    }
    
    
    //init
    Camera::Camera(Context& context)
    : Component(context)
    , m_is_dirty(true)
    {
    }
    
    
    
    //all events
    void Camera::on_attach(Actor& entity) { m_is_dirty = true; m_frustum_is_dirty = true; }
    void Camera::on_deattch()             { m_is_dirty = true; m_frustum_is_dirty = true; }
    void Camera::on_transform()           { m_is_dirty = true; m_frustum_is_dirty = true; }
    void Camera::on_message(const Message& msg){}
    
    //set
    void Camera::viewport(const IVec4& viewport)
    {
        m_viewport.viewport(viewport);
    }
    void Camera::projection(const Mat4& projection)
    {
        m_viewport.projection(projection); m_frustum_is_dirty = true;
    }
    void Camera::perspective(float fov, float aspect, float near, float far)
    {
        m_viewport.perspective(fov, aspect, near, far); m_frustum_is_dirty = true;
    }
    void Camera::ortogonal(float left, float right, float top, float bottom, float near, float far)
    {
        m_viewport.ortogonal(left,right,top,bottom,near,far); m_frustum_is_dirty = true;
    }
    
    //serialize
    void Camera::serialize(Data::Archive& archivie)
    {
        Data::serialize(archivie, this);
    }
    void Camera::serialize_json(Data::Json& archivie)
    {
        
    }
    
    //deserialize
    void Camera::deserialize(Data::Archive& archivie)
    {
        Data::deserialize(archivie, this);
    }
    void Camera::deserialize_json(Data::Json& archivie)
    {
        
    }
    
    //camera info
    const Mat4& Camera::model() const
    {
        if(auto actor = Component::actor().lock())
        {
            return actor->global_model_matrix();
        }
        return Constants::identity<Mat4>();
    }
    const Mat4& Camera::view() const
    {
        if(m_is_dirty)
        {
            if(auto actor = Component::actor().lock())
            {
                Mat4 const& model_matrix = actor->global_model_matrix();
                m_view = Square::inverse(model_matrix);
            }
            else
            {
                m_view = Constants::identity<Mat4>();
            }
            //dirty, not anymore
            m_is_dirty = false;
        }
        return m_view;
    }
    const Geometry::Frustum& Camera::frustum() const
    {
        if(m_frustum_is_dirty)
        {
            m_frustum.update_frustum(m_viewport.projection() * view());
            m_frustum_is_dirty = false;
        }
        return m_frustum;
    }
    const Render::Viewport& Camera::viewport() const
    {
        return m_viewport;
    }
    
    //set
    void Camera::set(Render::UniformBufferCamera *gpubuffer) const
    {
        //info camera
        gpubuffer->m_projection = viewport().projection();
        gpubuffer->m_view = view();
        //invo camera in world space
        if(auto actor = Component::actor().lock())
        {
            gpubuffer->m_model = actor->global_model_matrix();
            gpubuffer->m_position = actor->position(true);
        }
        else
        {
            gpubuffer->m_model = Constants::identity<Mat4>();
            gpubuffer->m_position = Vec3(0);
        }

    }
    
}
}
