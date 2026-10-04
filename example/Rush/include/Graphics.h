//
//  Graphics.h
//  Rush
//
//  The rendering of the game: the pipeline of the world (SQUARE_RENDERING=forward|deferred,
//  default deferred) and its post effects with the settings of the game: SSAO (a light shade in
//  the creases), SSR (before the bloom: the reflected lights glow too), the fog of the map (off
//  but in a map with fog; before the bloom: the lights in the fog glow), Bloom.
//
#pragma once
#include <Square/Square.h>
#include <RushTypes.h>

class Graphics
{
public:

	Graphics(Square::Context& context);

	//the pipeline and the post effects of a world
	void setup(Square::Scene::World& world);

	//the fog of a map (its sun: where its light goes), off when the map has none
	void fog(const RaceFog& fog, const Square::Vec3& sun_direction);

	Square::Shared<Square::Render::SSAO>  ssao() const;
	Square::Shared<Square::Render::SSR>   ssr() const;
	Square::Shared<Square::Render::Bloom> bloom() const;
	Square::Shared<Square::Render::Fog>   fog() const;

private:

	Square::Context&                      m_context;
	Square::Shared<Square::Render::SSAO>  m_ssao;
	Square::Shared<Square::Render::SSR>   m_ssr;
	Square::Shared<Square::Render::Fog>   m_fog;
	Square::Shared<Square::Render::Bloom> m_bloom;
};
