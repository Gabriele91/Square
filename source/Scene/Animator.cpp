//
//  Animator.cpp
//  Square
//
//  See Animator.h for the high level description.
//
#include <algorithm>
#include <cmath>
#include "Square/Config.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/Animator.h"
#include "Square/Core/ClassObjectRegistration.h"

namespace Square
{
namespace Scene
{
	namespace AuxAnimator
	{
		static Quat to_quat(const Vec4& value)
		{
			return Quat(value.x, value.y, value.z, value.w);
		}

		//the key before a time (the last one at or before it) and the share of the way to the
		//next one; one key, or before the first: the first, after the last: the last
		static size_t key_at(const std::vector<float>& times, float time, float& share, float& span)
		{
			share = 0.0f;
			span = 0.0f;
			size_t key = 0;
			const bool between = times.size() > 1 && time > times.front();
			if (between && time >= times.back())
			{
				key = times.size() - 1;
			}
			else if (between)
			{
				key = size_t(std::upper_bound(times.begin(), times.end(), time) - times.begin()) - 1;
				span = times[key + 1] - times[key];
				if (span > 0.0f)
				{
					share = (time - times[key]) / span;
				}
			}
			return key;
		}

		//the value of a channel at a time
		static Vec4 sample(const Animator::Channel& channel, float time)
		{
			Vec4 value(0.0f);
			const size_t keys = channel.m_times.size();
			const size_t a_value = channel.m_interpolation == Animator::I_CUBIC ? 3 : 1;
			if (keys && channel.m_values.size() >= keys * a_value)
			{
				float share = 0.0f;
				float span = 0.0f;
				const size_t key = key_at(channel.m_times, time, share, span);
				const size_t next = std::min(key + 1, keys - 1);
				const bool rotation = channel.m_property == Animator::P_ROTATION;
				switch (channel.m_interpolation)
				{
				case Animator::I_STEP:
				{
					value = channel.m_values[key];
				}
				break;
				case Animator::I_CUBIC:
				{
					//Hermite: the values of the keys, their tangents (out of this one, in of the next)
					const Vec4& from = channel.m_values[key * 3 + 1];
					const Vec4& out = channel.m_values[key * 3 + 2];
					const Vec4& in = channel.m_values[next * 3 + 0];
					const Vec4& to = channel.m_values[next * 3 + 1];
					const float t = share;
					const float t2 = t * t;
					const float t3 = t2 * t;
					value = from * (2.0f * t3 - 3.0f * t2 + 1.0f)
					      + out * (span * (t3 - 2.0f * t2 + t))
					      + to * (-2.0f * t3 + 3.0f * t2)
					      + in * (span * (t3 - t2));
					if (rotation)
					{
						value = normalize(value);
					}
				}
				break;
				default:
				case Animator::I_LINEAR:
				{
					const Vec4& from = channel.m_values[key];
					const Vec4& to = channel.m_values[next];
					if (rotation)
					{
						const Quat turn = glm::slerp(to_quat(from), to_quat(to), share);
						value = Vec4(turn.x, turn.y, turn.z, turn.w);
					}
					else
					{
						value = from + (to - from) * share;
					}
				}
				break;
				}
			}
			return value;
		}
	}

	SQUARE_CLASS_OBJECT_REGISTRATION(Animator);

	template < typename T, T Animator::Packed::* Field >
	void Animator::packed_attribute(Square::Context& ctx, const char* name)
	{
		ctx.add_attribute_function<Animator, T>
			(name
			, T()
			, [](const Animator* animator) -> T { return animator->pack().*Field; }
			, [](Animator* animator, const T& value) { animator->m_packed.*Field = value; });
	}

	//regs
	void Animator::object_registration(Square::Context& ctx)
	{
		ctx.add_object<Animator>();
		packed_attribute< std::vector<std::string>, &Packed::m_clip_names >(ctx, "clip_names");
		packed_attribute< std::vector<float>, &Packed::m_clip_durations >(ctx, "clip_durations");
		packed_attribute< std::vector<int>, &Packed::m_clip_channels >(ctx, "clip_channels");
		packed_attribute< std::vector<std::string>, &Packed::m_channel_targets >(ctx, "channel_targets");
		packed_attribute< std::vector<int>, &Packed::m_channel_kinds >(ctx, "channel_kinds");
		packed_attribute< std::vector<int>, &Packed::m_channel_keys >(ctx, "channel_keys");
		packed_attribute< std::vector<float>, &Packed::m_key_times >(ctx, "key_times");
		packed_attribute< std::vector<Vec4>, &Packed::m_key_values >(ctx, "key_values");
		ctx.add_attribute_function<Animator, float>
			("speed"
			, 1.0f
			, [](const Animator* animator) -> float { return animator->speed(); }
			, [](Animator* animator, const float& speed) { animator->speed(speed); });
		ctx.add_attribute_function<Animator, bool>
			("autoplay"
			, false
			, [](const Animator* animator) -> bool { return animator->autoplay(); }
			, [](Animator* animator, const bool& autoplay) { animator->autoplay(autoplay); });
	}

