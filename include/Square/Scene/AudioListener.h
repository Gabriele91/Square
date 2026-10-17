//
//  AudioListener.h
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"

namespace Square
{
namespace Scene
{
	//The ears of the AudioSystem, usually on the camera.
	//One for scene (with two the last updated wins).
	class SQUARE_API AudioListener : public Component
	{
	public:
		//A square object
		SQUARE_OBJECT(AudioListener)

		//Registration in context
		static void object_registration(Context& ctx);

		//Init
		AudioListener(Context& context);
		virtual ~AudioListener();

		//all events
		virtual void on_attach(Actor& entity) override;
		virtual void on_late_update(double delta_time) override;

		//serialize
		virtual void serialize(Data::Archive& archive) override;
		virtual void serialize_json(Data::JsonValue& archive) override;
		//deserialize
		virtual void deserialize(Data::Archive& archive) override;
		virtual void deserialize_json(Data::JsonValue& archive) override;

	private:

		Vec3 m_last_position{ 0.0f }; //for the velocity
		bool m_has_last{ false };
	};
}
}
