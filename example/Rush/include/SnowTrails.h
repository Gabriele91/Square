//
//  SnowTrails.h
//  Rush
//
//  The trails of the hovercraft in the snow (a map with snow: RaceMap::m_trails): a map over the
//  field, from above (world x/z), of how deep the snow is pressed, drawn by the CPU. Every frame
//  each hovercraft on the ground presses a groove along its way (from where it was: as wide as
//  its hull, deepest in the middle, as a ball rolled in the snow); the snow fills the grooves
//  back slowly. The map is a texture (R8) given to the materials of the arena that read one
//  (Settings::map: "trail_map" of the PBRSnow effect), uploaded in the frames it changed.
//  The wakes on the water are one too (a map with water: RaceMap::m_wakes): "wake_map" of the
//  PBRWater effect, small, filled back in a few seconds (the waves of the hull fading).
//
#pragma once
#include <vector>
#include <Square/Square.h>

class SnowTrails
{
public:

	struct Settings
	{
		Square::Vec2 center{ 0.0f, 0.0f }; //of the map (world x, z)
		float        size{ 180.0f };       //side of the map (world units)
		int          resolution{ 2048 };    //texels of a side
		float        radius{ 1.3f };       //half the width of a groove (world units): a little narrower than the hull
		float        depth{ 1.0f };        //the middle of a groove, [0, 1]
		float        refill{ 45.0f };      //seconds the snow takes to fill the deepest groove back
		const char*  map{ "trail_map" };   //the parameters of the materials it is given to: the map,
		const char*  area{ "trail_area" }; //where it is (corner x, z; 1 / its size)
	};

	SnowTrails(Square::Context& context);

	//the map (empty), its texture (false: no texture)
	bool create(const Settings& settings);
	//the materials of an actor (and its children) that read a trail map: this one, where it is
	void attach(Square::Shared<Square::Scene::Actor> actor) const;

	//a hovercraft (id: its place in the race) where it is now: on the ground it presses the snow
	//from where it was (a jump, a respawn: no groove)
	void press(size_t id, const Square::Vec3& position, bool on_ground);
	//the snow filled back, the texture of the map uploaded if it changed
	void update(double delta_time);

private:

	//a groove from a point to another (world x, z)
	void groove(const Square::Vec2& from, const Square::Vec2& to);
	//the profile of a groove around a point (texels of the map)
	void stamp(const Square::Vec2& texel);
	Square::Vec2 to_texel(const Square::Vec2& point) const;

	struct Track
	{
		Square::Vec2 m_previous{ 0.0f };
		bool         m_on_ground{ false };
	};

	Square::Context&                           m_context;
	Settings                                   m_settings;
	std::vector<unsigned char>                 m_map;
	Square::Shared<Square::Resource::Texture>  m_texture;
	std::vector<Track>                         m_tracks;
	float                                      m_fill{ 0.0f }; //levels of the map to fill back (a fraction left)
	bool                                       m_changed{ false };
};
