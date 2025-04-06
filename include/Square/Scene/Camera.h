//
//  Camera.h
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Viewport.h"
#include "Square/Geometry/Frustum.h"
#include "Square/Scene/Component.h"


namespace Square
{
namespace Scene
{
    //Class Declaretion
    class  Actor;
    //Camera
    class SQUARE_API Camera : public Component
                            , public SharedObject<Camera>
                            , public Render::Camera
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
        virtual void serialize(Data::Archive& archive)   override;
        virtual void serialize_json(Data::JsonValue& archive) override;
        //deserialize
        virtual void deserialize(Data::Archive& archive)   override;
        virtual void deserialize_json(Data::JsonValue& archive) override;
        
        //set
        void viewport(const IVec4& viewport);
        void projection(const Mat4& projection);
        void perspective(float fov, float aspect, float near, float far);
        void ortogonal(float left, float right, float top, float bottom, float near, float far);

        //get
        const Mat4& model() const override;
        const Mat4& view() const override;
        const Mat4& projection() const override;
        const Render::Viewport& viewport() const override;
        const Geometry::Frustum& frustum() const override;
        
        //set values to constant buffer
        void set(Render::UniformBufferCamera* gpubuffer) const override;

    protected:
        Render::Viewport          m_viewport;         //viewport info
        mutable Mat4              m_view;             //inverse of model matrix of actor
        mutable bool              m_is_dirty{ true }; // for view matrix
        mutable Geometry::Frustum m_frustum;                  //fustrum
        mutable bool              m_frustum_is_dirty{ true }; // for fustrum
    };
}
}
