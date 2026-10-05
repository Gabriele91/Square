//
//  NavGrid.cpp
//  Square
//
//  See NavGrid.h for the high level description.
//
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include "Square/Navigation/NavGrid.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/StaticMesh.h"

namespace Square
{
namespace Navigation
{
	namespace AuxNavGrid
	{
		//the projections of the points of a shape on an axis (x/z)
		static void project(const Vec2* points, size_t count, const Vec2& axis, float& low, float& high)
		{
			low = high = dot(points[0], axis);
			for (size_t i = 1; i < count; ++i)
			{
				const float p = dot(points[i], axis);
				low = std::min(low, p);
				high = std::max(high, p);
			}
		}

		//a triangle (x/z) over a box (x/z): separating axes, the ones of the box and the normals of
		//the edges of the triangle (a triangle seen edge on, a wall, is a segment: still tested)
		static bool overlap(const Vec2 triangle[3], const Vec2& box_min, const Vec2& box_max)
		{
			const Vec2 box[4]{ box_min, Vec2(box_max.x, box_min.y), box_max, Vec2(box_min.x, box_max.y) };
			Vec2 axes[5]{ Vec2(1.0f, 0.0f), Vec2(0.0f, 1.0f) };
			size_t count = 2;
			for (size_t i = 0; i != 3; ++i)
			{
				const Vec2 edge = triangle[(i + 1) % 3] - triangle[i];
				if (length(edge) > 1e-6f) axes[count++] = Vec2(-edge.y, edge.x);
			}
			for (size_t i = 0; i != count; ++i)
			{
				float a0, a1, b0, b1;
				project(triangle, 3, axes[i], a0, a1);
				project(box, 4, axes[i], b0, b1);
				if (a1 < b0 || b1 < a0) return false;
			}
			return true;
		}

		//barycentric coordinates of a point in a triangle (x/z); false: degenerate
		static bool barycentric(const Vec2& p, const Vec2& a, const Vec2& b, const Vec2& c, Vec3& out)
		{
			const Vec2  v0 = b - a, v1 = c - a, v2 = p - a;
			const float d = v0.x * v1.y - v1.x * v0.y;
			if (std::abs(d) < 1e-9f) return false;
			const float v = (v2.x * v1.y - v1.x * v2.y) / d;
			const float w = (v0.x * v2.y - v2.x * v0.y) / d;
			out = Vec3(1.0f - v - w, v, w);
			return true;
		}

		//a cell of the open list of A*
		struct Open
		{
			float m_cost;
			int   m_index;
			bool operator < (const Open& other) const { return m_cost > other.m_cost; }
		};

		//8 neighbours: x, z, cost
		static const int   s_dx[8]{ 1, -1, 0, 0, 1, 1, -1, -1 };
		static const int   s_dz[8]{ 0, 0, 1, -1, 1, -1, 1, -1 };
		static const float s_cost[8]{ 1.0f, 1.0f, 1.0f, 1.0f, 1.41421356f, 1.41421356f, 1.41421356f, 1.41421356f };
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//build
	bool NavGrid::build(const std::vector<Vec3>& triangles, const Vec3& min, const Vec3& max, const Settings& settings)
	{
		clear();
		m_settings = settings;
		m_settings.cell_size = std::max(m_settings.cell_size, 0.05f);
		m_min = min;
		m_width = std::max(int(std::ceil((max.x - min.x) / m_settings.cell_size)), 1);
		m_height = std::max(int(std::ceil((max.z - min.z) / m_settings.cell_size)), 1);
		m_cells.assign(size_t(m_width) * size_t(m_height), Cell());
		//1) floor, 2) obstacles (over the floor), 3) clearance
		for (size_t i = 0; i + 2 < triangles.size(); i += 3) rasterize_floor(triangles[i], triangles[i + 1], triangles[i + 2]);
		for (Cell& cell : m_cells) cell.m_blocked = !cell.m_has_floor;
		for (size_t i = 0; i + 2 < triangles.size(); i += 3) rasterize_obstacle(triangles[i], triangles[i + 1], triangles[i + 2]);
		erode();
		return std::any_of(m_cells.begin(), m_cells.end(), [](const Cell& cell) { return !cell.m_blocked; });
	}

