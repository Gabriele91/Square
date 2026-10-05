//
//  Graphics.cpp
//  Rush
//
//  See Graphics.h.
//
#include <cstdlib>
#include <Graphics.h>

Graphics::Graphics(Square::Context& context)
: m_context(context)
{
}

void Graphics::setup(Square::Scene::World& world)
{
	using namespace Square;
	auto render_world = world.instance<RenderInstance>();
	if (!render_world) return;
	//the pipeline
	const char* rendering_type = std::getenv("SQUARE_RENDERING");
	const bool  forward = rendering_type && case_insensitive_equal(rendering_type, "forward");
	render_world->pipeline((forward ? RP_FORWARD : RP_DEFERRED) | RP_DEBUG);
	//SSAO (deferred: it darkens the ambient light), softer than the defaults: a light shade in
	//the creases, not a dark halo
	m_ssao = MakeShared<Render::SSAO>(m_context);
	Render::SSAO::Settings ssao_settings;
	ssao_settings.radius          = 0.85f; //smaller creases
	ssao_settings.intensity       = 0.45f; //light occlusion
	ssao_settings.contrast        = 1.1f;  //linear: no extra darkening
	ssao_settings.max_pixels      = 32.0f; //near the camera: short reach, less cache misses
	ssao_settings.resolution      = Render::PER_QUARTER;
	ssao_settings.blur            = Render::SSAO::Settings::BLUR_LOW;
	m_ssao->settings(ssao_settings);
	render_world->add_post_effect(m_ssao);
	//screen space reflections (deferred): before the bloom, the reflected lights glow too; light:
	//a quarter of the frame (the reflections are blurred anyway), shorter rays, fewer steps
	m_ssr = MakeShared<Render::SSR>(m_context);
	Render::SSR::Settings ssr_settings;
	ssr_settings.resolution   = Render::PER_HALF;
	ssr_settings.max_distance = 60.0;
	ssr_settings.steps        = 40;
	ssr_settings.denoise      = false;
	ssr_settings.blur         = Render::SSR::Settings::BLUR_MEDIUM;
	m_ssr->settings(ssr_settings);
	render_world->add_post_effect(m_ssr);
	//the fog of the map (none in the menu)
	m_fog = MakeShared<Render::Fog>(m_context);
	m_fog->enabled(false);
	render_world->add_post_effect(m_fog);
	//the falling snow of the map (none in the menu)
	m_snow = MakeShared<Render::Snow>(m_context);
	Render::Snow::Settings snow_settings;
	//light: few small flakes in the world (they pass by with the parallax of the hovercraft)
	snow_settings.density      = 0.25f;
	snow_settings.spacing      = 1.8f;
	snow_settings.size         = 0.03f;
	snow_settings.speed        = 1.2f;
	snow_settings.max_distance = 28.0f;
	snow_settings.intensity    = 0.7f;
	m_snow->settings(snow_settings);
	m_snow->enabled(false);
	render_world->add_post_effect(m_snow);
	//bloom (forward and deferred): the lights and the emissive glow
	m_bloom = MakeShared<Render::Bloom>(m_context);
	Render::Bloom::Settings bloom_settings;
	bloom_settings.levels = 5; //the widest level (1/64 of the frame) is the least visible
	m_bloom->settings(bloom_settings);
	render_world->add_post_effect(m_bloom);
}

void Graphics::fog(const RaceFog& fog, const Square::Vec3& sun_direction)
{
	using namespace Square;
	if (!m_fog) return;
	m_fog->enabled(fog.m_on);
	Render::Fog::Settings settings;
	settings.color         = fog.m_color;
	settings.density       = fog.m_density;
	settings.height        = fog.m_height;
	settings.falloff       = fog.m_falloff;
	settings.max_opacity   = 0.92f; //the far lights still come through
	settings.sky           = 1.0f;
	settings.sun_color     = fog.m_sun;
	settings.sun_direction = sun_direction;
	settings.sun_exponent  = 6.0f;
	m_fog->settings(settings);
}

void Graphics::snow(bool snow)
{
	if (m_snow) m_snow->enabled(snow);
}

Square::Shared<Square::Render::SSAO> Graphics::ssao() const
{
	return m_ssao;
}

Square::Shared<Square::Render::SSR> Graphics::ssr() const
{
	return m_ssr;
}

Square::Shared<Square::Render::Bloom> Graphics::bloom() const
{
	return m_bloom;
}

Square::Shared<Square::Render::Fog> Graphics::fog() const
{
	return m_fog;
}

Square::Shared<Square::Render::Snow> Graphics::snow() const
{
	return m_snow;
}
