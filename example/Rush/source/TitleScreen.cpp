//
//  TitleScreen.cpp
//  Rush
//
//  See TitleScreen.h.
//
#include <algorithm>
#include <cmath>
#include <TitleScreen.h>
#include <HovercraftFans.h>
#include <Race.h>

namespace AuxTitle
{
	//the camera of the title (as the shot of origial_assets/title: CAMERA, LOOK, FOV), from the
	//hovercraft of the player: low on the water, looking left of it (the hovercraft on the right
	//of the screen, the menu on the left)
	const Square::Vec3 s_camera_offset{ 5.0f, 1.4f, -14.0f };
	const Square::Vec3 s_look_offset{ -3.8f, 1.1f, 0.0f };
	constexpr float    s_camera_fov = 0.55f; //radians, vertical
	constexpr float    s_far = 2000.0f;      //the sky, the mountains far away
	//the camera breathing (as the menus of Halo Reach: a scene, not a picture): a slow loop of its
	//place (an ellipse: right and left, up and down; world units), where it looks still (the
	//hovercraft in place, the far plates shifting behind it: the depth); its period (seconds)
	constexpr float    s_drift_side = 0.18f;
	constexpr float    s_drift_up = 0.07f;
	constexpr float    s_drift_period = 24.0f;
	//the hovercraft: the player at the origin, heading to the left of the camera (degrees around
	//y), the racer behind (at "racer_1" of the scene)
	constexpr float    s_player_yaw = -116.0f;
	constexpr float    s_racer_yaw = -112.0f;

	//the rotation that looks along a direction (no roll; +z forward, as CameraFollow)
	Square::Quat look_rotation(const Square::Vec3& direction)
	{
		using namespace Square;
		const float horizontal = std::sqrt(direction.x * direction.x + direction.z * direction.z);
		const Quat yaw   = angle_axis(std::atan2(direction.x, direction.z), Constants::axis_y);
		const Quat pitch = angle_axis(-std::atan2(direction.y, horizontal), Constants::axis_x);
		return yaw * pitch;
	}
}

TitleScreen::TitleScreen(Square::Context& context)
: m_context(context)
{
}

void TitleScreen::load(Square::Shared<Square::Scene::Level> level)
{
	using namespace Square;
	m_level = level;
	m_root = level->actor();
	m_root->name("title");
	m_root->position(s_title_origin);
	//the shot: its scene under the root of the title
	m_scene = level->load_actor("title/scene");
	if (m_scene)
	{
		level->remove(m_scene);
		m_root->add(m_scene);
		m_scene->position(Vec3(0.0f));
	}
	else
	{
		m_context.logger()->warning("title: no scene (title/scene)");
	}
	//its sun: the shadow filtered (its distance: of the scene, near)
	auto sun = m_scene ? m_scene->child("sun") : nullptr;
	if (sun && sun->contains<Scene::DirectionLight>())
	{
		sun->component<Scene::DirectionLight>()->shadow_filter(Render::ShadowFilter::PCF);
	}
	//the hovercraft: the player (red), the racer behind (blue)
	m_riders.clear();
	hovercraft(s_skins[0], Vec3(0.0f), AuxTitle::s_player_yaw);
	auto racer = m_scene ? m_scene->child("racer_1") : nullptr;
	if (racer) hovercraft(s_skins[1], racer->position(), AuxTitle::s_racer_yaw);
	find_water();
	setup_camera();
}

Square::Shared<Square::Scene::Actor> TitleScreen::hovercraft(const std::string& skin, const Square::Vec3& position, float yaw)
{
	using namespace Square;
	auto actor = m_level->load_actor("hovercraft/scene");
	if (!actor) return nullptr;
	//from the root of the level to the one of the title
	m_level->remove(actor);
	m_root->add(actor);
	actor->name("title_hovercraft_" + std::to_string(m_riders.size()));
	actor->position(position);
	actor->rotation(angle_axis(radians(yaw), Constants::axis_y));
	Race::paint(m_context, actor, skin);
	//its fans at idle
	actor->component<HovercraftFans>();
	Rider rider;
	rider.m_actor = actor;
	rider.m_position = position;
	rider.m_yaw = yaw;
	rider.m_phase = float(m_riders.size()) * 2.1f;
	m_riders.push_back(rider);
	return actor;
}