	bool NavGrid::build(const Shared<Scene::Actor>& actor, const Settings& settings)
	{
		clear();
		if (!actor) return false;
		std::vector<Vec3> triangles;
		actor->visit([&triangles](Shared<Scene::Actor> node) -> bool
		{
			if (node->contains<Scene::StaticMesh>()) node->component<Scene::StaticMesh>()->triangles(triangles);
			return true;
		});
		if (triangles.empty()) return false;
		//its bounds, a border (its edges inside the grid)
		Vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
		for (const Vec3& point : triangles)
		{
			min = glm::min(min, point);
			max = glm::max(max, point);
		}
		const float border = settings.agent_radius + settings.cell_size * 2.0f;
		return build(triangles, min - Vec3(border, 0.0f, border), max + Vec3(border, 0.0f, border), settings);
	}

	void NavGrid::clear()
	{
		m_cells.clear();
		m_width = m_height = 0;
	}

	void NavGrid::rasterize_floor(const Vec3& a, const Vec3& b, const Vec3& c)
	{
		using namespace AuxNavGrid;
		const Vec3 cross_product = cross(b - a, c - a);
		const float area = length(cross_product);
		if (area < 1e-9f) return;
		//walkable: not steeper than max_slope (either winding)
		if (std::abs(cross_product.y / area) < std::cos(radians(m_settings.max_slope))) return;
		int x0, z0, x1, z1;
		to_cell(glm::min(a, glm::min(b, c)), x0, z0);
		to_cell(glm::max(a, glm::max(b, c)), x1, z1);
		x0 = std::max(x0, 0); z0 = std::max(z0, 0);
		x1 = std::min(x1, m_width - 1); z1 = std::min(z1, m_height - 1);
		const Vec2 a2(a.x, a.z), b2(b.x, b.z), c2(c.x, c.z);
		for (int z = z0; z <= z1; ++z)
		{
			for (int x = x0; x <= x1; ++x)
			{
				const Vec3 center = cell_center(x, z);
				Vec3 weights;
				if (!barycentric(Vec2(center.x, center.z), a2, b2, c2, weights)) continue;
				const bool covered = weights.x >= -1e-4f && weights.y >= -1e-4f && weights.z >= -1e-4f;
				if (!covered) continue;
				//the lowest floor of the cell
				const float y = a.y * weights.x + b.y * weights.y + c.y * weights.z;
				Cell& cell = m_cells[size_t(index(x, z))];
				cell.m_floor = cell.m_has_floor ? std::min(cell.m_floor, y) : y;
				cell.m_has_floor = true;
			}
		}
	}

	void NavGrid::rasterize_obstacle(const Vec3& a, const Vec3& b, const Vec3& c)
	{
		using namespace AuxNavGrid;
		int x0, z0, x1, z1;
		to_cell(glm::min(a, glm::min(b, c)), x0, z0);
		to_cell(glm::max(a, glm::max(b, c)), x1, z1);
		x0 = std::max(x0, 0); z0 = std::max(z0, 0);
		x1 = std::min(x1, m_width - 1); z1 = std::min(z1, m_height - 1);
		const Vec2  footprint[3]{ Vec2(a.x, a.z), Vec2(b.x, b.z), Vec2(c.x, c.z) };
		const float low = std::min(a.y, std::min(b.y, c.y));
		const float high = std::max(a.y, std::max(b.y, c.y));
		//its plane (y from x, z) unless it stands up (a wall: its whole height)
		const Vec3  normal = cross(b - a, c - a);
		const bool  flat_enough = std::abs(normal.y) > 1e-3f * length(normal);
		const float half = m_settings.cell_size * 0.5f;
		for (int z = z0; z <= z1; ++z)
		{
			for (int x = x0; x <= x1; ++x)
			{
				Cell& cell = m_cells[size_t(index(x, z))];
				if (cell.m_blocked) continue;
				const Vec3 center = cell_center(x, z);
				const Vec2 box_min(center.x - half, center.z - half), box_max(center.x + half, center.z + half);
				if (!overlap(footprint, box_min, box_max)) continue;
				//its height over the cell (the plane at the corners, within the triangle)
				float y0 = low, y1 = high;
				if (flat_enough)
				{
					y0 = std::numeric_limits<float>::max();
					y1 = std::numeric_limits<float>::lowest();
					for (const Vec2& corner : { box_min, box_max, Vec2(box_min.x, box_max.y), Vec2(box_max.x, box_min.y) })
					{
						const float y = a.y - (normal.x * (corner.x - a.x) + normal.z * (corner.y - a.z)) / normal.y;
						y0 = std::min(y0, y);
						y1 = std::max(y1, y);
					}
					y0 = std::clamp(y0, low, high);
					y1 = std::clamp(y1, low, high);
				}
				//in the room of the agent over the floor: an obstacle
				const bool over_step = y1 > cell.m_floor + m_settings.max_step;
				const bool under_head = y0 < cell.m_floor + m_settings.agent_height;
				if (over_step && under_head) cell.m_blocked = true;
			}
		}
	}

