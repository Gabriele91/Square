//
//  AudioSystem.h
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/System/System.h"
#include "Square/Audio/Bus.h"

//miniaudio
struct ma_engine;
struct ma_sound;

namespace Square
{
	//Class Declaretion
	namespace Audio
	{
		class MusicPlayer;
		class Voice;
	}

	//The audio of the engine (miniaudio).
	//Sounds play in Audio::Voice and are mixed in the buses (Audio::Bus).
	//The listener is set by Scene::AudioListener, the music by Audio::MusicPlayer.
	//No audio device: the system does not start and the voices do not play.
	class SQUARE_API AudioSystem : public System
	{
	public:
		//A square system, and its ring
		SQUARE_SYSTEM(AudioSystem, SystemRing::DRIVER + 10)

		//Registration in context
		static void object_registration(Context& ctx);

		//Init
		AudioSystem(Context& context);
		virtual ~AudioSystem();

		//System
		virtual bool initialize() override;
		virtual void shutdown() override;
		virtual void update(double delta_time) override;

		//volumes (1 = normal)
		void  volume(float volume);
		float volume() const;
		void  volume(Audio::Bus bus, float volume);
		float volume(Audio::Bus bus) const;

		//pause all the sounds of a bus
		void pause(Audio::Bus bus, bool pause);
		bool paused(Audio::Bus bus) const;

		//listener (world space, looks along +z)
		void listener(const Vec3& position, const Quat& rotation, const Vec3& velocity);

		//music
		Audio::MusicPlayer& music();

		//miniaudio
		ma_engine* engine() const;
		ma_sound*  bus(Audio::Bus bus) const;

		//from the space of the engine (+z forward) to the one of miniaudio (-z forward)
		static Vec3 to_device(const Vec3& value);

		//the voices opened (by Audio::Voice), closed at shutdown
		void add(Audio::Voice* voice);
		void remove(Audio::Voice* voice);

	private:

		static constexpr size_t BUSES = size_t(Audio::Bus::COUNT);

		//init
		bool create_engine();
		void create_buses();
		//valid bus
		static bool valid(Audio::Bus bus);

		ma_engine*                    m_engine{ nullptr }; //mixer and device
		Array<ma_sound*, BUSES>       m_buses{};           //sound groups
		Array<float, BUSES>           m_volumes{};         //volume of each bus
		Array<bool, BUSES>            m_paused{};          //bus paused
		float                         m_volume{ 1.0f };    //master volume
		UniquePtr<Audio::MusicPlayer> m_music;             //songs
		Vector<Audio::Voice*>         m_voices;            //opened
	};
}
