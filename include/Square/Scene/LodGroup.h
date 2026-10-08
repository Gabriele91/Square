//
//  LodGroup.h
//  Square
//
//  The levels of detail of an object (as Unity's LODGroup): on its actor, each level a child of
//  it (by its name: "<name>_lod0" the detailed one, "<name>_lod1"...: the converter makes them
//  from the nodes of a model so named). For each camera one level is drawn (all the renderables
//  under its child), the others not, none past the last one (culled):
//  - SCREEN (the default): by the share of the height of the screen of the object (its size over
//    the height of the view at its distance; 1: as tall as the screen), level i while it is at
//    least thresholds[i] (they go down: 0.6, 0.3, 0.1);
//  - DISTANCE: by the distance of the camera from its center, level i while it is under
//    thresholds[i] (they go up: 100, 200, 400).
//  Its bounds (center, size: the largest side of the box of its renderables, in the space of its
//  actor) from its renderables when not given.
//  A change of level is a cross-fade (as Unity's animated cross-fade): for the fade duration of
//  the settings of its world (Render::LevelOfDetailSettings) both levels are drawn, the pixels of a pattern of the screen going from one to the other
//  (Render::Renderable::lod_fade); also a level that appears or goes (culled).
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"
#include "Square/Render/LevelOfDetail.h"
#include "Square/Render/Renderable.h"
#include <string>
#include <vector>

namespace Square
{
namespace Scene
{
	enum class LodMode : int
	{
		SCREEN,
		DISTANCE
	};

	class SQUARE_API LodGroup : public Square::Scene::Component
							   , public Square::Render::LevelOfDetail
	{
	public:
		SQUARE_OBJECT(LodGroup)

		//a level: its child (by its name), where it ends (see the modes)
		struct Level
		{
			std::string m_actor;
			float       m_threshold{ 0.0f };
		};

		LodGroup(Square::Context& context);

		//how a level is chosen
		void mode(LodMode mode);
		LodMode mode() const { return m_mode; }

		//its levels, the detailed one first
		void levels(const std::vector<Level>& levels);
		const std::vector<Level>& levels() const { return m_levels; }

		//its bounds in the space of its actor: center, size (the largest side)
		void bounds(const Square::Vec3& center, float size);
		const Square::Vec3& center() const { return m_center; }
		float size() const { return m_size; }
		//... from its renderables now (false: none)
		bool recalculate_bounds();

		//the level drawn by the last camera (levels().size(): none)
		size_t shown() const { return m_shown; }

		//the level of a camera (by the mode; a level forced by the settings: that one)
		size_t level(const Square::Render::Camera& camera, const Square::Render::LevelOfDetailSettings& settings) const;

		//the renderables of its levels found again (its children changed)
		void refresh();

		//LevelOfDetail
		virtual void select(const Square::Render::Camera& camera, const Square::Render::LevelOfDetailSettings& settings) override;

		//events
		virtual void on_attach(Square::Scene::Actor& entity) override;
		virtual void on_deattch() override;

		//regs
		static void object_registration(Square::Context& ctx);

		//serialize
		virtual void serialize(Square::Data::Archive& archive)  override;
		virtual void serialize_json(Square::Data::JsonValue& archive) override;
		//deserialize
		virtual void deserialize(Square::Data::Archive& archive) override;
		virtual void deserialize_json(Square::Data::JsonValue& archive) override;

	private:

		using Renderables = std::vector< Square::Weak<Square::Render::Renderable> >;

		//the renderables of each level, found when needed (none yet, or one gone)
		bool found() const;
		void find();
		//the level of a camera: a cross-fade to it (its seconds; 0: at once), its renderables faded
		void show(size_t level, float fade_duration);
		void apply();

		LodMode                  m_mode{ LodMode::SCREEN };
		std::vector<Level>       m_levels;
		Square::Vec3             m_center{ 0.0f };
		float                    m_size{ 0.0f };
		std::vector<Renderables> m_renderables;
		bool                     m_found{ false };
		size_t                   m_shown{ ~size_t(0) };    //(none applied yet)
		size_t                   m_previous{ ~size_t(0) }; //the level fading out (none)
		float                    m_fade{ 1.0f };           //how far the cross-fade is (1 done)
		double                   m_time{ -1.0 };           //of the last select (seconds)
		bool                     m_applied{ false };       //the fades of now on its renderables
	};
}
}
