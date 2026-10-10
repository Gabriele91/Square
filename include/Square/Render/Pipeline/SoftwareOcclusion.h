//
//  SoftwareOcclusion.h
//  Square
//
//  The occlusion culling of a camera on the CPU (as Godot 4 and the mobile renderer of Unreal):
//  each frame the occluders it sees (Render::Occluder: few triangles, simple shapes inside what
//  they stand for) are drawn on the CPU into a small depth buffer (its width: Settings::width, its
//  height by the aspect of the camera; the inverse of the depth along the view, the nearest kept),
//  then reduced into levels (each texel the farthest of the four under it). A box is hidden when,
//  over all the texels its rectangle on the screen covers (a level where they are a few), the
//  occluders are nearer than its nearest point. Nothing is late (the occluders of this frame), it
//  is the same on every backend; a camera that is not perspective hides nothing.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Math/Linear.h"
#include "Square/Geometry/AABoundingBox.h"
#include "Square/Geometry/Sphere.h"
#include <vector>

namespace Square
{
namespace Render
{
	class Camera;
	class Occluder;

	class SQUARE_API SoftwareOcclusion
	{
	public:

		struct Settings
		{
			bool enabled{ true };
			int  width{ 256 }; //pixels of the depth buffer across (its height by the aspect)
		};

		//what the last draw did
		struct Stats
		{
			size_t m_occluders{ 0 };  //drawn (in the frustum)
			size_t m_triangles{ 0 };  //of them, drawn
			size_t m_tested{ 0 };     //boxes tested (the renderables)
			size_t m_hidden{ 0 };     //of them, hidden
			size_t m_instances_tested{ 0 }; //spheres tested (the instances of the instanced meshes)
			size_t m_instances_hidden{ 0 }; //of them, hidden
		};

		void settings(const Settings& settings) { m_settings = settings; }
		const Settings& settings() const { return m_settings; }

		//the depth of the occluders a camera sees (none, disabled, not perspective: nothing hidden)
		void draw(const Camera& camera, const std::vector< Weak<Occluder> >& occluders);

		//a box (world space) hidden by the occluders of the last draw (counted: in the stats; a
		//debug view does not count its tests)
		bool hidden(const Geometry::AABoundingBox& box, bool counted = true) const;
		//a sphere (world space) hidden by the occluders of the last draw (an instance: counted in
		//the stats of the instances)
		bool hidden(const Geometry::Sphere& sphere) const;

		//it can hide something (the last draw had occluders)
		bool active() const { return m_active; }
		//a new number each draw (what was selected by an older one: again)
		uint64 version() const { return m_version; }
		const Stats& stats() const { return m_stats; }

		//the depth buffer (the inverse of the depth along the view: 0 nothing), its size
		const std::vector<float>& depth() const { return m_levels.empty() ? m_empty : m_levels[0]; }
		const IVec2& size() const { return m_size; }

	private:

		//a triangle in clip space (cut by the near plane), into the depth buffer
		void draw_triangle(const Vec4& a, const Vec4& b, const Vec4& c);
		//a triangle all in front of the near plane, into the depth buffer
		void raster_triangle(const Vec4& a, const Vec4& b, const Vec4& c);
		//the levels from the depth buffer (each texel the farthest of the four under it)
		void build_levels();

		Settings                         m_settings;
		mutable Stats                    m_stats;             //(the tests: by hidden)
		bool                             m_active{ false };
		uint64                           m_version{ 0 };
		Mat4                             m_view_projection{ 1.0f };
		float                            m_near{ 0.0f };      //the depth along the view of the near plane
		IVec2                            m_size{ 0, 0 };
		std::vector< std::vector<float> > m_levels;           //0: the depth buffer
		std::vector< IVec2 >             m_level_sizes;
		std::vector<float>               m_empty;
	};
}
}
