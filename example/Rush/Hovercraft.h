//
//  Hovercraft.h
//  Rush
//
//  Hovercraft driven like the Blitz3D "Autophysik" car, on the collisions of Blitz3D:
//  - the four wheels are spheres (radius `hover`) at the bottom corners of the hull; every
//    step each one moves, with the collisions (slide), from where it is to where the hull
//    and gravity bring it: they share the drive, each one has its own fall;
//  - the hull follows the wheels: aligned to them (left/right then front/rear) and centered
//    on them, so a wheel stopped by a wall or lifted by a step turns and tilts the hull;
//  - with a wheel on the ground (a contact facing up): throttle, drag, grip; in the air it
//    keeps its speed; the speed is what the hull really covered (a wall stops it, sliding
//    along it keeps a part), and a wheel landing hard takes some away;
//  - it steers faster the faster it goes, and at least idle_turn (also still);
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
		float hover{ 0.5f };           //radius of the wheels: gap between the hull and the ground
		float model_offset_y{ 0.0f };  //model up/down, share of the hull height (-0.25: a quarter lower)
		float floor_normal_y{ 0.5f };  //a contact with normal.y over it is ground (grip), else a wall
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

	//put the hull on the first surface under start (hovering), still
	void spawn(const Square::Vec3& start);
	//advance the simulation of dt seconds
	void update(double dt, const Input& input);

	float speed() const { return m_speed; }
	Settings& settings() { return m_settings; }

private:
	enum Corner { FRONT_LEFT, FRONT_RIGHT, BACK_LEFT, BACK_RIGHT };

	//one corner: a sphere that moves with the collisions, its own fall
	struct Wheel
	{
		Square::Vec3 m_position{ 0.0f }; //center, where the collisions left it
		float        m_fall{ 0.0f };     //vertical speed of its own (gravity), per step
		bool         m_ground{ false };  //it touched the ground in the last step
	};

	void step(const Input& input);
	//wheels at the corners of the hull, still
	void reset_wheels();
	void follow_camera();

	Square::Shared<Square::Scene::Actor> m_actor;
	Square::Shared<Square::Scene::Actor> m_camera;
	const CollisionMesh&                 m_scene;
	Settings                             m_settings;
	//hull, in its own space (from the vertices of its meshes)
	std::array<Square::Vec3, 4> m_corners;   //bottom corners: the centers of the wheels
	std::array<Wheel, 4>        m_wheels;
	//state
	Square::Vec3 m_velocity{ 0.0f };          //drive shared by the wheels, per step
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

	//on the first surface under start, still
	void spawn(const Square::Vec3& start);
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
