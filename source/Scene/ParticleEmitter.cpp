//
//  ParticleEmitter.cpp
//  Square
//
//  See ParticleEmitter.h for the high level description.
//
#include <algorithm>
#include <cmath>
#include <limits>
#include "Square/Config.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/ParticleEmitter.h"
#include "Square/Render/Material.h"
#include "Square/Core/ClassObjectRegistration.h"

namespace Square
{
namespace Scene
{
	namespace AuxParticleEmitter
	{
		//the longest step of the simulation (a slow frame: more steps)
		static constexpr float s_step = 1.0f / 30.0f;

		//an attribute of the settings of an emitter (a field of them: the functions of an attribute
		//capture nothing)
		template < typename T, T ParticleEmitter::Settings::* Field >
		static void setting(Square::Context& ctx, const char* name, const T& value)
		{
			ctx.add_attribute_function<ParticleEmitter, T>
				(name
				, value
				, [](const ParticleEmitter* emitter) -> T { return emitter->settings().*Field; }
				, [](ParticleEmitter* emitter, const T& set)
				{
					ParticleEmitter::Settings settings = emitter->settings();
					settings.*Field = set;
					emitter->settings(settings);
				});
		}

		//a unit vector around an axis within an angle (a cone), by two random numbers [0, 1)
		static Vec3 in_cone(const Vec3& axis, float angle, float u, float v)
		{
			const float cos_theta = 1.0f - u * (1.0f - std::cos(angle));
			const float sin_theta = std::sqrt(std::max(1.0f - cos_theta * cos_theta, 0.0f));
			const float phi = 6.28318530718f * v;
			const Vec3 helper = std::abs(axis.y) < 0.99f ? Vec3(0.0f, 1.0f, 0.0f) : Vec3(1.0f, 0.0f, 0.0f);
			const Vec3 side = normalize(cross(helper, axis));
			const Vec3 other = cross(axis, side);
			return normalize(axis * cos_theta + (side * std::cos(phi) + other * std::sin(phi)) * sin_theta);
		}
	}

	SQUARE_CLASS_OBJECT_REGISTRATION(ParticleEmitter);

	//regs
	void ParticleEmitter::object_registration(Square::Context& ctx)
	{
		using namespace Square::Resource;
		using namespace AuxParticleEmitter;
		ctx.add_object<ParticleEmitter>();
		// Material
		ctx.add_attribute_function<ParticleEmitter, std::string>
			("material"
			, std::string()
			, [](const ParticleEmitter* emitter) -> std::string
			{
				return emitter->m_material ? emitter->m_material->resource_untyped_name() : std::string();
			}
			, [](ParticleEmitter* emitter, const std::string& name)
			{
				if (name.size())
				{
					emitter->m_material = emitter->context().resource<Material>(name);
					if (!emitter->m_material)
					{
						emitter->context().logger()->warning("ParticleEmitter: unable to load the material " + name);
					}
				}
			});
		// Settings
		const Settings defaults;
		setting<float, &Settings::m_rate>(ctx, "rate", defaults.m_rate);
		setting<int, &Settings::m_max>(ctx, "max", defaults.m_max);
		setting<Vec2, &Settings::m_life>(ctx, "life", defaults.m_life);
		setting<Vec3, &Settings::m_box>(ctx, "box", defaults.m_box);
		setting<Vec3, &Settings::m_direction>(ctx, "direction", defaults.m_direction);
		setting<float, &Settings::m_spread>(ctx, "spread", defaults.m_spread);
		setting<Vec2, &Settings::m_speed>(ctx, "speed", defaults.m_speed);
		setting<Vec3, &Settings::m_gravity>(ctx, "gravity", defaults.m_gravity);
		setting<float, &Settings::m_drag>(ctx, "drag", defaults.m_drag);
		setting<Vec2, &Settings::m_size>(ctx, "size", defaults.m_size);
		setting<float, &Settings::m_size_random>(ctx, "size_random", defaults.m_size_random);
		setting<Vec4, &Settings::m_color_start>(ctx, "color_start", defaults.m_color_start);
		setting<Vec4, &Settings::m_color_end>(ctx, "color_end", defaults.m_color_end);
		setting<Vec2, &Settings::m_spin>(ctx, "spin", defaults.m_spin);
		setting<bool, &Settings::m_frame_by_life>(ctx, "frame_by_life", defaults.m_frame_by_life);
		setting<float, &Settings::m_prewarm>(ctx, "prewarm", defaults.m_prewarm);
	}

