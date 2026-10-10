//
//  ParticleEmitter.h
//  Square
//
//  An emitter of particles: simulated on the CPU (every update of its actor), drawn as sprites
//  (Render::SpriteBatch: a draw call for at most SpriteBatch::sprites_max of them), its material a
//  sprite effect (Sprite, SpriteAdditive). Its particles are born in its box (the space of its
//  actor), rate a second, toward its direction within its spread (a cone), at a speed, live a
//  time, then fall by the gravity (world) and slow by the drag; their size and their color go from
//  the ones of the start to the ones of the end, they turn; the frames of the flipbook of the
//  material by their life (frame_by_life), else by the time. They live in the world (the emitter
//  moves, they stay); prewarm: the seconds simulated when it starts (full at once). Saved with its
//  scene: the converter makes it from a node "square_particles" (a plane: its box, its material;
//  its settings "square_<attribute>").
//
#pragma once
#include <random>
#include <vector>
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Resource/Material.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/SpriteBatch.h"

namespace Square
{
namespace Scene
{
	class SQUARE_API ParticleEmitter : public Square::Scene::Component
	                                 , public Square::Render::Renderable
	{
	public:
		SQUARE_OBJECT(ParticleEmitter)

		struct Settings
		{
			float m_rate{ 10.0f };                       //born a second
			int   m_max{ 512 };                          //alive at most
			Vec2  m_life{ 1.0f, 2.0f };                  //seconds, between
			Vec3  m_box{ 0.0f };                         //half sides of the box where they are born (the space of the actor)
			Vec3  m_direction{ 0.0f, 1.0f, 0.0f };       //where they go (the space of the actor)
			float m_spread{ 0.3f };                      //radians around it (a cone)
			Vec2  m_speed{ 1.0f, 2.0f };                 //world units a second, between
			Vec3  m_gravity{ 0.0f, 0.0f, 0.0f };         //world units a second a second (world)
			float m_drag{ 0.0f };                        //the speed lost a second (a share)
			Vec2  m_size{ 1.0f, 2.0f };                  //at the start, at the end (world units)
			float m_size_random{ 0.25f };                //a share of the size of each one, at random
			Vec4  m_color_start{ 1.0f };                 //at the start (alpha too)
			Vec4  m_color_end{ 1.0f, 1.0f, 1.0f, 0.0f }; //at the end
			Vec2  m_spin{ -0.5f, 0.5f };                 //radians a second, between
			bool  m_frame_by_life{ false };              //the frames of the flipbook by their life
			float m_prewarm{ 0.0f };                     //seconds simulated when it starts
		};

		ParticleEmitter(Square::Context& context);
		virtual ~ParticleEmitter();

		//its material (a sprite effect), its settings (a new max: its pool made again)
		void material(const Square::Shared<Square::Resource::Material>& material) { m_material = material; }
		const Square::Shared<Square::Resource::Material>& material_resource() const { return m_material; }
		void settings(const Settings& settings);
		const Settings& settings() const { return m_settings; }

		//it emits (false: the ones alive go on, no new one)
		void emitting(bool emitting) { m_emitting = emitting; }
		bool emitting() const { return m_emitting; }
		//the particles alive
		size_t alive() const { return m_alive; }

		//the simulation, every update of its actor
		virtual void on_update(double delta_time) override;

		//Renderable
		virtual size_t materials_count() const override;
		virtual Square::Weak<Square::Render::Material> material(size_t i = 0) const override;
		virtual void draw
		(
			  Square::Render::Context& render
			, size_t material_id
			, Square::Render::EffectPassInputs& input
			, Square::Render::EffectPass& pass
			, int draw_id = 0
		) override;
		virtual bool support_culling() const override;
		virtual const Square::Geometry::OBoundingBox& bounding_box() override;
		virtual Square::Weak<Square::Render::Transform> transform() const override;

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

		//a particle of the pool: alive or free; where (world), its speed (world), its age and its
		//life (seconds), its size (a share), its rotation and its spin
		struct Particle
		{
			bool  m_alive{ false };
			Vec3  m_position{ 0.0f };
			Vec3  m_velocity{ 0.0f };
			float m_age{ 0.0f };
			float m_life{ 1.0f };
			float m_scale{ 1.0f };
			float m_rotation{ 0.0f };
			float m_spin{ 0.0f };
		};

		//the pool of max particles (all free), its sprites, allocated once
		void make_pool();
		//the simulation of some seconds (born, moved, dead)
		void simulate(float seconds);
		//a free particle of the pool born now (world)
		void born(Particle& particle, const Mat4& model);
		//a random number between two
		float random(const Vec2& range);

		Square::Shared< Square::Resource::Material > m_material;
		Settings                                     m_settings;
		std::vector<Particle>                        m_particles; //the pool: max of them, alive or free
		std::vector<uint32>                          m_free;      //the free ones (a stack of their indices)
		size_t                                       m_alive{ 0 };
		std::vector< Square::Render::SpriteInstance > m_sprites;  //max of them: the alive ones drawn
		Square::Render::SpriteBatch                  m_batch;
		std::mt19937                                 m_random{ 91 };
		float                                        m_to_emit{ 0.0f }; //a share of a particle not born yet
		bool                                         m_emitting{ true };
		bool                                         m_warmed{ false };
		Square::Geometry::OBoundingBox               m_obb_global;
		Square::Weak< Square::Render::Transform >    m_transform;
	};
}
}
