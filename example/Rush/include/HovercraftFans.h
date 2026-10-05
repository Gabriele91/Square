//
//  HovercraftFans.h
//  Rush
//
//  The fans of a hovercraft spinning, as a component of the hovercraft actor: its nodes named
//  "fan_blades..." (the blades and the hub of a ducted fan, their origin at its center) turn
//  around the axis of the hull (z of the hovercraft), slow at idle, faster with the speed of its
//  HovercraftDriver (none, the title: idle).
//
#pragma once
#include <vector>
#include <Square/Square.h>

class HovercraftFans : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(HovercraftFans)

	struct Settings
	{
		float idle{ 6.0f };   //radians per second, still
		float full{ 45.0f };  //radians per second, at the top speed
	};

	//Registration in context
	static void object_registration(Square::Context& ctx);

	HovercraftFans(Square::Context& context);

	void settings(const Settings& settings) { m_settings = settings; }
	const Settings& settings() const { return m_settings; }

	//events
	virtual void on_update(double delta_time) override;

	//serialize (no attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	//the blades, their rotation in the model
	struct Fan
	{
		Square::Weak<Square::Scene::Actor> m_node;
		Square::Quat                       m_rest;
	};
	void find_fans();

	Settings         m_settings;
	std::vector<Fan> m_fans;
	bool             m_found{ false };
	float            m_angle{ 0.0f };
};
