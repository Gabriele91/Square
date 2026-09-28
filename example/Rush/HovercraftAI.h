//
//  HovercraftAI.h
//  Rush
//
//  An NPC at the controls of a hovercraft, as a component of the hovercraft actor (the bots
//  of Limit Rush, control_player_bots): always forward, it turns towards the current
//  checkpoint of the race. It sets the Input of its HovercraftDriver every frame, as the
//  player does with HovercraftInput.
//
#pragma once
#include <Square/Square.h>

class Checkpoints;
class HovercraftDriver;

class HovercraftAI : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(HovercraftAI)

	struct Settings
	{
		float dead_zone{ 6.0f };   //degrees: nearer to the direction of the target it goes straight
	};

	//Registration in context
	static void object_registration(Square::Context& ctx);

	HovercraftAI(Square::Context& context);

	void settings(const Settings& settings) { m_settings = settings; }
	const Settings& settings() const { return m_settings; }

	//the race: it goes to the current checkpoint
	void checkpoints(Square::Shared<Checkpoints> checkpoints) { m_checkpoints = checkpoints; }

	//events
	virtual void on_update(double delta_time) override;

	//serialize (no attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	Square::Shared<HovercraftDriver> driver() const;

	Settings                    m_settings;
	Square::Weak<Checkpoints>   m_checkpoints;
};
