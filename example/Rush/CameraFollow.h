//
//  CameraFollow.h
//  Rush
//
//  A chase camera, as a component of the camera actor: every frame it goes a share of the way
//  towards a point behind the target (offset in target space) and looks at the target (no roll).
//  It moves in on_update, before the CollisionSystem: with a SphereCollider on the camera, the
//  collisions stop it on the map before it is drawn.
//  While the target moves backward it holds its place (it looks at it, like the original).
//
#pragma once
#include <Square/Square.h>

class CameraFollow : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(CameraFollow)

	struct Settings
	{
		Square::Vec3 offset{ 0.0f, 8.0f, -25.0f }; //pivot of the camera, in target space
		float        follow{ 0.1f };               //share of the way to the pivot per step
		bool         hold_backward{ true };        //does not follow while the target moves backward
		double       step{ 1.0 / 60.0 };           //seconds of a step
	};

	//Registration in context
	static void object_registration(Square::Context& ctx);

	CameraFollow(Square::Context& context);

	Settings& settings() { return m_settings; }

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
	void look_at_target();

	Settings                           m_settings;
	Square::Weak<Square::Scene::Actor> m_target;
	Square::Vec3                       m_target_previous{ 0.0f };
};
