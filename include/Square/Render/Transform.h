//  Square
//
//  Created by Gabriele Di Bari on 08/06/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Object.h"

namespace Square
{
namespace Render
{
	class Transform : public BaseObject
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
	};
}
}