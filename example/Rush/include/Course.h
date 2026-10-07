//
//  Course.h
//  Rush
//
//  The course of a circuit (a race of laps, or from a start to a finish), from the nodes of its
//  map (made in Blender):
//  - "guide_<n>": the guide, a line along the way of the race in the order of n (a loop when its
//    two ends meet): how far a racer is (its places), the wrong way, where a racer is put back
//    (asked, or fallen), the line of the AI;
//  - "cp_<n>_<radius>": the checkpoints, in the order of n: passed going past their line (across
//    the guide) nearer than their radius to them. Large: every way of a stretch (the paths, the
//    shortcuts, the water) goes through them. A loop: cp_1 is the line of the start (and of the
//    finish, at the last lap); a start to a finish: the last one is the finish;
//  - "route_<name>_<n>": the other ways (a shortcut, the water, the safer one), lines from a
//    point near the guide to another one near it, in the order of n: the AI takes them or not.
//  The way of the race is a net of possibilities between its checkpoints, not a single line.
//
#pragma once
#include <string>
#include <vector>
#include <Square/Square.h>

class Course
{
public:

	//a checkpoint: where it is, its radius, where it is along the guide (world units), the
	//direction of the race there (x/z, unit)
	struct Checkpoint
	{
		Square::Vec3 m_position{ 0.0f };
		float        m_radius{ 0.0f };
		float        m_along{ 0.0f };
		Square::Vec3 m_direction{ 0.0f, 0.0f, 1.0f };
	};

	//the course from the nodes under root (false: no guide, no checkpoints: not a circuit)
	bool collect(Square::Shared<Square::Scene::Actor> root);
	bool valid() const { return m_guide.size() >= 2 && !m_checkpoints.empty(); }

	//a loop (laps), else from a start to a finish
	bool closed() const { return m_closed; }
	//the length of the guide (a loop: of a lap)
	float length() const { return m_length; }

	const std::vector<Square::Vec3>&              guide() const { return m_guide; }
	const std::vector<Checkpoint>&                checkpoints() const { return m_checkpoints; }
	const std::vector<std::vector<Square::Vec3>>& routes() const { return m_routes; }

	//the nearest point of the guide to a position (x/z): where it is along the guide; segment:
	//where to look from (around it: the guide can pass near itself), then the one found;
	//anywhere: the whole guide
	float project(const Square::Vec3& position, size_t& segment, bool anywhere = false) const;
	//the point of the guide at a distance along it, its direction (x/z, unit)
	Square::Vec3 point(float along) const;
	Square::Vec3 direction(float along) const;
	//past the line of a checkpoint (across the guide), nearer than its radius
	bool crossed(size_t checkpoint, const Square::Vec3& position) const;

private:

	float wrap(float along) const;
	size_t segment_at(float along) const;

	std::vector<Square::Vec3>              m_guide;
	std::vector<float>                     m_along;  //where each point of the guide is along it
	std::vector<Checkpoint>                m_checkpoints;
	std::vector<std::vector<Square::Vec3>> m_routes;
	bool                                   m_closed{ false };
	float                                  m_length{ 0.0f };
};