	ParticleEmitter::ParticleEmitter(Square::Context& context)
	: Component(context)
	, m_batch(context)
	{
		make_pool();
	}

	void ParticleEmitter::settings(const Settings& settings)
	{
		const bool resize = settings.m_max != m_settings.m_max;
		m_settings = settings;
		if (resize)
		{
			make_pool();
		}
	}

	void ParticleEmitter::make_pool()
	{
		const size_t count = size_t(std::max(m_settings.m_max, 0));
		m_particles.assign(count, Particle());
		m_sprites.assign(count, Render::SpriteInstance());
		//all free: the last on the top of the stack (born in their order)
		m_free.resize(count);
		for (size_t i = 0; i != count; ++i)
		{
			m_free[i] = uint32(count - 1 - i);
		}
		m_alive = 0;
	}

	ParticleEmitter::~ParticleEmitter()
	{
	}

	float ParticleEmitter::random(const Vec2& range)
	{
		std::uniform_real_distribution<float> share(0.0f, 1.0f);
		return range.x + (range.y - range.x) * share(m_random);
	}

	void ParticleEmitter::born(Particle& particle, const Mat4& model)
	{
		using namespace AuxParticleEmitter;
		particle.m_alive = true;
		particle.m_age = 0.0f;
		//where: in its box (the space of the actor)
		const Vec3 local
		(
			  random(Vec2(-1.0f, 1.0f)) * m_settings.m_box.x
			, random(Vec2(-1.0f, 1.0f)) * m_settings.m_box.y
			, random(Vec2(-1.0f, 1.0f)) * m_settings.m_box.z
		);
		particle.m_position = Vec3(model * Vec4(local, 1.0f));
		//toward: its direction (the space of the actor) within the spread
		Vec3 axis = Vec3(model * Vec4(m_settings.m_direction, 0.0f));
		axis = length(axis) > 0.0f ? normalize(axis) : Vec3(0.0f, 1.0f, 0.0f);
		const Vec3 way = in_cone(axis, m_settings.m_spread, random(Vec2(0.0f, 1.0f)), random(Vec2(0.0f, 1.0f)));
		particle.m_velocity = way * random(m_settings.m_speed);
		particle.m_life = std::max(random(m_settings.m_life), 0.01f);
		particle.m_scale = 1.0f + m_settings.m_size_random * random(Vec2(-1.0f, 1.0f));
		particle.m_rotation = random(Vec2(0.0f, 6.28318530718f));
		particle.m_spin = random(m_settings.m_spin);
	}

	void ParticleEmitter::simulate(float seconds)
	{
		Mat4 model(1.0f);
		if (auto transform = m_transform.lock())
		{
			model = transform->global_model_matrix();
		}
		//the new ones: free ones of the pool (a share of one kept for the next step; none free:
		//not born)
		if (m_emitting)
		{
			m_to_emit += std::max(m_settings.m_rate, 0.0f) * seconds;
			while (m_to_emit >= 1.0f)
			{
				if (!m_free.empty())
				{
					born(m_particles[m_free.back()], model);
					m_free.pop_back();
					m_alive += 1;
				}
				m_to_emit -= 1.0f;
			}
		}
		//moved, older; the dead ones free again
		const float keep = std::max(1.0f - m_settings.m_drag * seconds, 0.0f);
		for (size_t i = 0; i != m_particles.size(); ++i)
		{
			Particle& particle = m_particles[i];
			if (particle.m_alive)
			{
				particle.m_age += seconds;
				if (particle.m_age >= particle.m_life)
				{
					particle.m_alive = false;
					m_free.push_back(uint32(i));
					m_alive -= 1;
				}
				else
				{
					particle.m_velocity = (particle.m_velocity + m_settings.m_gravity * seconds) * keep;
					particle.m_position += particle.m_velocity * seconds;
					particle.m_rotation += particle.m_spin * seconds;
				}
			}
		}
	}

