//
//  Hovercraft.h
//  Rush
//
//  The Blitz3D "Autophysik - Driver Demo" as a component, on the collisions of Rush
//  (Collision.h). Every frame, before the CollisionSystem (UpdateWorld):
//  - update: the body is aligned to the four wheels (where the collisions left them), left/
//    right then front/rear (AlignToVector); its velocity;
//  - update_input: steering, then on the scene (EntityCollided) throttle/brake/drag forward
//    along the body and a little gravity (MoveEntity, TranslateEntity), in the air the
//    velocity plus gravity; the wheels back under the body.
//  Who drives it writes its Input: the player (HovercraftInput) or, later, an NPC. The chase
//  camera is a component of the camera (CameraFollow).
//  Body: a sphere on the actor (half the hull height); wheels: spheres on four children (a
//  quarter of it), height probes that come down on the ground under the corners.
//  The values are per frame of the original (a step of 1/60 s): a frame of dt seconds counts
//  dt / step steps.
//
#pragma once
#include <Square/Square.h>
#include <array>

class SphereCollider;

class HovercraftDriver : public Square::Scene::Component
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
		float gravity{ -0.005f };      //GRAVITY: vertical motion added every step
		float acceleration{ 0.07f };   //VEL: speed gained per step with the throttle
		float drag{ 0.97f };           //CDOWN: share of the speed kept per step without throttle
		float max_speed{ 0.75f };       //forward
		float max_reverse{ -0.6f };    //backward
		float turn{ 2.2f };            //degrees of yaw per step, per unit of speed
		float idle_turn{ 1.8f };       //degrees of yaw per step at least, also still (0: like the original, only moving forward)
		int   body_type{ 1 };          //collision type of the body (BODY)
		int   wheel_type{ 2 };         //collision type of the wheels (WHEEL)
		int   scene_type{ 3 };         //collision type of the ground (SCENE)
		float floor_normal_y{ 0.5f };  //a body contact with normal.y under it is a wall (in front, or pushing down): no drive, it falls
		double step{ 1.0 / 60.0 };     //seconds of a step (a frame of the original)
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

	//serialize (no attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	//wheels: front left, front right, back left, back right (1..4 of the original)
	enum Wheel { FRONT_LEFT, FRONT_RIGHT, BACK_LEFT, BACK_RIGHT };

	//body and wheels, once the actor is in a world (false while it is not)
	bool set_wheels();
	//a frame
	void update(float steps);
	void update_input(float steps);
	//the wheels back under the body
	void place_wheels();

	Settings                             m_settings;
	Input                                m_input;
	//body and wheels (offsets in body space, world units)
	Square::Shared<SphereCollider>                      m_body;
	std::array<Square::Shared<Square::Scene::Actor>, 4> m_wheels;
	std::array<Square::Vec3, 4>                         m_wheel_offsets;
	//state
	float        m_speed{ 0.0f };
	Square::Vec3 m_velocity{ 0.0f };          //of the last frame, per step
	Square::Vec3 m_previous{ 0.0f };
	float        m_previous_steps{ 0.0f };    //0: no previous frame
};
