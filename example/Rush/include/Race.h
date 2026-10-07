//
//  Race.h
//  Rush
//
//  A race on a map (Arena), a component of the actor of the race in its level (it finds its level
//  through its actor; it runs, its phases, while the level runs). An arena: the light beam on its
//  checkpoints (who touches it scores, the beam goes to another checkpoint), the scores. A
//  circuit (its Course: laps, or from a start to a finish): its checkpoints passed in order (any
//  way between them), how far each racer is along its guide (the places: never past a checkpoint
//  not passed), back on the guide a little behind (asked, or fallen). The hovercraft: the player,
//  the first, and the NPCs. Its phases:
//   - START: the camera comes to the player, the hovercraft still (no controls);
//   - PLAY: the player drives (HovercraftInput), the NPCs too (HovercraftAI);
//   - END: a racer got the winning score (an arena), the player crossed the line of its last lap
//     (a circuit); the game goes on, the player's hovercraft an NPC too, the scores stopped (the
//     UI shows win or lose, the place).
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
		//a circuit: the lap it is on, the checkpoint it goes to, how far it is along the race
		//(world units along the guide, from the start), where it is on the guide, its finish
		//(when, seconds of the race), seconds going the wrong way
		int                                  m_lap{ 1 };
		size_t                               m_next_checkpoint{ 0 };
		float                                m_progress{ 0.0f };
		size_t                               m_segment{ 0 };
		bool                                 m_finished{ false };
		double                               m_finish_time{ 0.0 };
		float                                m_wrong_way{ 0.0f };
	};

	Race(Square::Context& context);
	~Race();

	//the map, the light beam, the hovercraft at their starts, in the level of its actor (false:
	//no map)
	bool load(const Rush::RaceMap& map);
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

	//a circuit (laps along its gates), else an arena
	bool circuit() const;
	int  laps() const;
	//the place of a racer (1: the first), the racers in their order (the leader first)
	size_t place(size_t id) const;
	const std::vector<size_t>& standings() const;
	//the player going the wrong way (a while against the track)
	bool wrong_way() const;
	//seconds of the race (from GO!)
	double race_time() const;
	//back on the track: a circuit on its guide a little behind where it is, an arena its start
	void respawn(size_t id);
	//the sun of the map on its steps (its time: the one of the race)
	void update_sun();
	//the haze where the player is on a circuit (its zone, blending into the next one); false:
	//not a circuit (the haze of the map)
	bool zone_fog(Rush::RaceFog& fog) const;


private:

	void load_trails();

	void load_wakes();
	//the navigation of the map (the AI goes around its obstacles)
	void load_navigation();
	//the meshes of a hovercraft lower (in the snow)
	void sink(Square::Shared<Square::Scene::Actor> hovercraft) const;
	void load_light_beam();
	void load_hovercraft();
	void reached(size_t checkpoint, Square::Shared<Square::Scene::Actor> who);
	//a circuit: the checkpoints passed, how far they are, the falls, the wrong ways, the places
	void update_circuit(double delta_time);
	void passed(size_t id);
	void rank();
	//how far along the race a checkpoint is on a lap (lap 1: from the start)
	float checkpoint_progress(size_t checkpoint, int lap) const;
	//how far along the race a racer is now (its guide, near where it was; never past its next
	//checkpoint)
	float progress_now(size_t id);
	void phase(Phase phase);
	//the controls: the player its keys (an NPC in END), the NPCs the AI
	void controls(Phase phase);

	Square::Shared<Square::Scene::Level> m_level;
	const Rush::RaceMap*                 m_map{ nullptr };
	std::unique_ptr<Arena>               m_arena;
	std::unique_ptr<SnowTrails>          m_trails; //the map has snow: the grooves of the hovercraft
	std::unique_ptr<SnowTrails>          m_wakes;  //the map has water: the wakes of the hovercraft on it
	Square::Shared<Square::Navigation::NavGrid> m_navigation; //where the AI can drive, its paths
	float                                m_sink{ 0.0f }; //the hovercraft shown lower (in the snow)
	Square::Shared<Square::Scene::Actor> m_light_beam;
	std::vector<Racer>                   m_racers; //the first: the player
	Square::Shared<Checkpoints>          m_checkpoints;
	Phase                                m_phase{ Phase::START };
	double                               m_phase_time{ 0.0 };
	double                               m_water_time{ 0.0 }; //seconds of the water (its waves, its falls)
	size_t                               m_winner{ 0 };
	bool                                 m_end{ false }; //a racer won: END at the next update
	//a circuit: seconds since GO!, who finished, the order of the racers
	double                               m_race_time{ 0.0 };
	size_t                               m_finished{ 0 };
	std::vector<size_t>                  m_standings;
};