void TitleScreen::find_water()
{
	using namespace Square;
	//the materials with a water_time (PBRWater), each once
	m_water.clear();
	if (!m_scene) return;
	m_scene->visit([this](Shared<Scene::Actor> node) -> bool
	{
		if (!node->contains<Scene::StaticMesh>()) return true;
		for (const auto& material : node->component<Scene::StaticMesh>()->m_materials)
		{
			if (!material || !material->parameter_by_name("water_time")) continue;
			if (std::find(m_water.begin(), m_water.end(), material) == m_water.end()) m_water.push_back(material);
		}
		return true;
	});
}

void TitleScreen::setup_camera()
{
	using namespace Square;
	m_camera = m_root->child();
	m_camera->name("title_camera");
	m_camera->position(AuxTitle::s_camera_offset);
	m_camera->rotation(AuxTitle::look_rotation(AuxTitle::s_look_offset - AuxTitle::s_camera_offset));
	unsigned int width = 0, height = 0;
	m_context.window()->get_size(width, height);
	viewport(width, height);
}

void TitleScreen::viewport(unsigned int width, unsigned int height)
{
	using namespace Square;
	if (!m_camera || !width || !height) return;
	auto camera = m_camera->component<Scene::Camera>();
	camera->viewport({ 0, 0, width, height });
	camera->perspective(AuxTitle::s_camera_fov, float(width) / float(height), 0.1f, AuxTitle::s_far);
}

void TitleScreen::update(double delta_time)
{
	using namespace Square;
	m_time += std::min(delta_time, 0.1);
	const float t = float(m_time);
	//the water
	const Vec4 seconds(float(std::fmod(m_time, 3600.0)), 0.0f, 0.0f, 0.0f);
	for (const auto& material : m_water)
	{
		if (auto parameter = material->parameter_by_name("water_time")) parameter->set(seconds);
	}
	//the camera: its loop (an ellipse across the view), looking at the same point
	if (m_camera)
	{
		const float w = 2.0f * Constants::pi<float>() * t / AuxTitle::s_drift_period;
		const Vec3 forward = normalize(AuxTitle::s_look_offset - AuxTitle::s_camera_offset);
		const Vec3 side = normalize(cross(Constants::axis_y, forward));
		const Vec3 position = AuxTitle::s_camera_offset
		                    + side * (AuxTitle::s_drift_side * std::sin(w))
		                    + Constants::axis_y * (AuxTitle::s_drift_up * std::cos(w));
		m_camera->position(position);
		m_camera->rotation(AuxTitle::look_rotation(AuxTitle::s_look_offset - position));
	}
}

Square::Vec3 TitleScreen::sun_direction() const
{
	using namespace Square;
	auto sun = m_scene ? m_scene->child("sun") : nullptr;
	if (!sun) return Vec3(0.0f, -1.0f, 0.0f);
	//a directional light shines along its -z
	return normalize(sun->rotation(true) * Vec3(0.0f, 0.0f, -1.0f));
}

void TitleScreen::show(bool show)
{
	using namespace Square;
	auto world = m_level ? m_level->world().lock() : nullptr;
	auto render_world = world ? world->instance<RenderInstance>() : nullptr;
	//the background: the dusk of the sky (where the dome does not reach), the one of before back
	if (render_world && show)
	{
		m_clear_color = render_world->clear_color();
		render_world->clear_color(Vec4(0.06f, 0.08f, 0.14f, 1.0f));
	}
	if (render_world && !show) render_world->clear_color(m_clear_color);
}
