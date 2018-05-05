//
//  Camera.h
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Render/Viewport.h"
#include "Square/Geometry/Frustum.h"
#include "Square/Scene/Component.h"


namespace Square
{
namespace Scene
{
    //Class Declaretion
    class  Actor;
    //Camera uniform buffer
    ConstantBufferStruct UniformBufferCamera
    {
        Vec4 m_viewport;
        Mat4 m_projection;
        Mat4 m_view;
        Mat4 m_model;
        Vec3 m_position;
    };
    //Camera
    class SQUARE_API Camera : public Component
                            , public SharedObject<Camera>
    {
    public:
        
        //A square object
        SQUARE_OBJECT(Camera)
        
        //Reg object
        static void object_registration(Context& ctx);
        
        //Init
        Camera(Context& context);
        
        //all events
        virtual void on_attach(Actor& entity)      override;
        virtual void on_deattch()                  override;
        virtual void on_transform()                override;
        virtual void on_message(const Message& msg)override;
        
        //serialize
        virtual void serialize(Data::Archive& archivie)   override;
        virtual void serialize_json(Data::Json& archivie) override;
        //deserialize
        virtual void deserialize(Data::Archive& archivie)   override;
        virtual void deserialize_json(Data::Json& archivie) override;
        
        //set
        void viewport(const IVec4& viewport);
        void projection(const Mat4& projection);
        void perspective(float fov, float aspect, float near, float far);
        void ortogonal(float left, float right, float top, float bottom, float near, float far);

        //get
        const Render::Viewport& viewport() const;
        const Mat4& view() const;
        const Geometry::Frustum& frustum() const;
        
        //set values to constant buffer
        void set(UniformBufferCamera* gpubuffer);

    protected:
        Geometry::Frustum m_frustum; //fustrum
        Render::Viewport  m_viewport;//viewport info
        mutable Mat4      m_view;    //inverse of model matrix of actor
        mutable bool      m_is_dirty;
        void update_frustum();
    };
}
}
