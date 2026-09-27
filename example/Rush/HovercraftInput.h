//
//  HovercraftInput.h
//  Rush
//
//  The player at the controls of a hovercraft, as a component of the hovercraft actor: the
//  keys (from the key events of the game) held down become the Input of its HovercraftDriver.
//  An NPC is another component that writes the same Input.
//
#pragma once
#include <Square/Square.h>

class HovercraftDriver;

class HovercraftInput : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(HovercraftInput)

	struct Bindings
	{
		Square::Video::KeyboardEvent forward{ Square::Video::KEY_UP };
		Square::Video::KeyboardEvent backward{ Square::Video::KEY_DOWN };
		Square::Video::KeyboardEvent left{ Square::Video::KEY_LEFT };
		Square::Video::KeyboardEvent right{ Square::Video::KEY_RIGHT };
	};

	//Registration in context
	static void object_registration(Square::Context& ctx);

	HovercraftInput(Square::Context& context);

	Bindings& bindings() { return m_bindings; }

	//a key event of the game: true if it is a control of the hovercraft
	bool key(Square::Video::KeyboardEvent key, Square::Video::ActionEvent action);
	//all the controls up (focus lost, respawn...)
	void release();

	//serialize (no attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	Square::Shared<HovercraftDriver> driver() const;

	Bindings m_bindings;
};
