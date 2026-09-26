//
//  Hovercraft.h
//  Rush
//
//  Hovercraft driven like the Blitz3D "Autophysik" car, with no physics engine:
//  - four probes under the corners of the hull find the ground; the hull is aligned to
//    them, first left/right then front/rear;
//  - the four wheels (the corners) share the motion on x/z, but each one moves on y on its
//    own: gravity always pulls it, the ground under it stops it and pushes it up as it
//    rises (off a ramp it keeps going up: a jump), and a wheel hitting takes away speed;
//  - the hull follows the wheels: aligned to them, and they are put back under its corners;
//  - it steers only while moving forward, faster the faster it goes;
//  - the camera moves towards a point behind the hull and looks at it.
//  The values are per step, and the simulation runs at a fixed 60 steps per second,
//  like the per frame loop of the original.
//  HovercraftDriver: the component that drives its actor as a hovercraft, every frame
//  (Component::on_update, before AppInterface::run: the input of the frame is there),
//  colliding with the CollisionWorld of its world.
//
#pragma once
#include <Square/Square.h>
#include <array>
#include <vector>
#include <memory>

class CollisionMesh;

class Hovercraft
{
public:
	struct Input
	{
		bool forward{ false };
		bool backward{ false };
		bool left{ false };
		bool right{ false };
	};

	struct Settings
	{
		float gravity{ -0.008f };      //vertical speed added every step, always
		float acceleration{ 0.04f };   //speed gained per step with the throttle
		float max_speed{ 0.7f };       //forward
		float max_reverse{ -0.5f };    //backward
		float drag{ 0.97f };           //share of the speed kept per step without throttle
		float turn{ 2.0f };            //degrees of yaw per step, per unit of speed
		float idle_turn{ 1.5f };       //degrees of yaw per step at least, also when still
		float hover{ 0.5f };           //gap between the hull and the ground
		float model_offset_y{ 0.0f };  //model up/down, share of the hull height (-0.25: a quarter lower)
		float probe_reach{ 2.0f };     //how far above the hull corners the probes start
		float wall_normal_y{ 0.7f };   //triangles with |normal.y| below it are walls
		float wall_stop{ 1.0f };       //speed lost hitting a wall head-on (1: all), less when grazing
		float contact_tolerance{ 0.25f }; //a wheel drives within this distance from the ground
		float max_climb{ 0.6f };       //most upward speed a wheel gets from the ground rising, per step
		float probe_depth{ 1000.0f };  //how far below a wheel the ground is looked for
		float wheel_impact{ 1.0f };    //speed lost per unit of impact speed, all 4 wheels hitting
		Square::Vec3 camera_offset{ 0.0f, 8.0f, -25.0f }; //chase point, in hull space
		float camera_follow{ 0.1f };   //share of the way to the chase point per step
		double step{ 1.0 / 60.0 };     //seconds of a simulation step
	};

	Hovercraft
	(
	  Square::Context& context
	, Square::Shared<Square::Scene::Actor> actor
	, Square::Shared<Square::Scene::Actor> camera
	, const CollisionMesh& scene
	, const Settings& settings = Settings()
	);

	//put the hull on the ground at x, z (hovering), still
	void spawn(float x, float z);
	//advance the simulation of dt seconds
	void update(double dt, const Input& input);

	float speed() const { return m_speed; }
	Settings& settings() { return m_settings; }

private:
	enum Corner { FRONT_LEFT, FRONT_RIGHT, BACK_LEFT, BACK_RIGHT };

	//one corner: moves on y on its own, the hull puts it back under its corner every step
	struct Wheel
	{
		float m_y{ 0.0f };          //height of the contact point
		float m_velocity{ 0.0f };   //vertical, per step
		float m_ground{ 0.0f };     //ground under it at the last step
		bool  m_has_ground{ false };
		bool  m_touching{ false };  //on the ground, or close to it
		//gravity, then the ground (height ground_now, if found): true if touching; impact is
		//the speed of the hit with the ground (landing, a step), 0 if none
		bool update(float ground_now, bool ground_found, const Settings& settings, float& impact);
	};

	void step(const Input& input);
	//ground height under point (from a bit above it); false if none
	bool ground(const Square::Vec3& point, float& height) const;
	//wheels under the corners, still
	void reset_wheels();
	void follow_camera();

	Square::Shared<Square::Scene::Actor> m_actor;
	Square::Shared<Square::Scene::Actor> m_camera;
	const CollisionMesh&                 m_scene;
	Settings                             m_settings;
	//hull, in its own space (from the bounding boxes of its meshes)
	std::array<Square::Vec3, 4> m_corners;   //bottom corners
	std::array<Wheel, 4>        m_wheels;
	std::vector<Square::Vec3>   m_wall_spheres; //centers along the hull, against the walls
	float                       m_radius{ 1.0f };
	//state
	Square::Vec3 m_velocity{ 0.0f };          //x/z shared by the wheels, per step
	float        m_speed{ 0.0f };
	double       m_time{ 0.0 };
};


//the component: drives its actor as a hovercraft
class HovercraftDriver : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(HovercraftDriver)

	//Registration in context
	static void object_registration(Square::Context& ctx);

	HovercraftDriver(Square::Context& context);

	//settings and chase camera: before the first spawn/update (the hovercraft is made then,
	//from the meshes of the actor, with its scale)
	Hovercraft::Settings& settings() { return m_settings; }
	void camera(Square::Shared<Square::Scene::Actor> camera) { m_camera = camera; }

	//controls, held (set by the game)
	Hovercraft::Input& input() { return m_input; }

	//on the ground at x, z, still
	void spawn(float x, float z);
	float speed() const { return m_hovercraft ? m_hovercraft->speed() : 0.0f; }

	//events
	virtual void on_deattch() override;
	virtual void on_update(double delta_time) override;

	//serialize (no attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	Hovercraft::Settings                 m_settings;
	Hovercraft::Input                    m_input;
	Square::Shared<Square::Scene::Actor> m_camera;
	std::unique_ptr<Hovercraft>          m_hovercraft;
	//the hovercraft, made when the actor is in a world with a CollisionWorld
	bool ready();
};
