//
//  Bus.h
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"

namespace Square
{
namespace Audio
{
	//Every sound plays in a bus, every bus has its own volume (AudioSystem::volume)
	enum class Bus : unsigned char
	{
		MUSIC = 0, //songs (MusicPlayer)
		EFFECTS,   //sounds of the world
		ENGINES,   //engine loops
		INTERFACE, //menus
		COUNT
	};

	//serialize: "music", "effects", "engines", "interface"
	SQUARE_API StringView bus_to_string(Bus bus);
	SQUARE_API Bus bus_from_string(StringView bus, Bus bus_default);
}
}
