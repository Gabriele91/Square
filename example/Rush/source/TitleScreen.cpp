//
//  TitleScreen.cpp
//  Rush
//
//  See TitleScreen.h.
//
#include <cmath>
#include <TitleScreen.h>
#include <Turntable.h>
#include <Race.h>

namespace AuxTitle
{
	//the camera of the title: where it is and where it looks, from the hovercraft; it looks up
	//and right of it, so the hovercraft is at the bottom left of the screen
	const Square::Vec3 s_camera_offset{ -5.0f, 3.0f, -15.0f };
	const Square::Vec3 s_look_offset{ 3.8f, 0.4f, 0.0f };
	constexpr float    s_camera_fov = 0.55f; //radians, vertical

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
	//the hovercraft, red as the player
	m_hovercraft = level->load_actor("hovercraft/scene");
	if (m_hovercraft)
	{
		//from the root of the level to the one of the title
		level->remove(m_hovercraft);
		m_root->add(m_hovercraft);
		m_hovercraft->name("title_hovercraft");
		m_hovercraft->position(Vec3(0.0f));
		Race::paint(m_context, m_hovercraft, s_skins[0]);
		//turning while the title runs
		auto turntable = m_hovercraft->component<Turntable>();
		turntable->speed(s_title_turn_speed);
		turntable->yaw(30.0f);
	}
	setup_camera();
	setup_light();
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

void TitleScreen::setup_light()
{
	using namespace Square;
	//a fill light near the camera (the sun lights it too), no shadow
	auto fill = m_root->child();
	fill->name("title_light");
	fill->position(AuxTitle::s_camera_offset + Vec3(-2.0f, 3.0f, 0.0f));
	auto light = fill->component<Scene::PointLight>();
	light->diffuse({ 1.0f, 0.95f, 0.9f });
	light->specular({ 1.0f, 0.95f, 0.9f });
	light->constant(1.0f);
	light->radius(40.0f);
	light->inside_radius(10.0f);
}

void TitleScreen::viewport(unsigned int width, unsigned int height)
{
	using namespace Square;
	if (!m_camera || !width || !height) return;
	auto camera = m_camera->component<Scene::Camera>();
	camera->viewport({ 0, 0, width, height });
	camera->perspective(AuxTitle::s_camera_fov, float(width) / float(height), 0.1f, 200.0f);
}

void TitleScreen::show(bool show)
{
	using namespace Square;
	auto world = m_level ? m_level->world().lock() : nullptr;
	auto render_world = world ? world->instance<RenderInstance>() : nullptr;
	//the background: black in the title, the one of before back
	if (render_world && show)
	{
		m_clear_color = render_world->clear_color();
		render_world->clear_color(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}
	if (render_world && !show) render_world->clear_color(m_clear_color);
}
