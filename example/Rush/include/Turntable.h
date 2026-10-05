//
//  Turntable.h
//  Rush
//
//  An actor turning around its up axis (y), degrees per second: the hovercraft of the title. A
//  component: it turns while its level runs.
//
#pragma once
#include <Square/Square.h>

class Turntable : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(Turntable)

	//Registration in context
	static void object_registration(Square::Context& ctx);

	Turntable(Square::Context& context);

	//degrees per second, the angle now
	void speed(float speed) { m_speed = speed; }
	float speed() const { return m_speed; }
	void yaw(float yaw);
	float yaw() const { return m_yaw; }

	//events
	virtual void on_update(double delta_time) override;

	//serialize (speed)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	float m_speed{ 20.0f };
	float m_yaw{ 0.0f };
};
