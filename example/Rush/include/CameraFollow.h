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
//  A spring arm (as the cameras of Unreal): its place is on a ray from the target (its head) to
//  the pivot; a mesh collider between, the place comes in along the ray in front of it (quickly
//  in, slowly back out: no jumps), so the camera does not hit the walls (its sphere only a
//  last guard). Four rays (the middle, its sides, over it), the longest kept: a wall stops
//  them all, a small rock only one (it does not pull the camera in). Its rotation turns toward the target smoothly; right over the target (no yaw to
//  look along) it keeps its yaw.
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
		float        follow{ 0.16f };              //share of the way to the pivot per step
		float        intro_follow{ 0.0025f };      //start: share of the way per step at first
		float        intro_time{ 3.0f };           //start: seconds to reach the usual follow
		bool         hold_backward{ true };        //does not follow while the target moves backward
		float        head{ 2.0f };                 //the start of the arm over the target (world up)
		float        arm_margin{ 1.5f };           //the arm stops this much before a wall
		float        arm_spread{ 1.6f };           //its rays apart (sideways, up): a wall stops them all, a small rock one
		float        arm_min{ 3.0f };              //the arm at least this long
		float        arm_in{ 0.5f };               //share of the way per step of the arm getting shorter
		float        arm_out{ 0.04f };             //share of the way per step of the arm getting longer
		float        turn{ 0.6f };                 //share of the turn per step toward the target
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
	//rotation that looks at the target from where the camera is (right over it: the yaw it had)
	Square::Quat look_at_target() const;
	//the pivot on the arm: in front of the walls between the head of the target and it
	Square::Vec3 arm_pivot(const Square::Vec3& head, const Square::Vec3& pivot, float steps);

	Settings                           m_settings;
	Square::Weak<Square::Scene::Actor> m_target;
	Square::Vec3                       m_target_previous{ 0.0f };
	float                              m_intro{ -1.0f }; //seconds of the start glide (< 0: none)
	float                              m_arm{ -1.0f };   //the length of the arm now (< 0: its full length)
	float                              m_yaw{ 0.0f };    //the last yaw it looked along (radians)
};
