//
//  Graphics.h
//  Rush
//
//  The rendering of the game: the pipeline of the world (SQUARE_RENDERING=forward|deferred,
//  default deferred) and its post effects with the settings of the game: SSAO (a light shade in
//  the creases), SSR (before the bloom: the reflected lights glow too), the fog of the map (off
//  but in a map with fog; before the bloom: the lights in the fog glow), the falling snow of the
//  map (after the fog: the flakes in front of it), Bloom.
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
	//the falling snow of a map (shown if the weather is on: GameSettings)
	void snow(bool snow);
	void weather(bool weather);
	//the anti-aliasing (FXAA)
	void antialiasing(bool enable);
	//the depth of field (the title: sharp at focus world units, the far blurred)
	void depth_of_field(bool enable, float focus = 15.0f);

	Square::Shared<Square::Render::SSAO>  ssao() const;
	Square::Shared<Square::Render::SSR>   ssr() const;
	Square::Shared<Square::Render::Bloom> bloom() const;
	Square::Shared<Square::Render::Fog>   fog() const;
	Square::Shared<Square::Render::Snow>  snow() const;

private:

	Square::Context&                      m_context;
	Square::Shared<Square::Render::SSAO>  m_ssao;
	Square::Shared<Square::Render::SSR>   m_ssr;
	Square::Shared<Square::Render::Fog>   m_fog;
	Square::Shared<Square::Render::Snow>  m_snow;
	Square::Shared<Square::Render::Bloom> m_bloom;
	Square::Shared<Square::Render::DOF>   m_dof;
	Square::Shared<Square::Render::FXAA>  m_fxaa;
	bool                                  m_map_snow{ false };
	bool                                  m_weather{ true };
};
