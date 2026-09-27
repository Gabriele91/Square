//
//  Checkpoints.h
//  Rush
//
//  The circuit of the light beam, as a component of the beam actor:
//  - the checkpoints are the "checkpoint_<n>" nodes of the arena, in the order of n;
//  - the beam stands on the ground under the current one, chosen at random (random_next);
//  - when the target (the hovercraft) touches it, the beam goes to another one at random, never
//    the same one again;
//  - the two parts of the beam, the inner and the outer cylinder, spin in opposite ways.
//
#pragma once
#include <Square/Square.h>
#include <functional>
#include <random>
#include <vector>

class Checkpoints : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(Checkpoints)

	struct Settings
	{
		float        radius{ 4.0f };               //touched when the target is this near (x/z) to the beam
		float        height{ 8.0f };               //and between the ground and this over it (the beam)
		std::string  inner{ "Cylinder01-0" };      //child of the beam, spins at inner_speed
		std::string  outer{ "Cylinder02-0" };      //child of the beam, spins at outer_speed
		float        inner_speed{ 360.0f * 3.0f }; //degrees per second around the up axis
		float        outer_speed{ -360.0f * 2.0f };//opposite way of the inner one
		std::string  prefix{ "checkpoint_" };      //name of the checkpoints in the arena
	};

	//Registration in context
	static void object_registration(Square::Context& ctx);

	Checkpoints(Square::Context& context);

	void settings(const Settings& settings) { m_settings = settings; }
	const Settings& settings() const { return m_settings; }

	//no checkpoint (current() before the first one)
	static constexpr size_t NONE = size_t(-1);

	//the checkpoints: the "<prefix><n>" nodes under root (world positions), in the order of n;
	//the beam goes to one of them at random
	size_t collect(Square::Shared<Square::Scene::Actor> root);
	//who touches them
	void target(Square::Shared<Square::Scene::Actor> target) { m_target = target; }
	//called when the target touches the current checkpoint (index of it), before the beam moves on
	void on_reached(std::function<void(size_t)> callback) { m_on_reached = std::move(callback); }

	//current checkpoint (the beam is there), NONE before the first one
	size_t current() const { return m_current; }
	size_t size() const { return m_points.size(); }
	//ground under a checkpoint
	const Square::Vec3& point(size_t index) const { return m_points[index % m_points.size()]; }
	//move the beam to a checkpoint
	void go_to(size_t index);

	//events
	virtual void on_update(double delta_time) override;

	//serialize (no attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	//the two parts of the beam, found by name once the beam has its children
	bool set_parts();
	void spin(float seconds);
	bool touched() const;
	//a checkpoint at random, not the current one (any of them while there is no current one);
	//NONE if there are no checkpoints
	size_t random_next() const;

	Settings                             m_settings;
	std::vector<Square::Vec3>            m_points;   //ground under every checkpoint
	size_t                               m_current{ NONE };
	mutable std::mt19937                 m_random{ std::random_device{}() };
	Square::Weak<Square::Scene::Actor>   m_target;
	std::function<void(size_t)>          m_on_reached;
	//spinning parts
	Square::Shared<Square::Scene::Actor> m_inner;
	Square::Shared<Square::Scene::Actor> m_outer;
	Square::Quat                         m_inner_rest;
	Square::Quat                         m_outer_rest;
	float                                m_time{ 0.0f }; //seconds of spinning
};
