//
//  HovercraftAI.h
//  Rush
//
//  An NPC at the controls of a hovercraft, as a component of the hovercraft actor (the bots
//  of Limit Rush, control_player_bots): always forward, it turns towards where it goes. With the
//  navigation of the map (a NavGrid) it goes along a path to the current checkpoint, around the
//  obstacles (a new path when the checkpoint changes and every replan seconds), towards its next
//  point; without, straight to the checkpoint. Stuck (still while it drives) it backs up for a
//  while, its nose turning toward where it goes. It sets the Input of its HovercraftDriver every frame, as the
//  player does with HovercraftInput.
//
#pragma once
#include <vector>
#include <Square/Square.h>

class Checkpoints;
class HovercraftDriver;

class HovercraftAI : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(HovercraftAI)

	struct Settings
	{
		float dead_zone{ 6.0f };    //degrees: nearer to the direction of the target it goes straight
		float reach{ 5.0f };        //world units: nearer to a point of the path it goes to the next one
		float replan{ 1.0f };       //seconds between two paths to the same checkpoint
		float stuck_speed{ 0.1f };  //share of the top speed: slower while driving is stuck
		float stuck_time{ 1.0f };   //seconds stuck before it backs up
		float reverse_time{ 0.8f }; //seconds it backs up
	};

	//Registration in context
	static void object_registration(Square::Context& ctx);

	HovercraftAI(Square::Context& context);

	void settings(const Settings& settings) { m_settings = settings; }
	const Settings& settings() const { return m_settings; }

	//the race: it goes to the current checkpoint, on the navigation of the map (none: straight)
	void race(Square::Shared<Checkpoints> checkpoints, Square::Shared<Square::Navigation::NavGrid> navigation);

	//events
	virtual void on_update(double delta_time) override;

	//serialize (no attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

private:
	Square::Shared<HovercraftDriver> driver() const;
	//where it goes now: the next point of its path to the checkpoint (a new path if needed)
	Square::Vec3 target(const Square::Vec3& position, size_t checkpoint, const Square::Vec3& goal, double delta_time);

	Settings                                    m_settings;
	Square::Weak<Checkpoints>                   m_checkpoints;
	Square::Weak<Square::Navigation::NavGrid>   m_navigation;
	//the path to the checkpoint
	std::vector<Square::Vec3>                   m_path;
	size_t                                      m_next{ 0 };
	size_t                                      m_path_checkpoint{ size_t(-1) };
	float                                       m_replan{ 0.0f };
	//stuck: seconds still, seconds left backing up
	float                                       m_stuck{ 0.0f };
	float                                       m_reverse{ 0.0f };
};
