//
//  Voice.cpp
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include <algorithm>
#include "Square/Config.h"
#include "Square/Resource/Sound.h"
#include "Square/System/AudioSystem.h"
#include "Square/Audio/Voice.h"
#include "miniaudio.h"

namespace Square
{
namespace Audio
{
namespace AuxVoice
{
	//constants
	static constexpr float s_milliseconds = 1000.0f; //in a second

	static ma_uint64 milliseconds(float seconds)
	{
		return ma_uint64(std::max(seconds, 0.0f) * s_milliseconds);
	}
}
	//////////////////////////////////////////////////////////////
	//Init
	Voice::Voice()
	{
	}

	Voice::~Voice()
	{
		close();
	}

	//////////////////////////////////////////////////////////////
	//open/close
	bool Voice::open(AudioSystem& system, const Shared<Resource::Sound>& sound, Bus bus, bool spatial)
	{
		close();
		if (!system.engine() || !sound || sound->data().empty())
		{
			return false;
		}
		//decoder (same memory of the engine)
		ma_decoder_config config = ma_decoder_config_init_default();
		config.allocationCallbacks = system.engine()->allocationCallbacks;
		m_decoder = new ma_decoder();
		if (ma_decoder_init_memory(sound->data().data(), sound->data().size(), &config, m_decoder) != MA_SUCCESS)
		{
			delete m_decoder;
			m_decoder = nullptr;
			return false;
		}
		//sound
		const ma_uint32 flags = spatial ? 0 : MA_SOUND_FLAG_NO_SPATIALIZATION;
		m_handle = new ma_sound();
		if (ma_sound_init_from_data_source(system.engine(), m_decoder, flags, system.bus(bus), m_handle) != MA_SUCCESS)
		{
			delete m_handle;
			m_handle = nullptr;
			close();
			return false;
		}
		m_sound = sound;
		m_system = &system;
		m_system->add(this);
		return true;
	}

	void Voice::close()
	{
		if (m_handle)
		{
			ma_sound_uninit(m_handle);
			delete m_handle;
			m_handle = nullptr;
		}
		if (m_decoder)
		{
			ma_decoder_uninit(m_decoder);
			delete m_decoder;
			m_decoder = nullptr;
		}
		m_sound.reset();
		if (m_system)
		{
			m_system->remove(this);
			m_system = nullptr;
		}
	}

	bool Voice::opened() const
	{
		return m_handle != nullptr;
	}

	//////////////////////////////////////////////////////////////
	//play
	void Voice::play()
	{
		if (!m_handle)
		{
			return;
		}
		//remove the stop of a previous fade out
		ma_sound_reset_stop_time_and_fade(m_handle);
		ma_sound_start(m_handle);
	}

	void Voice::stop()
	{
		if (m_handle)
		{
			ma_sound_stop(m_handle);
		}
	}

	void Voice::stop(float fade_seconds)
	{
		if (m_handle)
		{
			ma_sound_stop_with_fade_in_milliseconds(m_handle, AuxVoice::milliseconds(fade_seconds));
		}
	}

	void Voice::rewind()
	{
		if (m_handle)
		{
			ma_sound_seek_to_pcm_frame(m_handle, 0);
		}
	}

	bool Voice::playing() const
	{
		return m_handle && ma_sound_is_playing(m_handle);
	}

	//////////////////////////////////////////////////////////////
	//set
	void Voice::loop(bool loop)
	{
		if (m_handle)
		{
			ma_sound_set_looping(m_handle, loop ? MA_TRUE : MA_FALSE);
		}
	}

	void Voice::volume(float volume)
	{
		if (m_handle)
		{
			ma_sound_set_volume(m_handle, volume);
		}
	}

	void Voice::pitch(float pitch)
	{
		if (m_handle)
		{
			ma_sound_set_pitch(m_handle, pitch);
		}
	}

	void Voice::fade(float from, float to, float seconds)
	{
		if (m_handle)
		{
			ma_sound_set_fade_in_milliseconds(m_handle, from, to, AuxVoice::milliseconds(seconds));
		}
	}

	//////////////////////////////////////////////////////////////
	//3D
	void Voice::position(const Vec3& position)
	{
		if (m_handle)
		{
			const Vec3 point = AudioSystem::to_device(position);
			ma_sound_set_position(m_handle, point.x, point.y, point.z);
		}
	}

	void Voice::velocity(const Vec3& velocity)
	{
		if (m_handle)
		{
			const Vec3 speed = AudioSystem::to_device(velocity);
			ma_sound_set_velocity(m_handle, speed.x, speed.y, speed.z);
		}
	}

	void Voice::distances(float min, float max)
	{
		if (m_handle)
		{
			ma_sound_set_min_distance(m_handle, min);
			ma_sound_set_max_distance(m_handle, max);
		}
	}

	void Voice::rolloff(float rolloff)
	{
		if (m_handle)
		{
			ma_sound_set_rolloff(m_handle, rolloff);
		}
	}

	void Voice::doppler(float doppler)
	{
		if (m_handle)
		{
			ma_sound_set_doppler_factor(m_handle, doppler);
		}
	}
}
}
