//
//  HovercraftAI.h
//  Rush
//
//  An NPC at the controls of a hovercraft, as a component of the hovercraft actor (the bots
//  of Limit Rush, control_player_bots): always forward, it turns towards where it goes. With the
//  navigation of the map (a NavGrid) it makes a path to the current checkpoint (a line of points
//  around the obstacles) and follows it (pure pursuit): its nearest point on the line, then a
//  point lookahead further along it, where it steers (it keeps on the line, the corners rounded
//  inside the cleared ground); a new path when the checkpoint changes, every replan seconds, or
//  when it is off the line; in a sharp turn it lets the throttle go. Without, straight to the
//  checkpoint. Stuck (still while it drives) it backs up for a
//  while, its nose turning toward where it goes. It sets the Input of its HovercraftDriver every frame, as the
//  player does with HovercraftInput.
//
#pragma once
#include <random>
#include <vector>
#include <Square/Square.h>

class Checkpoints;
class Course;
class HovercraftDriver;

class HovercraftAI : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(HovercraftAI)

	struct Settings
	{
		float dead_zone{ 6.0f };    //degrees: nearer to the direction of the target it goes straight
		float lookahead{ 7.0f };    //world units: how far along the path ahead of it it steers
		float off_path{ 6.0f };     //world units: farther from its path, a new one
		float replan{ 3.0f };       //seconds between two paths to the same checkpoint
		float sharp_turn{ 60.0f };  //degrees: a turn sharper than this, it lets the throttle go
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
	//a circuit: it goes along the guide of its course, lane world units on the right of it (its
	//own line: they do not all queue on one), steering further ahead the faster; at the start
	//of one of its other ways it takes it by its courage (a share: 0 never, 1 always), at its
	//end back on the guide
	void follow(const Course& course, float lane, float courage);

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
	//where it goes on the circuit: ahead along its line (from its nearest point), in its lane
	Square::Vec3 line_target(const Square::Vec3& position, float ahead);
	//the way it takes: another one starting near it (by its courage), back on the guide at
	//the end of one
	void choose_way(const Square::Vec3& position, const Square::Vec3& forward);
	//the nearest segment of a line to a position
	size_t nearest_segment(size_t line, const Square::Vec3& position) const;

	Settings                                    m_settings;
	Square::Weak<Checkpoints>                   m_checkpoints;
	Square::Weak<Square::Navigation::NavGrid>   m_navigation;
	//the path to the checkpoint
	std::vector<Square::Vec3>                   m_path;
	size_t                                      m_segment{ 0 }; //the segment of the path it is on (only forward)
	size_t                                      m_path_checkpoint{ size_t(-1) };
	float                                       m_replan{ 0.0f };
	//the circuit: its lines (the guide, then the other ways), the one it is on, its segment
	//(searched near it: a line can pass near itself), its lane, its courage, the ways it chose
	//to take or not (again when far from them)
	struct Line
	{
		std::vector<Square::Vec3> m_points;
		bool                      m_closed{ false };
	};
	std::vector<Line>                           m_lines;
	size_t                                      m_line{ 0 };
	size_t                                      m_line_segment{ 0 };
	float                                       m_lane{ 0.0f };
	float                                       m_courage{ 0.0f };
	std::vector<bool>                           m_decided;
	std::mt19937                                m_random{ std::random_device{}() };
	//stuck: seconds still, seconds left backing up
	float                                       m_stuck{ 0.0f };
	float                                       m_reverse{ 0.0f };
};
