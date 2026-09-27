//
//  HovercraftInput.h
//  Rush
//
//  The player at the controls of a hovercraft, as a component of the hovercraft actor: every
//  frame it reads the actions of the InputSystem (the keys are bound to them by the game) and
//  sets them as the Input of its HovercraftDriver.
//  An NPC is another component that sets the Input of the driver from its logic.
//
#pragma once
#include <Square/Square.h>
#include <string>

class HovercraftDriver;

class HovercraftInput : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(HovercraftInput)

	//the actions of the InputSystem for the controls
	struct Actions
	{
		std::string forward{ "forward" };
		std::string backward{ "backward" };
		std::string left{ "left" };
		std::string right{ "right" };
	};

	//Registration in context
	static void object_registration(Square::Context& ctx);

	HovercraftInput(Square::Context& context);

	void actions(const Actions& actions) { m_actions = actions; }
	const Actions& actions() const { return m_actions; }

	//events
	virtual void on_update(double delta_time) override;

	//serialize (no attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	Square::Shared<HovercraftDriver> driver() const;

	Actions m_actions;
};
