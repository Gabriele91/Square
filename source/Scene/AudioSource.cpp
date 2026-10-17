//
//  AudioSource.cpp
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Object.h"
#include "Square/Core/Context.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Resource/Sound.h"
#include "Square/System/AudioSystem.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/AudioSource.h"

namespace Square
{
namespace Scene
{
	//////////////////////////////////////////////////////////////
	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(AudioSource);

	//Registration in context
	void AudioSource::object_registration(Context& ctx)
	{
		//factory
		ctx.add_object<AudioSource>();
		//Attributes
		ctx.add_attribute_function<AudioSource, String>
		("sound"
		, String()
		, [](const AudioSource* source) -> String     { return source->sound(); }
		, [](AudioSource* source, const String& sound){ source->sound(sound); });

		ctx.add_attribute_function<AudioSource, String>
		("bus"
		, String("effects")
		, [](const AudioSource* source) -> String   { return String(Audio::bus_to_string(source->bus())); }
		, [](AudioSource* source, const String& bus){ source->bus(Audio::bus_from_string(bus, Audio::Bus::EFFECTS)); });

		ctx.add_attribute_function<AudioSource, bool>
		("spatial"
		, true
		, [](const AudioSource* source) -> bool        { return source->spatial(); }
		, [](AudioSource* source, const bool& spatial) { source->spatial(spatial); });

		ctx.add_attribute_function<AudioSource, bool>
		("loop"
		, false
		, [](const AudioSource* source) -> bool     { return source->loop(); }
		, [](AudioSource* source, const bool& loop) { source->loop(loop); });

		ctx.add_attribute_function<AudioSource, bool>
		("autoplay"
		, false
		, [](const AudioSource* source) -> bool         { return source->autoplay(); }
		, [](AudioSource* source, const bool& autoplay) { source->autoplay(autoplay); });

		ctx.add_attribute_function<AudioSource, float>
		("volume"
		, 1.0f
		, [](const AudioSource* source) -> float       { return source->volume(); }
		, [](AudioSource* source, const float& volume) { source->volume(volume); });

		ctx.add_attribute_function<AudioSource, float>
		("pitch"
		, 1.0f
		, [](const AudioSource* source) -> float      { return source->pitch(); }
		, [](AudioSource* source, const float& pitch) { source->pitch(pitch); });

		ctx.add_attribute_function<AudioSource, float>
		("min_distance"
		, 2.0f
		, [](const AudioSource* source) -> float         { return source->min_distance(); }
		, [](AudioSource* source, const float& distance) { source->min_distance(distance); });

		ctx.add_attribute_function<AudioSource, float>
		("max_distance"
		, 150.0f
		, [](const AudioSource* source) -> float         { return source->max_distance(); }
		, [](AudioSource* source, const float& distance) { source->max_distance(distance); });

		ctx.add_attribute_function<AudioSource, float>
		("rolloff"
		, 1.0f
		, [](const AudioSource* source) -> float        { return source->rolloff(); }
		, [](AudioSource* source, const float& rolloff) { source->rolloff(rolloff); });

		ctx.add_attribute_function<AudioSource, float>
		("doppler"
		, 1.0f
		, [](const AudioSource* source) -> float        { return source->doppler(); }
		, [](AudioSource* source, const float& doppler) { source->doppler(doppler); });
	}

	//////////////////////////////////////////////////////////////
	//Init
	AudioSource::AudioSource(Context& context) : Component(context)
	{
	}

	AudioSource::~AudioSource()
	{
	}

	//////////////////////////////////////////////////////////////
	//set
	void AudioSource::settings(const Settings& settings)
	{
		m_settings = settings;
		m_changed = true;
	}

	void AudioSource::sound(const String& sound)
	{
		m_settings.m_sound = sound;
		m_changed = true;
	}

	void AudioSource::bus(Audio::Bus bus)
	{
		m_settings.m_bus = bus;
		m_changed = true;
	}

	void AudioSource::spatial(bool spatial)
	{
		m_settings.m_spatial = spatial;
		m_changed = true;
	}

	void AudioSource::loop(bool loop)
	{
		m_settings.m_loop = loop;
		m_voice.loop(loop);
	}

	void AudioSource::autoplay(bool autoplay)
	{
		m_settings.m_autoplay = autoplay;
	}

	void AudioSource::volume(float volume)
	{
		m_settings.m_volume = volume;
		m_voice.volume(volume);
	}

	void AudioSource::pitch(float pitch)
	{
		m_settings.m_pitch = pitch;
		m_voice.pitch(pitch);
	}

	void AudioSource::min_distance(float distance)
	{
		m_settings.m_min_distance = distance;
		m_voice.distances(m_settings.m_min_distance, m_settings.m_max_distance);
	}

