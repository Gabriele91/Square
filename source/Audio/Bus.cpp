//
//  Bus.cpp
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Audio/Bus.h"

namespace Square
{
namespace Audio
{
namespace AuxBus
{
	//names of the buses, in the order of the enum (the bus is the index)
	static constexpr StringView s_names[] =
	{
		"music",     //MUSIC
		"effects",   //EFFECTS
		"engines",   //ENGINES
		"interface"  //INTERFACE
	};
	static_assert(sizeof(s_names) / sizeof(s_names[0]) == size_t(Bus::COUNT), "a name for every bus");
}
	//////////////////////////////////////////////////////////////
	//serialize
	StringView bus_to_string(Bus bus)
	{
		const size_t id = size_t(bus);
		if (id >= size_t(Bus::COUNT))
		{
			return StringView();
		}
		return AuxBus::s_names[id];
	}

	Bus bus_from_string(StringView bus, Bus bus_default)
	{
		for (size_t id = 0; id < size_t(Bus::COUNT); ++id)
		{
			if (bus == AuxBus::s_names[id])
			{
				return Bus(id);
			}
		}
		return bus_default;
	}
}
}
