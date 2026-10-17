//
//  AudioSystem.cpp
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include <algorithm>
#include "Square/Core/Context.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Core/Allocator/Memory.h"
#include "Square/System/AudioSystem.h"
#include "Square/Audio/MusicPlayer.h"
#include "Square/Audio/Voice.h"
#include "miniaudio.h"

namespace Square
{
namespace AuxAudioSystem
{
	//memory of miniaudio: the heap of the engine
	static void* allocate(size_t size, void*)
	{
		return Memory::allocate(size, Allocator::default_align);
	}

	static void* reallocate(void* ptr, size_t size, void*)
	{
		return Memory::reallocate(ptr, size);
	}

	static void deallocate(void* ptr, void*)
	{
		Memory::deallocate(ptr);
	}
}
	//////////////////////////////////////////////////////////////
	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(AudioSystem);

	//Registration in context
	void AudioSystem::object_registration(Context& ctx)
	{
		//system
		ctx.add_system<AudioSystem>(SystemStartup::AUTOMATIC);
	}

	//////////////////////////////////////////////////////////////
	//Init
	AudioSystem::AudioSystem(Context& context) : System(context)
	{
		m_volumes.fill(1.0f);
		m_paused.fill(false);
	}

	AudioSystem::~AudioSystem()
	{
		shutdown();
	}

	//////////////////////////////////////////////////////////////
	//System
	bool AudioSystem::initialize()
	{
		if (!create_engine())
		{
			context().logger()->warning("AudioSystem: no audio device");
			return false;
		}
		create_buses();
		m_music = MakeUniquePtr<Audio::MusicPlayer>(*this);
		return true;
	}

	void AudioSystem::shutdown()
	{
		//the music first (its voices are in a bus)
		m_music.reset();
		//the voices still opened
		while (!m_voices.empty())
		{
			m_voices.back()->close();
		}
		//buses
		for (ma_sound*& bus : m_buses)
		{
			if (bus)
			{
				ma_sound_group_uninit(bus);
				delete bus;
				bus = nullptr;
			}
		}
		//engine
		if (m_engine)
		{
			ma_engine_uninit(m_engine);
			delete m_engine;
			m_engine = nullptr;
		}
	}

	void AudioSystem::update(double delta_time)
	{
		if (m_music)
		{
			m_music->update();
		}
	}

	bool AudioSystem::create_engine()
	{
		ma_engine_config config = ma_engine_config_init();
		config.allocationCallbacks.onMalloc = &AuxAudioSystem::allocate;
		config.allocationCallbacks.onRealloc = &AuxAudioSystem::reallocate;
		config.allocationCallbacks.onFree = &AuxAudioSystem::deallocate;
		m_engine = new ma_engine();
		if (ma_engine_init(&config, m_engine) != MA_SUCCESS)
		{
			delete m_engine;
			m_engine = nullptr;
			return false;
		}
		ma_engine_set_volume(m_engine, m_volume);
		return true;
	}

	void AudioSystem::create_buses()
	{
		for (size_t id = 0; id < BUSES; ++id)
		{
			m_buses[id] = new ma_sound();
			if (ma_sound_group_init(m_engine, 0, nullptr, m_buses[id]) != MA_SUCCESS)
			{
				delete m_buses[id];
				m_buses[id] = nullptr;
				continue;
			}
			ma_sound_group_set_volume(m_buses[id], m_volumes[id]);
		}
	}

	bool AudioSystem::valid(Audio::Bus bus)
	{
		return size_t(bus) < BUSES;
	}

	//////////////////////////////////////////////////////////////
	//volumes
	void AudioSystem::volume(float volume)
	{
		m_volume = std::max(volume, 0.0f);
		if (m_engine)
		{
			ma_engine_set_volume(m_engine, m_volume);
		}
	}

	float AudioSystem::volume() const
	{
		return m_volume;
	}

	void AudioSystem::volume(Audio::Bus bus, float volume)
	{
		if (!valid(bus))
		{
			return;
		}
		const size_t id = size_t(bus);
		m_volumes[id] = std::max(volume, 0.0f);
		if (m_buses[id])
		{
			ma_sound_group_set_volume(m_buses[id], m_volumes[id]);
		}
	}

	float AudioSystem::volume(Audio::Bus bus) const
	{
		if (!valid(bus))
		{
			return 0.0f;
		}
		return m_volumes[size_t(bus)];
	}

	//////////////////////////////////////////////////////////////
	//pause
	void AudioSystem::pause(Audio::Bus bus, bool pause)
	{
		if (!valid(bus))
		{
			return;
		}
		const size_t id = size_t(bus);
		m_paused[id] = pause;
		if (!m_buses[id])
		{
			return;
		}
		//a stopped group does not read its sounds: they stay where they are
		if (pause)
		{
			ma_sound_group_stop(m_buses[id]);
		}
		else
		{
			ma_sound_group_start(m_buses[id]);
		}
	}

	bool AudioSystem::paused(Audio::Bus bus) const
	{
		if (!valid(bus))
		{
			return false;
		}
		return m_paused[size_t(bus)];
	}

	//////////////////////////////////////////////////////////////
	//listener
	void AudioSystem::listener(const Vec3& position, const Quat& rotation, const Vec3& velocity)
	{
		if (!m_engine)
		{
			return;
		}
		const Vec3 point = to_device(position);
		const Vec3 forward = to_device(rotation * Constants::axis_z);
		const Vec3 up = to_device(rotation * Constants::axis_y);
		const Vec3 speed = to_device(velocity);
		ma_engine_listener_set_position(m_engine, 0, point.x, point.y, point.z);
		ma_engine_listener_set_direction(m_engine, 0, forward.x, forward.y, forward.z);
		ma_engine_listener_set_world_up(m_engine, 0, up.x, up.y, up.z);
		ma_engine_listener_set_velocity(m_engine, 0, speed.x, speed.y, speed.z);
	}

	//////////////////////////////////////////////////////////////
	//get
	Audio::MusicPlayer& AudioSystem::music()
	{
		return *m_music;
	}

	ma_engine* AudioSystem::engine() const
	{
		return m_engine;
	}

	ma_sound* AudioSystem::bus(Audio::Bus bus) const
	{
		if (!valid(bus))
		{
			return nullptr;
		}
		return m_buses[size_t(bus)];
	}

	Vec3 AudioSystem::to_device(const Vec3& value)
	{
		return Vec3(value.x, value.y, -value.z);
	}

	//////////////////////////////////////////////////////////////
	//voices
	void AudioSystem::add(Audio::Voice* voice)
	{
		m_voices.push_back(voice);
	}

	void AudioSystem::remove(Audio::Voice* voice)
	{
		m_voices.erase(std::remove(m_voices.begin(), m_voices.end(), voice), m_voices.end());
	}
}