	Animator::Animator(Square::Context& context)
	: Component(context)
	{
	}

	Animator::~Animator()
	{
	}

	void Animator::clips(const std::vector<Clip>& clips)
	{
		m_clips = clips;
		m_tracks.clear();
		m_targets_found = false;
	}

	int Animator::clip_id(const std::string& name) const
	{
		int id = -1;
		for (size_t i = 0; i < m_clips.size() && id < 0; ++i)
		{
			if (m_clips[i].m_name == name)
			{
				id = int(i);
			}
		}
		return id;
	}

	Animator::Track* Animator::track(size_t clip)
	{
		Track* found = nullptr;
		for (auto& track : m_tracks)
		{
			if (track.m_clip == clip)
			{
				found = &track;
			}
		}
		return found;
	}

	const Animator::Track* Animator::track(size_t clip) const
	{
		return const_cast<Animator*>(this)->track(clip);
	}

	bool Animator::play(const std::string& name, bool loop, float speed)
	{
		const int id = clip_id(name);
		if (id >= 0)
		{
			//from its start, played last (it wins over the others)
			stop(name);
			Track track;
			track.m_clip = size_t(id);
			track.m_loop = loop;
			track.m_speed = speed;
			m_tracks.push_back(track);
		}
		return id >= 0;
	}

	void Animator::play_all(bool loop)
	{
		m_tracks.clear();
		for (size_t i = 0; i < m_clips.size(); ++i)
		{
			Track track;
			track.m_clip = i;
			track.m_loop = loop;
			m_tracks.push_back(track);
		}
	}

	void Animator::stop(const std::string& name)
	{
		const int id = clip_id(name);
		m_tracks.erase
		(
			std::remove_if(m_tracks.begin(), m_tracks.end(), [id](const Track& track) { return int(track.m_clip) == id; }),
			m_tracks.end()
		);
	}

	void Animator::stop_all()
	{
		m_tracks.clear();
	}

	bool Animator::playing(const std::string& name) const
	{
		const int id = clip_id(name);
		const Track* played = id >= 0 ? track(size_t(id)) : nullptr;
		return played && !played->m_ended;
	}

	float Animator::time(const std::string& name) const
	{
		const int id = clip_id(name);
		const Track* played = id >= 0 ? track(size_t(id)) : nullptr;
		return played ? played->m_time : 0.0f;
	}

	void Animator::time(const std::string& name, float time)
	{
		const int id = clip_id(name);
		if (Track* played = id >= 0 ? track(size_t(id)) : nullptr)
		{
			played->m_time = time;
			played->m_ended = false;
		}
	}

	void Animator::find_targets()
	{
		m_targets.clear();
		auto owner = actor().lock();
		m_targets.reserve(m_clips.size());
		for (const auto& clip : m_clips)
		{
			std::vector< Weak<Actor> > targets;
			targets.reserve(clip.m_channels.size());
			for (const auto& channel : clip.m_channels)
			{
				Shared<Actor> target;
				if (owner)
				{
					target = owner->find(channel.m_target);
				}
				if (owner && !target)
				{
					context().logger()->warning("Animator: " + clip.m_name + ", no actor " + channel.m_target);
				}
				targets.push_back(target);
			}
			m_targets.push_back(std::move(targets));
		}
		m_targets_found = bool(owner);
	}

	void Animator::apply(size_t clip, float time)
	{
		const Clip& played = m_clips[clip];
		const auto& targets = m_targets[clip];
		for (size_t i = 0; i < played.m_channels.size(); ++i)
		{
			if (auto target = targets[i].lock())
			{
				const Channel& channel = played.m_channels[i];
				const Vec4 value = AuxAnimator::sample(channel, time);
				switch (channel.m_property)
				{
				case P_ROTATION: target->rotation(AuxAnimator::to_quat(value)); break;
				case P_SCALE:    target->scale(Vec3(value)); break;
				default:
				case P_POSITION: target->position(Vec3(value)); break;
				}
			}
		}
	}

