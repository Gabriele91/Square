//
//  MusicPlayer.cpp
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Resource/Sound.h"
#include "Square/System/AudioSystem.h"
#include "Square/Audio/MusicPlayer.h"

namespace Square
{
namespace Audio
{
	//////////////////////////////////////////////////////////////
	//Init
	MusicPlayer::MusicPlayer(AudioSystem& system)
	: m_system(system)
	{
	}

	MusicPlayer::~MusicPlayer()
	{
	}

	//////////////////////////////////////////////////////////////
	//play
	bool MusicPlayer::play(const String& name, float fade_seconds, bool loop)
	{
		//already playing
		if (name == m_song && m_decks[m_deck].playing())
		{
			return true;
		}
		//get song
		auto sound = m_system.context().resource<Resource::Sound>(name);
		if (!sound)
		{
			m_system.context().logger()->warning("MusicPlayer: song " + name + " not found");
			return false;
		}
		//fade out the old song
		stop(fade_seconds);
		//fade in the new one
		m_deck = next_deck();
		Voice& deck = m_decks[m_deck];
		if (!deck.open(m_system, sound, Bus::MUSIC, false))
		{
			return false;
		}
		deck.loop(loop);
		deck.fade(0.0f, 1.0f, fade_seconds);
		deck.play();
		m_song = name;
		return true;
	}

	void MusicPlayer::stop(float fade_seconds)
	{
		Voice& deck = m_decks[m_deck];
		if (deck.playing())
		{
			deck.stop(fade_seconds);
		}
		m_song.clear();
	}

	//////////////////////////////////////////////////////////////
	//get
	const String& MusicPlayer::song() const
	{
		return m_song;
	}

	//////////////////////////////////////////////////////////////
	//update
	void MusicPlayer::update()
	{
		//close the decks at the end of their fade out
		for (size_t deck = 0; deck < DECKS; ++deck)
		{
			const bool current = deck == m_deck && !m_song.empty();
			if (!current && m_decks[deck].opened() && !m_decks[deck].playing())
			{
				m_decks[deck].close();
			}
		}
	}

	size_t MusicPlayer::next_deck() const
	{
		return (m_deck + 1) % DECKS;
	}
}
}
