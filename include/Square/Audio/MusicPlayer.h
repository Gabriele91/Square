//
//  MusicPlayer.h
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Audio/Voice.h"

namespace Square
{
	//Class Declaretion
	class AudioSystem;

namespace Audio
{
	//One song at time on the bus MUSIC.
	//A new song cross fades with the old one (two decks, used in turn).
	class SQUARE_API MusicPlayer : public Uncopyable
	{
	public:

		//Init
		MusicPlayer(AudioSystem& system);
		~MusicPlayer();

		//play a song (a Resource::Sound), if it is already playing nothing changes
		bool play(const String& name, float fade_seconds, bool loop = true);
		void stop(float fade_seconds);

		//get the name of the song ("" = none)
		const String& song() const;

		//called by the AudioSystem every frame
		void update();

	private:

		static constexpr size_t DECKS = 2;

		//the deck that is not playing
		size_t next_deck() const;

		AudioSystem&       m_system;    //owner
		Array<Voice, DECKS> m_decks;    //songs
		size_t             m_deck{ 0 }; //deck of the current song
		String             m_song;      //name of the current song
	};
}
}