	void AudioSource::max_distance(float distance)
	{
		m_settings.m_max_distance = distance;
		m_voice.distances(m_settings.m_min_distance, m_settings.m_max_distance);
	}

	void AudioSource::rolloff(float rolloff)
	{
		m_settings.m_rolloff = rolloff;
		m_voice.rolloff(rolloff);
	}

	void AudioSource::doppler(float doppler)
	{
		m_settings.m_doppler = doppler;
		m_voice.doppler(doppler);
	}

	//////////////////////////////////////////////////////////////
	//get
	const AudioSource::Settings& AudioSource::settings() const { return m_settings; }
	const String& AudioSource::sound() const { return m_settings.m_sound; }
	Audio::Bus AudioSource::bus() const      { return m_settings.m_bus; }
	bool  AudioSource::spatial() const       { return m_settings.m_spatial; }
	bool  AudioSource::loop() const          { return m_settings.m_loop; }
	bool  AudioSource::autoplay() const      { return m_settings.m_autoplay; }
	float AudioSource::volume() const        { return m_settings.m_volume; }
	float AudioSource::pitch() const         { return m_settings.m_pitch; }
	float AudioSource::min_distance() const  { return m_settings.m_min_distance; }
	float AudioSource::max_distance() const  { return m_settings.m_max_distance; }
	float AudioSource::rolloff() const       { return m_settings.m_rolloff; }
	float AudioSource::doppler() const       { return m_settings.m_doppler; }

	//////////////////////////////////////////////////////////////
	//play
	void AudioSource::play()
	{
		refresh();
		m_voice.rewind();
		m_voice.play();
	}

	void AudioSource::stop()
	{
		m_voice.stop();
	}

	void AudioSource::stop(float fade_seconds)
	{
		m_voice.stop(fade_seconds);
	}

	bool AudioSource::playing() const
	{
		return m_voice.playing();
	}

	//////////////////////////////////////////////////////////////
	//voice
	void AudioSource::refresh()
	{
		if (!m_changed)
		{
			return;
		}
		m_changed = false;
		if (!open())
		{
			return;
		}
		if (m_settings.m_autoplay)
		{
			m_voice.play();
		}
	}

	bool AudioSource::open()
	{
		m_voice.close();
		auto* system = System::get<AudioSystem>(context());
		if (!system || m_settings.m_sound.empty())
		{
			return false;
		}
		auto sound = context().resource<Resource::Sound>(m_settings.m_sound);
		if (!m_voice.open(*system, sound, m_settings.m_bus, m_settings.m_spatial))
		{
			context().logger()->warning("AudioSource: cannot play " + m_settings.m_sound);
			return false;
		}
		apply();
		m_has_last = false;
		follow(0.0f);
		return true;
	}

	void AudioSource::apply()
	{
		m_voice.loop(m_settings.m_loop);
		m_voice.volume(m_settings.m_volume);
		m_voice.pitch(m_settings.m_pitch);
		if (m_settings.m_spatial)
		{
			m_voice.distances(m_settings.m_min_distance, m_settings.m_max_distance);
			m_voice.rolloff(m_settings.m_rolloff);
			m_voice.doppler(m_settings.m_doppler);
		}
	}

	void AudioSource::follow(float delta_time)
	{
		auto entity = actor().lock();
		if (!entity || !m_settings.m_spatial || !m_voice.opened())
		{
			return;
		}
		const Vec3 position = entity->position(true);
		Vec3 velocity(0.0f);
		if (m_has_last && delta_time > 0.0f)
		{
			velocity = (position - m_last_position) / delta_time;
		}
		m_voice.position(position);
		m_voice.velocity(velocity);
		m_last_position = position;
		m_has_last = true;
	}

	//////////////////////////////////////////////////////////////
	//all events
	void AudioSource::on_attach(Actor& entity)
	{
		m_has_last = false;
	}

	void AudioSource::on_deattch()
	{
		m_voice.close();
		m_changed = true;
	}

	void AudioSource::on_late_update(double delta_time)
	{
		refresh();
		follow(float(delta_time));
	}

	//////////////////////////////////////////////////////////////
	//serialize
	void AudioSource::serialize(Data::Archive& archive)
	{
		Data::serialize(archive, this);
	}

	void AudioSource::serialize_json(Data::JsonValue& archive)
	{
		Data::serialize_json(archive, this);
	}

	//deserialize
	void AudioSource::deserialize(Data::Archive& archive)
	{
		Data::deserialize(archive, this);
	}

	void AudioSource::deserialize_json(Data::JsonValue& archive)
	{
		Data::deserialize_json(archive, this);
	}
}
}
