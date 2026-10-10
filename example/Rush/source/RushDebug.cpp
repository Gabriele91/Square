//
//  RushDebug.cpp
//  Rush
//
//  See RushDebug.h.
//
#include <fstream>
#include <RushDebug.h>
#include <Race.h>
#include <Collision.h>

namespace AuxRushDebug
{
	//the key of its sections in the debug panel
	static const char* s_tab_key = "rush_tab";
}

RushDebug::RushDebug(Square::Context& context, Square::Scene::World& world, std::function<Race*()> race)
: m_context(context)
, m_world(world)
, m_race(race)
{
	using namespace Square;
	if (auto* ui_system = System::get<UISystem>(m_context))
	{
		//its tab: the collisions, the mirror, the level
		ui_system->debug_sections("Rush", AuxRushDebug::s_tab_key, [this](std::vector<DebugSection>& sections)
		{
			DebugSection draw{ "Debug draw", {} };
			draw.m_options.push_back(DebugOption::toggle("Collisions",
				[this]() { auto collision = m_world.instance<CollisionWorld>(); return collision && collision->debug(); },
				[this](bool value) { if (auto collision = m_world.instance<CollisionWorld>()) collision->debug(value); }));
			sections.push_back(draw);
			DebugSection race{ "Race", {} };
			race.m_options.push_back(DebugOption::toggle("Mirror hovercraft",
				[this]() { return m_mirror && m_race(); },
				[this](bool value) { mirror(value); }));
			sections.push_back(race);
			DebugSection level{ "Level", {} };
			level.m_options.push_back(DebugOption::button("Save", [this]() { save_level(false); }));
			level.m_options.push_back(DebugOption::button("Load", [this]() { load_level(false); }));
			level.m_options.push_back(DebugOption::button("Save json", [this]() { save_level(true); }));
			level.m_options.push_back(DebugOption::button("Load json", [this]() { load_level(true); }));
			sections.push_back(level);
		});
	}
}

RushDebug::~RushDebug()
{
	using namespace Square;
	if (auto* ui_system = System::get<UISystem>(m_context))
	{
		ui_system->remove_debug_sections(AuxRushDebug::s_tab_key);
	}
}

void RushDebug::race_ended()
{
	m_mirror = false;
	m_kept.clear();
}

void RushDebug::mirror(bool enable)
{
	using namespace Square;
	Race* race = m_race();
	const bool player = race && !race->racers().empty() && race->racers()[0].m_actor;
	if (player && enable && !m_mirror)
	{
		//chrome: a white metal (the albedo of a metal is its reflected color), perfectly smooth;
		//each material a copy of its own (the ones of the others untouched), the old one kept
		auto white = m_context.resource<Resource::Texture>("white");
		race->racers()[0].m_actor->visit([&](Shared<Scene::Actor> node) -> bool
		{
			if (node->contains<Scene::StaticMesh>())
			{
				auto mesh = node->component<Scene::StaticMesh>();
				for (size_t i = 0; i != mesh->m_materials.size(); ++i)
				{
					const Shared<Resource::Material>& material = mesh->m_materials[i];
					auto own = material ? DynamicPointerCast<Resource::Material>(m_context.resource_instance(material->resource_name())) : nullptr;
					if (own)
					{
						if (auto p = own->parameter_by_name("albedo_map"))    p->set(white);
						if (auto p = own->parameter_by_name("metallic_map"))  p->set(white);
						if (auto p = own->parameter_by_name("roughness_map")) p->set(white);
						if (auto p = own->parameter_by_name("color"))         p->set(Vec4(0.95f, 0.95f, 0.95f, 1.0f));
						if (auto p = own->parameter_by_name("metallic"))      p->set(1.0f);
						if (auto p = own->parameter_by_name("roughness"))     p->set(0.0f);
						m_kept.push_back({ mesh, i, material });
						mesh->m_materials[i] = own;
					}
				}
			}
			return true;
		});
		m_mirror = true;
	}
	else if (!enable && m_mirror)
	{
		//back: the materials it had
		for (const Kept& kept : m_kept)
		{
			auto mesh = kept.m_mesh.lock();
			if (mesh && kept.m_index < mesh->m_materials.size())
			{
				mesh->m_materials[kept.m_index] = kept.m_material;
			}
		}
		m_kept.clear();
		m_mirror = false;
	}
}

std::string RushDebug::level_path(bool json)
{
	return Square::Filesystem::join(Square::Filesystem::resource_dir(), json ? "level.jsq" : "level.sq");
}

void RushDebug::save_level(bool json)
{
	using namespace Square;
	using namespace Square::Data;
	using namespace Square::Filesystem::Stream;
	if (json)
	{
		Json jout = Json(JsonObject());
		m_world.serialize_json(jout);
		std::ofstream(level_path(true)) << jout;
	}
	else
	{
		GZOStream ofile(level_path(false));
		ArchiveBinWrite out(m_context, ofile);
		m_world.serialize(out);
	}
}

void RushDebug::load_level(bool json)
{
	using namespace Square;
	using namespace Square::Data;
	using namespace Square::Filesystem::Stream;
	const std::string path = level_path(json);
	if (Filesystem::exists(path))
	{
		if (json)
		{
			Json jin;
			if (jin.parser(Filesystem::text_file_read_all(path)))
			{
				m_world.deserialize_json(jin);
			}
		}
		else
		{
			GZIStream ifile(path);
			ArchiveBinRead in(m_context, ifile);
			m_world.deserialize(in);
		}
	}
}
