//
//  Sound.h
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Resource.h"
#include "Square/Core/SmartPointers.h"

namespace Square
{
namespace Resource
{
	//A sound file (.wav, .ogg, .mp3, .flac) kept in memory as it is.
	//It is decoded while it plays (every Audio::Voice has its own decoder).
	class SQUARE_API Sound : public ResourceObject
	                       , public InheritableSharedObject<Sound>
	{
	public:
		//Init object
		SQUARE_OBJECT(Sound)

		//Registration in context
		static void object_registration(Context& ctx);

		//Contructor
		Sound(Context& context);
		Sound(Context& context, const String& path);

		//load
		bool load(const String& path) override;

		//get
		const Vector<unsigned char>& data() const;

	private:

		Vector<unsigned char> m_data; //the file
	};
}
}
