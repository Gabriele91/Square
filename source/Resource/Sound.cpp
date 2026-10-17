//
//  Sound.cpp
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Resource/Sound.h"

namespace Square
{
namespace Resource
{
	//////////////////////////////////////////////////////////////
	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(Sound);

	//Registration in context
	void Sound::object_registration(Context& ctx)
	{
		//factory
		ctx.add_resource<Sound>({ ".wav", ".ogg", ".mp3", ".flac" });
	}

	//////////////////////////////////////////////////////////////
	//Init
	Sound::Sound(Context& context)
	: ResourceObject(context)
	, BaseInheritableSharedObject(context.allocator())
	{
	}

	Sound::Sound(Context& context, const String& path)
	: ResourceObject(context)
	, BaseInheritableSharedObject(context.allocator())
	{
		load(path);
	}

	//////////////////////////////////////////////////////////////
	//load
	bool Sound::load(const String& path)
	{
		//also from an archive (.sqz)
		m_data = Filesystem::binary_file_read_all(path);
		if (m_data.empty())
		{
			ResourceObject::context().logger()->warning("Sound: cannot read " + path);
			return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////
	//get
	const Vector<unsigned char>& Sound::data() const
	{
		return m_data;
	}
}
}
