//
//  AudioListener.cpp
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Object.h"
#include "Square/Core/Context.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/System/AudioSystem.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/AudioListener.h"

namespace Square
{
namespace Scene
{
	//////////////////////////////////////////////////////////////
	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(AudioListener);

	//Registration in context
	void AudioListener::object_registration(Context& ctx)
	{
		//factory
		ctx.add_object<AudioListener>();
	}

	//////////////////////////////////////////////////////////////
	//Init
	AudioListener::AudioListener(Context& context) : Component(context)
	{
	}

	AudioListener::~AudioListener()
	{
	}

	//////////////////////////////////////////////////////////////
	//all events
	void AudioListener::on_attach(Actor& entity)
	{
		m_has_last = false;
	}

	void AudioListener::on_late_update(double delta_time)
	{
		auto* system = System::get<AudioSystem>(context());
		auto entity = actor().lock();
		if (!system || !entity)
		{
			return;
		}
		const Vec3 position = entity->position(true);
		Vec3 velocity(0.0f);
		if (m_has_last && delta_time > 0.0)
		{
			velocity = (position - m_last_position) / float(delta_time);
		}
		system->listener(position, entity->rotation(true), velocity);
		m_last_position = position;
		m_has_last = true;
	}

	//////////////////////////////////////////////////////////////
	//serialize
	void AudioListener::serialize(Data::Archive& archive)
	{
		Data::serialize(archive, this);
	}

	void AudioListener::serialize_json(Data::JsonValue& archive)
	{
		Data::serialize_json(archive, this);
	}

	//deserialize
	void AudioListener::deserialize(Data::Archive& archive)
	{
		Data::deserialize(archive, this);
	}

	void AudioListener::deserialize_json(Data::JsonValue& archive)
	{
		Data::deserialize_json(archive, this);
	}
}
}