	void ParticleEmitter::on_update(double delta_time)
	{
		using namespace AuxParticleEmitter;
		//full at once: its first seconds
		if (!m_warmed)
		{
			for (float warmed = 0.0f; warmed < m_settings.m_prewarm; warmed += s_step)
			{
				simulate(s_step);
			}
			m_warmed = true;
		}
		//the seconds of the frame, in steps
		float seconds = std::min(float(delta_time), 0.25f);
		while (seconds > 0.0f)
		{
			const float step = std::min(seconds, s_step);
			simulate(step);
			seconds -= step;
		}
		//their box (world): around all of them, their largest size; none: around the emitter
		const float half = std::max(m_settings.m_size.x, m_settings.m_size.y) * (1.0f + m_settings.m_size_random) * 0.5f;
		Vec3 low(std::numeric_limits<float>::max());
		Vec3 high(std::numeric_limits<float>::lowest());
		for (const Particle& particle : m_particles)
		{
			if (particle.m_alive)
			{
				low = glm::min(low, particle.m_position);
				high = glm::max(high, particle.m_position);
			}
		}
		if (m_alive == 0)
		{
			Vec3 at(0.0f);
			if (auto transform = m_transform.lock())
			{
				at = Vec3(transform->global_model_matrix()[3]);
			}
			low = high = at;
		}
		m_obb_global.set(Mat3(1.0f), (low + high) * 0.5f, (high - low) * 0.5f + Vec3(half));
	}

	size_t ParticleEmitter::materials_count() const
	{
		return m_material ? 1 : 0;
	}

	Weak<Render::Material> ParticleEmitter::material(size_t i) const
	{
		Weak<Render::Material> material;
		if (i == 0 && m_material)
		{
			material = DynamicPointerCast<Render::Material>(m_material);
		}
		return material;
	}

	void ParticleEmitter::draw
	(
		  Render::Context& render
		, size_t material_id
		, Render::EffectPassInputs& input
		, Render::EffectPass& pass
		, int draw_id
	)
	{
		if (material_id == 0 && m_material && m_alive)
		{
			//each alive one a sprite (the first ones of the array) in the space of the actor (the
			//shader: by its matrix)
			Mat4 to_local(1.0f);
			if (auto transform = m_transform.lock())
			{
				to_local = inverse(transform->global_model_matrix());
			}
			size_t count = 0;
			for (const Particle& particle : m_particles)
			{
				if (particle.m_alive)
				{
					const float t = particle.m_age / particle.m_life;
					const float size = (m_settings.m_size.x + (m_settings.m_size.y - m_settings.m_size.x) * t) * particle.m_scale;
					Render::SpriteInstance& sprite = m_sprites[count++];
					sprite.m_center_rotation = Vec4(Vec3(to_local * Vec4(particle.m_position, 1.0f)), particle.m_rotation);
					sprite.m_size_frame = m_settings.m_frame_by_life ? Vec4(size, size, t, 1.0f) : Vec4(size, size, -1.0f, 0.0f);
					sprite.m_color = m_settings.m_color_start + (m_settings.m_color_end - m_settings.m_color_start) * t;
				}
			}
			m_batch.draw(render, pass, input, m_material->parameters(), draw_id, m_sprites.data(), count);
		}
	}

	bool ParticleEmitter::support_culling() const
	{
		return true;
	}

	const Geometry::OBoundingBox& ParticleEmitter::bounding_box()
	{
		return m_obb_global;
	}

	Weak<Render::Transform> ParticleEmitter::transform() const
	{
		return m_transform;
	}

	//events
	void ParticleEmitter::on_attach(Actor& entity)
	{
		m_transform = DynamicPointerCast<Render::Transform>(entity.shared_from_this());
	}

	void ParticleEmitter::on_deattch()
	{
		m_transform.reset();
	}

	//serialize
	void ParticleEmitter::serialize(Data::Archive& archive)
	{
		Data::serialize(archive, this);
	}

	void ParticleEmitter::serialize_json(Data::JsonValue& archive)
	{
		Data::serialize_json(archive, this);
	}

	void ParticleEmitter::deserialize(Data::Archive& archive)
	{
		Data::deserialize(archive, this);
	}

	void ParticleEmitter::deserialize_json(Data::JsonValue& archive)
	{
		Data::deserialize_json(archive, this);
	}
}
}
