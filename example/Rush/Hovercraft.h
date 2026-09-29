//
//  Hovercraft.h
//  Rush
//
//  A hovercraft that drives on the collisions of Rush (Collision.h), at the fixed steps of the
//  CollisionWorld (the same motion at any frame rate). Every step, before the collisions:
//  - update: the body is aligned to the four wheels (where the collisions of the last step
//    left them), left/right then front/rear; its velocity;
//  - update_input: steering, then on the ground (a contact of the body under it) throttle/
//    brake/drag forward along the body and a little gravity, in the air the velocity plus
//    gravity; the wheels back under the body.
//  After the steps of a frame the actor shows a pose between the last two steps (the pose of
//  the simulation is put back before the next step).
//  Who drives it writes its Input: the player (HovercraftInput) or an NPC (HovercraftAI). The
//  chase camera is a component of the camera (CameraFollow).
//  Body: an ellipsoid on the actor (around the hull); wheels: spheres on four children (a
//  quarter of the hull height), height probes that come down on the ground under the corners.
//  The values are per step of Settings::step seconds (1/60: the tuning of the game).
//
#pragma once
#include <Square/Square.h>
#include <array>
#include "Collision.h"

class HovercraftDriver : public Square::Scene::Component, public FixedStepListener
{
public:
	SQUARE_OBJECT(HovercraftDriver)

	struct Input
	{
		bool forward{ false };
		bool backward{ false };
		bool left{ false };
		bool right{ false };
	};

	struct Settings
	{
		float gravity{ -0.007f };      //vertical speed added every step
		float acceleration{ 0.065f };  //speed gained per step with the throttle
		float drag{ 0.9725f };           //share of the speed kept per step without throttle
		float max_speed{ 0.7f };       //forward
		float max_reverse{ -0.55f };   //backward
		float turn{ 2.2f };            //degrees of yaw per step, per unit of speed
		float idle_turn{ 1.8f };       //degrees of yaw per step at least, also still (0: it turns only moving forward)
		int   body_type{ 1 };          //collision type of the body
		int   wheel_type{ 2 };         //collision type of the wheels
		int   scene_type{ 3 };         //collision type of the ground
		float floor_normal_y{ 0.5f };  //a body contact with normal.y under it is a wall (in front, or pushing down): no drive, it falls
		Square::Vec2 body_radius_scale{ 1.0f, 1.0f }; //radii of the body from the hull: x the x/z radius (half the longer side), y the y radius (half the height)
		double step{ 1.0 / 60.0 };     //seconds of a step of the values above (a fixed step of another length scales them)
	};

	//Registration in context
	static void object_registration(Square::Context& ctx);

	HovercraftDriver(Square::Context& context);

	//settings: before the first spawn/update (body and wheels are made then, from the meshes
	//of the actor, with its scale)
	void settings(const Settings& settings) { m_settings = settings; }
	const Settings& settings() const { return m_settings; }

	//controls, held (set by who drives it: the player, an NPC)
	void input(const Input& input) { m_input = input; }
	const Input& input() const { return m_input; }
	//speed along the body, per step (negative: backward)
	float speed() const { return m_speed; }

	//drop it on the first surface under start, still, turned by yaw degrees around the up axis
	//(0: forward along +z, 90: along +x)
	void spawn(const Square::Vec3& start, float yaw = 0.0f);

	//events
	virtual void on_deattch() override;
	virtual void on_update(double delta_time) override;
	//fixed steps (CollisionWorld)
	virtual void on_fixed_update(double step) override;
	virtual void on_fixed_interpolate(double alpha) override;

	//serialize (no attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	//wheels: front left, front right, back left, back right
	enum Wheel { FRONT_LEFT, FRONT_RIGHT, BACK_LEFT, BACK_RIGHT };

	//position and rotation of the actor
	struct Pose
	{
		Square::Vec3 m_position{ 0.0f };
		Square::Quat m_rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
	};

	//body and wheels, once the actor is in a world (false while it is not)
	bool set_wheels();
	//a step
	void update(float steps);
	void update_input(float steps);
	//the wheels back under the body
	void place_wheels();
	//the pose of the actor
	Pose pose() const;
	void pose(const Pose& pose);

	Settings                             m_settings;
	Input                                m_input;
	//body and wheels (offsets in body space, world units)
	Square::Shared<SphereCollider>                      m_body;
	std::array<Square::Shared<Square::Scene::Actor>, 4> m_wheels;
	std::array<Square::Vec3, 4>                         m_wheel_offsets;
	//state
	float        m_speed{ 0.0f };
	Square::Vec3 m_velocity{ 0.0f };          //of the last step, per step
	Square::Vec3 m_previous{ 0.0f };
	bool         m_has_previous{ false };
	//poses of the last two steps (the actor shows a pose between them)
	Pose         m_pose_previous;
	Pose         m_pose_current;
	bool         m_has_pose{ false };
	bool         m_stepped{ false };          //a step ran after the last interpolation: the actor has the pose of the simulation
};