	void Animator::on_update(double delta_time)
	{
		if (m_autoplay && !m_started)
		{
			play_all(true);
		}
		m_started = true;
		if (!m_targets_found)
		{
			find_targets();
		}
		for (auto& track : m_tracks)
		{
			const Clip& clip = m_clips[track.m_clip];
			if (!track.m_ended)
			{
				track.m_time += float(delta_time) * m_speed * track.m_speed;
				const bool past = track.m_time >= clip.m_duration;
				if (past && track.m_loop && clip.m_duration > 0.0f)
				{
					track.m_time = std::fmod(track.m_time, clip.m_duration);
				}
				else if (past)
				{
					//once: its last pose, then it ends
					track.m_time = clip.m_duration;
					track.m_ended = true;
				}
				else if (track.m_time < 0.0f && track.m_loop && clip.m_duration > 0.0f)
				{
					//backward (a speed under 0)
					track.m_time = clip.m_duration + std::fmod(track.m_time, clip.m_duration);
				}
				apply(track.m_clip, track.m_time);
			}
		}
	}

	//events
	void Animator::on_attach(Actor& entity)
	{
		m_targets_found = false;
	}

	void Animator::on_deattch()
	{
		m_targets.clear();
		m_targets_found = false;
	}

	//packed
	Animator::Packed Animator::pack() const
	{
		Packed packed;
		for (const auto& clip : m_clips)
		{
			packed.m_clip_names.push_back(clip.m_name);
			packed.m_clip_durations.push_back(clip.m_duration);
			packed.m_clip_channels.push_back(int(clip.m_channels.size()));
			for (const auto& channel : clip.m_channels)
			{
				packed.m_channel_targets.push_back(channel.m_target);
				packed.m_channel_kinds.push_back(int(channel.m_property) * 4 + int(channel.m_interpolation));
				packed.m_channel_keys.push_back(int(channel.m_times.size()));
				packed.m_key_times.insert(packed.m_key_times.end(), channel.m_times.begin(), channel.m_times.end());
				packed.m_key_values.insert(packed.m_key_values.end(), channel.m_values.begin(), channel.m_values.end());
			}
		}
		return packed;
	}

	void Animator::unpack(const Packed& packed)
	{
		std::vector<Clip> clips;
		size_t channel_id = 0;
		size_t time_id = 0;
		size_t value_id = 0;
		const size_t clip_count = std::min({ packed.m_clip_names.size(), packed.m_clip_durations.size(), packed.m_clip_channels.size() });
		const size_t channel_count = std::min({ packed.m_channel_targets.size(), packed.m_channel_kinds.size(), packed.m_channel_keys.size() });
		bool valid = true;
		for (size_t c = 0; c < clip_count && valid; ++c)
		{
			Clip clip;
			clip.m_name = packed.m_clip_names[c];
			clip.m_duration = packed.m_clip_durations[c];
			for (int i = 0; i < packed.m_clip_channels[c] && valid; ++i)
			{
				valid = channel_id < channel_count;
				if (valid)
				{
					Channel channel;
					channel.m_target = packed.m_channel_targets[channel_id];
					channel.m_property = Property(packed.m_channel_kinds[channel_id] / 4);
					channel.m_interpolation = Interpolation(packed.m_channel_kinds[channel_id] % 4);
					const size_t keys = size_t(std::max(packed.m_channel_keys[channel_id], 0));
					const size_t values = keys * (channel.m_interpolation == I_CUBIC ? 3 : 1);
					valid = time_id + keys <= packed.m_key_times.size() && value_id + values <= packed.m_key_values.size();
					if (valid)
					{
						channel.m_times.assign(packed.m_key_times.begin() + time_id, packed.m_key_times.begin() + time_id + keys);
						channel.m_values.assign(packed.m_key_values.begin() + value_id, packed.m_key_values.begin() + value_id + values);
						clip.m_channels.push_back(std::move(channel));
					}
					time_id += keys;
					value_id += values;
					++channel_id;
				}
			}
			clips.push_back(std::move(clip));
		}
		if (!valid)
		{
			context().logger()->warning("Animator: its clips are not whole, some of them lost");
		}
		this->clips(clips);
	}

	//serialize
	void Animator::serialize(Data::Archive& archive)
	{
		Data::serialize(archive, this);
	}

	void Animator::serialize_json(Data::JsonValue& archive)
	{
		Data::serialize_json(archive, this);
	}

	void Animator::deserialize(Data::Archive& archive)
	{
		Data::deserialize(archive, this);
		unpack(m_packed);
		m_packed = Packed();
	}

	void Animator::deserialize_json(Data::JsonValue& archive)
	{
		Data::deserialize_json(archive, this);
		unpack(m_packed);
		m_packed = Packed();
	}
}
}
