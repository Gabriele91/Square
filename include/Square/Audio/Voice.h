//
//  Voice.h
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Audio/Bus.h"

//miniaudio
struct ma_decoder;
struct ma_sound;

namespace Square
{
	//Class Declaretion
	class AudioSystem;
	namespace Resource
	{
		class Sound;
	}

namespace Audio
{
	//A sound that plays in a bus.
	//Open it once (e.g. at the load of a level), then play it as many times as you want.
	class SQUARE_API Voice : public Uncopyable
	{
	public:

		//Init
		Voice();
		~Voice();

		//open/close (false: no audio device or a file that cannot be decoded)
		bool open(AudioSystem& system, const Shared<Resource::Sound>& sound, Bus bus, bool spatial);
		void close();
		bool opened() const;

		//play
		void play();
		void stop();
		void stop(float fade_seconds);
		void rewind();
		bool playing() const;

		//set
		void loop(bool loop);
		void volume(float volume);
		void pitch(float pitch);                      //1 = normal, 2 = one octave up
		void fade(float from, float to, float seconds); //from < 0 = the volume now

		//3D (world space, units per second)
		void position(const Vec3& position);
		void velocity(const Vec3& velocity);
		void distances(float min, float max);         //full volume under min, no attenuation over max
		void rolloff(float rolloff);
		void doppler(float doppler);                  //0 = no doppler

	private:

		AudioSystem*            m_system{ nullptr };  //opened in it
		Shared<Resource::Sound> m_sound;              //the bytes read by the decoder
		ma_decoder*             m_decoder{ nullptr }; //decoder of the sound
		ma_sound*               m_handle{ nullptr };  //sound of miniaudio
	};
}
}