	void NavGrid::erode()
	{
		//the distance (cells) of every cell from the nearest blocked one or the border: two passes
		//of a chamfer (1, sqrt 2)
		const float far = std::numeric_limits<float>::max() * 0.5f;
		std::vector<float> distance(m_cells.size(), far);
		for (int z = 0; z != m_height; ++z)
		{
			for (int x = 0; x != m_width; ++x)
			{
				const bool border = x == 0 || z == 0 || x == m_width - 1 || z == m_height - 1;
				if (border || m_cells[size_t(index(x, z))].m_blocked) distance[size_t(index(x, z))] = 0.0f;
			}
		}
		auto relax = [&](int x, int z, int dx, int dz, float cost)
		{
			if (!inside(x + dx, z + dz)) return;
			float& d = distance[size_t(index(x, z))];
			d = std::min(d, distance[size_t(index(x + dx, z + dz))] + cost);
		};
		for (int z = 0; z != m_height; ++z)
		{
			for (int x = 0; x != m_width; ++x)
			{
				relax(x, z, -1, 0, 1.0f); relax(x, z, 0, -1, 1.0f);
				relax(x, z, -1, -1, 1.41421356f); relax(x, z, 1, -1, 1.41421356f);
			}
		}
		for (int z = m_height - 1; z >= 0; --z)
		{
			for (int x = m_width - 1; x >= 0; --x)
			{
				relax(x, z, 1, 0, 1.0f); relax(x, z, 0, 1, 1.0f);
				relax(x, z, 1, 1, 1.41421356f); relax(x, z, -1, 1, 1.41421356f);
			}
		}
		const float clearance = m_settings.agent_radius / m_settings.cell_size;
		for (size_t i = 0; i != m_cells.size(); ++i)
		{
			if (distance[i] < clearance) m_cells[i].m_blocked = true;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//queries
	void NavGrid::to_cell(const Vec3& point, int& x, int& z) const
	{
		x = int(std::floor((point.x - m_min.x) / m_settings.cell_size));
		z = int(std::floor((point.z - m_min.z) / m_settings.cell_size));
	}

	Vec3 NavGrid::cell_center(int x, int z) const
	{
		const float floor = inside(x, z) && m_cells[size_t(index(x, z))].m_has_floor ? m_cells[size_t(index(x, z))].m_floor : m_min.y;
		return Vec3(m_min.x + (float(x) + 0.5f) * m_settings.cell_size, floor, m_min.z + (float(z) + 0.5f) * m_settings.cell_size);
	}

	bool NavGrid::cell_walkable(int x, int z) const
	{
		return inside(x, z) && !m_cells[size_t(index(x, z))].m_blocked;
	}

	bool NavGrid::walkable(const Vec3& point) const
	{
		int x, z;
		to_cell(point, x, z);
		return cell_walkable(x, z);
	}

	float NavGrid::floor(const Vec3& point) const
	{
		int x, z;
		to_cell(point, x, z);
		if (!inside(x, z) || !m_cells[size_t(index(x, z))].m_has_floor) return point.y;
		return m_cells[size_t(index(x, z))].m_floor;
	}

	bool NavGrid::can_step(int x0, int z0, int x1, int z1) const
	{
		if (!cell_walkable(x0, z0) || !cell_walkable(x1, z1)) return false;
		const float rise = std::abs(m_cells[size_t(index(x1, z1))].m_floor - m_cells[size_t(index(x0, z0))].m_floor);
		return rise <= m_settings.max_step;
	}

	bool NavGrid::nearest(const Vec3& point, Vec3& out, float max_distance) const
	{
		int px, pz;
		to_cell(point, px, pz);
		if (cell_walkable(px, pz))
		{
			out = Vec3(point.x, floor(point), point.z);
			return true;
		}
		//the walkable cell nearest to it in a window
		const int radius = int(std::ceil(max_distance / m_settings.cell_size));
		int best = -1;
		float best_distance = std::numeric_limits<float>::max();
		for (int z = pz - radius; z <= pz + radius; ++z)
		{
			for (int x = px - radius; x <= px + radius; ++x)
			{
				if (!cell_walkable(x, z)) continue;
				const float d = float((x - px) * (x - px) + (z - pz) * (z - pz));
				if (d >= best_distance) continue;
				best_distance = d;
				best = index(x, z);
			}
		}
		if (best < 0) return false;
		out = cell_center(best % m_width, best / m_width);
		return true;
	}

	bool NavGrid::line_of_sight(const Vec3& from, const Vec3& to) const
	{
		//samples every half cell: walkable, no step higher than max_step between them
		const Vec2  a(from.x, from.z), b(to.x, to.z);
		const float distance = length(b - a);
		const int   samples = std::max(int(std::ceil(distance / (m_settings.cell_size * 0.5f))), 1);
		int px, pz;
		to_cell(from, px, pz);
		if (!cell_walkable(px, pz)) return false;
		for (int i = 1; i <= samples; ++i)
		{
			const Vec2 p = a + (b - a) * (float(i) / float(samples));
			int x, z;
			to_cell(Vec3(p.x, 0.0f, p.y), x, z);
			if (x == px && z == pz) continue;
			if (!can_step(px, pz, x, z)) return false;
			//a diagonal move: not through the corner of a blocked cell
			if (x != px && z != pz && (!cell_walkable(x, pz) || !cell_walkable(px, z))) return false;
			px = x;
			pz = z;
		}
		return true;
	}

	bool NavGrid::find_path(const Vec3& from, const Vec3& to, std::vector<Vec3>& path) const
	{
		using namespace AuxNavGrid;
		path.clear();
		if (m_cells.empty()) return false;
		Vec3 start, goal;
		if (!nearest(from, start) || !nearest(to, goal)) return false;
		//straight there
		if (line_of_sight(start, goal))
		{
			path = { start, goal };
			return true;
		}
		int sx, sz, gx, gz;
		to_cell(start, sx, sz);
		to_cell(goal, gx, gz);
		const int start_index = index(sx, sz);
		const int goal_index = index(gx, gz);
		//A*: octile distance to the goal
		auto heuristic = [&](int x, int z)
		{
			const float dx = float(std::abs(x - gx)), dz = float(std::abs(z - gz));
			return std::max(dx, dz) + 0.41421356f * std::min(dx, dz);
		};
		std::vector<float> cost(m_cells.size(), std::numeric_limits<float>::max());
		std::vector<int>   parent(m_cells.size(), -1);
		std::vector<bool>  closed(m_cells.size(), false);
		std::priority_queue<Open> open;
		cost[size_t(start_index)] = 0.0f;
		open.push({ heuristic(sx, sz), start_index });
		bool found = false;
		while (!open.empty())
		{
			const int current = open.top().m_index;
			open.pop();
			if (closed[size_t(current)]) continue;
			closed[size_t(current)] = true;
			if (current == goal_index)
			{
				found = true;
				break;
			}
			const int cx = current % m_width, cz = current / m_width;
			for (int n = 0; n != 8; ++n)
			{
				const int nx = cx + s_dx[n], nz = cz + s_dz[n];
				if (!can_step(cx, cz, nx, nz)) continue;
				//a diagonal: not through the corner of a blocked cell
				const bool diagonal = s_dx[n] != 0 && s_dz[n] != 0;
				if (diagonal && (!cell_walkable(nx, cz) || !cell_walkable(cx, nz))) continue;
				const int   next = index(nx, nz);
				const float next_cost = cost[size_t(current)] + s_cost[n];
				if (closed[size_t(next)] || next_cost >= cost[size_t(next)]) continue;
				cost[size_t(next)] = next_cost;
				parent[size_t(next)] = current;
				open.push({ next_cost + heuristic(nx, nz), next });
			}
		}
		if (!found) return false;
		//the cells, goal to start
		std::vector<Vec3> cells;
		for (int i = goal_index; i != -1; i = parent[size_t(i)]) cells.push_back(cell_center(i % m_width, i / m_width));
		std::reverse(cells.begin(), cells.end());
		cells.front() = start;
		cells.back() = goal;
		//string pulling: from each point the farthest one in line of sight
		path.push_back(cells.front());
		size_t i = 0;
		while (i + 1 < cells.size())
		{
			size_t next = i + 1;
			for (size_t j = cells.size() - 1; j > i + 1; --j)
			{
				if (line_of_sight(cells[i], cells[j]))
				{
					next = j;
					break;
				}
			}
			path.push_back(cells[next]);
			i = next;
		}
		return true;
	}
}
}
