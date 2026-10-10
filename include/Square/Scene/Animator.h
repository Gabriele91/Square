//
//  Animator.h
//  Square
//
//  The animations of the actors under its actor (glTF animations): its clips, each one some
//  channels, a channel the position, the rotation or the scale of an actor (its path from the
//  actor of the animator: "arm/hand", "" itself) by its keys (their times in seconds, their values;
//  step, linear or cubic between them). It plays some clips at once (layers: a channel of a clip
//  played later wins), each one at its time, once or in loop, at its speed. Every update of its
//  actor the actors animated move (they are not static: the converter makes them so). Autoplay:
//  all its clips in loop when it starts (the ambient animations of a map). Saved with its scene:
//  the converter makes it at the root of a model with animations.
//
#pragma once
#include <string>
#include <vector>
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"

namespace Square
{
namespace Scene
{
	class SQUARE_API Animator : public Square::Scene::Component
	{
	public:
		SQUARE_OBJECT(Animator)

		//what a channel moves
		enum Property : int
		{
			P_POSITION = 0,
			P_ROTATION = 1, //(its values: quaternions, x y z w)
			P_SCALE    = 2
		};

		//between two keys
		enum Interpolation : int
		{
			I_STEP   = 0,
			I_LINEAR = 1,
			I_CUBIC  = 2 //(3 values a key: its tangent in, its value, its tangent out)
		};

		struct Channel
		{
			std::string                m_target;   //the path of the actor from the animator ("" itself)
			Property                   m_property{ P_POSITION };
			Interpolation              m_interpolation{ I_LINEAR };
			std::vector< float >       m_times;    //seconds, growing
			std::vector< Square::Vec4 > m_values;  //a key (3 when cubic)
		};

		struct Clip
		{
			std::string            m_name;
			float                  m_duration{ 0.0f }; //seconds (the last key of its channels)
			std::vector< Channel > m_channels;
		};

		Animator(Square::Context& context);
		virtual ~Animator();

		//its clips (their actors found again)
		void clips(const std::vector< Clip >& clips);
		const std::vector< Clip >& clips() const { return m_clips; }
		//the index of a clip by its name (-1: none)
		int clip_id(const std::string& name) const;

		//play a clip from its start (again if playing), once or in loop; its speed (1: its time)
		bool play(const std::string& name, bool loop = true, float speed = 1.0f);
		//all of them in loop
		void play_all(bool loop = true);
		//stop a clip (its actors stay where they are), all of them
		void stop(const std::string& name);
		void stop_all();
		//a clip playing (false: stopped, or once and ended)
		bool playing(const std::string& name) const;
		//the time of a clip playing (seconds), to set it (a jump)
		float time(const std::string& name) const;
		void time(const std::string& name, float time);

		//the speed of all of them (1: their time; 0 paused)
		void speed(float speed) { m_speed = speed; }
		float speed() const { return m_speed; }
		//all the clips in loop when it starts
		void autoplay(bool autoplay) { m_autoplay = autoplay; }
		bool autoplay() const { return m_autoplay; }

		//the clips played: their time on, their actors moved
		virtual void on_update(double delta_time) override;

		//events
		virtual void on_attach(Square::Scene::Actor& entity) override;
		virtual void on_deattch() override;

		//regs
		static void object_registration(Square::Context& ctx);

		//serialize
		virtual void serialize(Square::Data::Archive& archive)  override;
		virtual void serialize_json(Square::Data::JsonValue& archive) override;
		virtual void deserialize(Square::Data::Archive& archive) override;
		virtual void deserialize_json(Square::Data::JsonValue& archive) override;

	private:

		//a clip played: its time (seconds), once or in loop, its speed
		struct Track
		{
			size_t m_clip{ 0 };
			float  m_time{ 0.0f };
			float  m_speed{ 1.0f };
			bool   m_loop{ true };
			bool   m_ended{ false };
		};

		//the clips flat (the attributes of the scene file)
		struct Packed
		{
			std::vector< std::string >  m_clip_names;
			std::vector< float >        m_clip_durations;
			std::vector< int >          m_clip_channels;   //the channels of each clip
			std::vector< std::string >  m_channel_targets;
			std::vector< int >          m_channel_kinds;   //property * 4 + interpolation
			std::vector< int >          m_channel_keys;    //the keys of each channel
			std::vector< float >        m_key_times;
			std::vector< Square::Vec4 > m_key_values;
		};
		Packed pack() const;
		void unpack(const Packed& packed);
		//the attribute of a part of the packed clips (read: packed now; set: kept, the clips made
		//again when the animator is deserialized)
		template < typename T, T Packed::* Field >
		static void packed_attribute(Square::Context& ctx, const char* name);

		//the actors of the channels of the clips (by their paths)
		void find_targets();
		//a clip at a time: its actors moved
		void apply(size_t clip, float time);
		//the track of a clip (nullptr: not played)
		Track* track(size_t clip);
		const Track* track(size_t clip) const;

		std::vector< Clip >                                m_clips;
		std::vector< std::vector< Square::Weak<Actor> > >  m_targets; //by clip, by channel
		std::vector< Track >                               m_tracks;
		Packed                                             m_packed;  //(while deserialized)
		float                                              m_speed{ 1.0f };
		bool                                               m_autoplay{ false };
		bool                                               m_started{ false };
		bool                                               m_targets_found{ false };
	};
}
}
