//
//  NavGrid.h
//  Square
//
//  Navigation for agents on the ground (vehicles, characters): a grid over a level, from above
//  (x/z, y up), built from the triangles of its geometry, and the paths on it.
//  Build:
//   1) floor: every walkable triangle (not steeper than max_slope) gives its height to the cells
//      whose center it covers; a cell keeps its lowest floor (the ground under a bridge, a roof);
//   2) obstacles: every triangle over the cells its footprint touches (a wall is a line from
//      above: a conservative test) that rises in the room of the agent over the floor of a cell
//      (from max_step to agent_height) blocks it;
//   3) clearance: the cells nearer than agent_radius to a blocked one (or to the border) are
//      blocked too: the agent is a point on what is left.
//  Paths: A* on the 8 neighbours of a cell (no corner cut, no step higher than max_step), then
//  smoothed: from each point the farthest one of the path in line of sight (string pulling),
//  so an agent goes straight where it can and turns around the obstacles.
//
#pragma once
#include <vector>
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/SmartPointers.h"

namespace Square
{
namespace Scene
{
	class Actor;
}
namespace Navigation
{
	class SQUARE_API NavGrid
	{
	public:

		struct Settings
		{
			float cell_size{ 1.0f };     //side of a cell (world units)
			float agent_radius{ 1.5f };  //the agent's clearance from the obstacles (world units)
			float agent_height{ 2.0f };  //room it needs over the floor (world units)
			float max_slope{ 35.0f };    //degrees: steeper is not floor
			float max_step{ 0.8f };      //a step it climbs (world units): between cells, under an obstacle
		};

		//the grid over min/max (x, z) of triangles (world space, 3 vertices each); false: nothing
		//walkable
		bool build(const std::vector<Vec3>& triangles, const Vec3& min, const Vec3& max, const Settings& settings);
		//the grid of the geometry of an actor and its children (the triangles of their static
		//meshes, world space; e.g. a navmesh made in an editor: the walkable area, the holes of
		//the obstacles), over its bounds; false: nothing walkable
		bool build(const Shared<Scene::Actor>& actor, const Settings& settings);
		void clear();

		const Settings& settings() const { return m_settings; }
		bool empty() const { return m_cells.empty(); }

		//a point on a walkable cell
		bool walkable(const Vec3& point) const;
		//the floor of the cell of a point (its y when it has none)
		float floor(const Vec3& point) const;
		//the walkable point nearest to a point (within max_distance); false: none
		bool nearest(const Vec3& point, Vec3& out, float max_distance = 16.0f) const;
		//a straight walk between two points on walkable cells (no step higher than max_step)
		bool line_of_sight(const Vec3& from, const Vec3& to) const;
		//a path from a point to another (both moved to the nearest walkable point), its points on
		//the floor, smoothed (the first: from); false: no path
		bool find_path(const Vec3& from, const Vec3& to, std::vector<Vec3>& path) const;

		//size of the grid (cells), a cell
		int width() const { return m_width; }
		int height() const { return m_height; }
		bool cell_walkable(int x, int z) const;
		Vec3 cell_center(int x, int z) const;

	private:

		struct Cell
		{
			float m_floor{ 0.0f };
			bool  m_has_floor{ false };
			bool  m_blocked{ false };
		};

		bool inside(int x, int z) const { return x >= 0 && z >= 0 && x < m_width && z < m_height; }
		int  index(int x, int z) const { return z * m_width + x; }
		void to_cell(const Vec3& point, int& x, int& z) const;
		//a step from a cell to a neighbour: both walkable, not a higher step than max_step
		bool can_step(int x0, int z0, int x1, int z1) const;
		void rasterize_floor(const Vec3& a, const Vec3& b, const Vec3& c);
		void rasterize_obstacle(const Vec3& a, const Vec3& b, const Vec3& c);
		void erode();

		Settings          m_settings;
		Vec3              m_min{ 0.0f };
		int               m_width{ 0 };
		int               m_height{ 0 };
		std::vector<Cell> m_cells;
	};
}
}
