//
//  CameraFollow.h
//  Rush
//
//  A chase camera, as a component of the camera actor: every frame it goes a share of the way
//  towards a point behind the target (offset in target space) and looks at the target (no roll).
//  At the start (target set, no snap) it is in its place of the scene: from there it glides
//  behind the target, looking at it, slowly at first (intro_follow), then faster until the
//  usual follow (in intro_time seconds).
//  It moves in on_update, before the CollisionSystem: with a SphereCollider on the camera, the
//  collisions stop it on the map before it is drawn.
//  While the target moves backward it holds its place (it looks at it, like the original).
//  Its walls are the "camera_bounds..." meshes of the map (one sided colliders of the camera
//  bounds type, see Arena): from out of them (the start) it comes in, in them it stays.
//
#pragma once
#include <Square/Square.h>
#include <RushTypes.h>

class CameraFollow : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(CameraFollow)

	struct Settings
	{
		Square::Vec3 offset{ 0.0f, 6.0f, -27.0f }; //pivot of the camera, in target space
		float        follow{ 0.1f };               //share of the way to the pivot per step
		float        intro_follow{ 0.0025f };      //start: share of the way per step at first
		float        intro_time{ 3.0f };           //start: seconds to reach the usual follow
		bool         hold_backward{ true };        //does not follow while the target moves backward
		double       step{ 1.0 / 60.0 };           //seconds of a step
	};

	//Registration in context
	static void object_registration(Square::Context& ctx);

	CameraFollow(Square::Context& context);

	void settings(const Settings& settings) { m_settings = settings; }
	const Settings& settings() const { return m_settings; }

	//who it follows
	void target(Square::Shared<Square::Scene::Actor> target);
	Square::Shared<Square::Scene::Actor> target() const { return m_target.lock(); }

	//straight to the pivot (after a teleport of the target, or at the start)
	void snap();


	//events
	virtual void on_update(double delta_time) override;

	//serialize (no attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	//rotation that looks at the target from where the camera is
	Square::Quat look_at_target() const;

	Settings                           m_settings;
	Square::Weak<Square::Scene::Actor> m_target;
	Square::Vec3                       m_target_previous{ 0.0f };
	float                              m_intro{ -1.0f }; //seconds of the start glide (< 0: none)
};
