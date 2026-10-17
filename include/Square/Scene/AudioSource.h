//
//  AudioSource.h
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"
#include "Square/Audio/Bus.h"
#include "Square/Audio/Voice.h"

namespace Square
{
namespace Scene
{
	//A sound of an actor.
	//Spatial: it follows the actor (position and velocity), else it is heard as it is.
	//The voice is opened again only when the sound, the bus or spatial change.
	class SQUARE_API AudioSource : public Component
	{
	public:
		//A square object
		SQUARE_OBJECT(AudioSource)

		//all the values (distances in world units)
		struct Settings
		{
			String     m_sound;                     //name of a Resource::Sound
			Audio::Bus m_bus{ Audio::Bus::EFFECTS };
			bool       m_spatial{ true };
			bool       m_loop{ false };
			bool       m_autoplay{ false };         //play as soon as it is opened
			float      m_volume{ 1.0f };
			float      m_pitch{ 1.0f };
			float      m_min_distance{ 2.0f };      //full volume under it
			float      m_max_distance{ 150.0f };    //no attenuation over it
			float      m_rolloff{ 1.0f };
			float      m_doppler{ 1.0f };
		};

		//Registration in context
		static void object_registration(Context& ctx);

		//Init
		AudioSource(Context& context);
		virtual ~AudioSource();

		//set
		void settings(const Settings& settings);
		void sound(const String& sound);
		void bus(Audio::Bus bus);
		void spatial(bool spatial);
		void loop(bool loop);
		void autoplay(bool autoplay);
		void volume(float volume);
		void pitch(float pitch);
		void min_distance(float distance);
		void max_distance(float distance);
		void rolloff(float rolloff);
		void doppler(float doppler);

		//get
		const Settings& settings() const;
		const String& sound() const;
		Audio::Bus bus() const;
		bool spatial() const;
		bool loop() const;
		bool autoplay() const;
		float volume() const;
		float pitch() const;
		float min_distance() const;
		float max_distance() const;
		float rolloff() const;
		float doppler() const;

		//play
		void play();
		void stop();
		void stop(float fade_seconds);
		bool playing() const;

		//all events
		virtual void on_attach(Actor& entity) override;
		virtual void on_deattch() override;
		virtual void on_late_update(double delta_time) override;

		//serialize
		virtual void serialize(Data::Archive& archive) override;
		virtual void serialize_json(Data::JsonValue& archive) override;
		//deserialize
		virtual void deserialize(Data::Archive& archive) override;
		virtual void deserialize_json(Data::JsonValue& archive) override;

	private:

		//open the voice if the sound changed
		void refresh();
		bool open();
		//set the values of the voice
		void apply();
		//put the voice where the actor is
		void follow(float delta_time);

		Settings     m_settings;               //values
		Audio::Voice m_voice;                  //sound
		bool         m_changed{ false };       //voice to open again
		Vec3         m_last_position{ 0.0f };  //for the velocity
		bool         m_has_last{ false };
	};
}
}
