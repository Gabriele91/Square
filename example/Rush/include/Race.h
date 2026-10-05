//
//  Race.h
//  Rush
//
//  A race on a map (Arena), a component of the actor of the race in its level (it finds its level
//  through its actor; it runs, its phases, while the level runs): the light beam on its checkpoints (who touches it scores, the beam
//  goes to another checkpoint), the hovercraft (the player, the first, and the NPCs), the
//  scores. Its phases:
//   - START: the camera comes to the player, the hovercraft still (no controls);
//   - PLAY: the player drives (HovercraftInput), the NPCs too (HovercraftAI);
//   - END: a racer got s_winning_score; the game goes on, the player's hovercraft an NPC too,
//     the scores stopped (the UI shows win or lose).
//
#pragma once
#include <memory>
#include <string>
#include <vector>
#include <Square/Square.h>
#include <RushTypes.h>
#include <Hovercraft.h>

class Arena;
class Checkpoints;
class SnowTrails;

class Race : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(Race)

	//Registration in context
	static void object_registration(Square::Context& ctx);

	enum class Phase
	{
		START,
		PLAY,
		END
	};

	//a hovercraft of the race
	struct Racer
	{
		std::string                          m_name;
		Square::Shared<Square::Scene::Actor> m_actor;
		Square::Shared<HovercraftDriver>     m_driver;
		int                                  m_score{ 0 };
	};

	Race(Square::Context& context);
	~Race();

	//the map, the light beam, the hovercraft at their starts, in the level of its actor (false:
	//no map)
	bool load(const RaceMap& map);
	//all out of the level
	void unload();

	//its phases, every frame (while its level runs)
	virtual void on_update(double delta_time) override;

	//serialize (no attributes)
	virtual void serialize(Square::Data::Archive& archive) override;
	virtual void serialize_json(Square::Data::JsonValue& archive) override;
	virtual void deserialize(Square::Data::Archive& archive) override;
	virtual void deserialize_json(Square::Data::JsonValue& archive) override;

	Phase  phase() const;
	double phase_time() const; //seconds in the phase
	size_t winner() const;     //the racer that won (END)

	//a hovercraft at its start, facing the middle of the level; the player's camera straight
	//behind it (a teleport)
	void spawn(size_t id, bool snap_camera = true);

	//a hovercraft of its own color: its materials its own (new objects of the same .mat), the
	//albedo of the skin texture
	void paint(size_t id);
	static void paint(Square::Context& context, Square::Shared<Square::Scene::Actor> hovercraft, const std::string& skin);

	const Arena&              arena() const;
	const std::vector<Racer>& racers() const;
	//the speed of the player, 0 to 100 (of its top speed)
	int player_speed() const;

private:

	void load_trails();
	//the meshes of a hovercraft lower (in the snow)
	void sink(Square::Shared<Square::Scene::Actor> hovercraft) const;
	void load_light_beam();
	void load_hovercraft();
	void reached(size_t checkpoint, Square::Shared<Square::Scene::Actor> who);
	void phase(Phase phase);
	//the controls: the player its keys (an NPC in END), the NPCs the AI
	void controls(Phase phase);
	static HovercraftDriver::Settings settings(size_t id);

	Square::Shared<Square::Scene::Level> m_level;
	std::unique_ptr<Arena>               m_arena;
	std::unique_ptr<SnowTrails>          m_trails; //the map has snow: the grooves of the hovercraft
	float                                m_sink{ 0.0f }; //the hovercraft shown lower (in the snow)
	Square::Shared<Square::Scene::Actor> m_light_beam;
	std::vector<Racer>                   m_racers; //the first: the player
	Square::Shared<Checkpoints>          m_checkpoints;
	Phase                                m_phase{ Phase::START };
	double                               m_phase_time{ 0.0 };
	size_t                               m_winner{ 0 };
	bool                                 m_end{ false }; //a racer won: END at the next update
};
