//  Square
//
//  Created by Gabriele Di Bari on 08/06/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Object.h"
#include "Square/Render/ConstantBuffer.h"

namespace Square
{
namespace Render
{
    //Transform uniform buffer
    CBStruct UniformBufferTransform
    {
        Mat4 m_model;
		Mat4 m_inv_model;
		Mat4 m_rotation;
		CBAlignas Vec3 m_position;
		CBAlignas Vec3 m_scale;
    };
    //Transform cpu class
	class SQUARE_API Transform : public BaseObject
	{
	public:
		//Transform
		SQUARE_OBJECT(Transform)
		//matrix op
		virtual void translation(const Vec3& vector) = 0;
		virtual void move(const Vec3& vector) = 0;
		virtual void turn(const Quat& rot) = 0;

		virtual void position(const Vec3& pos) = 0;
		virtual void rotation(const Quat& rot) = 0;
		virtual void scale(const Vec3& pos) = 0;

		virtual Vec3 position(bool global = false) const = 0;
		virtual Quat rotation(bool global = false) const = 0;
		virtual Vec3 scale(bool global = false) const = 0;

		virtual Mat4 const& local_model_matrix() const = 0;
		virtual Mat4 const& global_model_matrix() const = 0;
        
        //set gpu buffer
        virtual void set(UniformBufferTransform* gpubuffer) const = 0;
	};
}
}
