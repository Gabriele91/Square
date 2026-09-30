//
//  HovercraftAI.cpp
//  Rush
//
#include <HovercraftAI.h>
#include <Hovercraft.h>
#include <Checkpoints.h>
#include <cmath>

using namespace Square;

SQUARE_CLASS_OBJECT_REGISTRATION(HovercraftAI);

void HovercraftAI::object_registration(Context& ctx)
{
	//factory: actor->component<HovercraftAI>()
	ctx.add_object<HovercraftAI>();
}

HovercraftAI::HovercraftAI(Context& context) : Component(context)
{
}

Shared<HovercraftDriver> HovercraftAI::driver() const
{
	auto hovercraft = actor().lock();
	return hovercraft && hovercraft->contains<HovercraftDriver>() ? hovercraft->component<HovercraftDriver>() : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
//a frame: towards the checkpoint
void HovercraftAI::on_update(double delta_time)
{
	auto hovercraft = actor().lock();
	auto hovercraft_driver = driver();
	auto race = m_checkpoints.lock();
	if (!hovercraft || !hovercraft_driver) return;
	HovercraftDriver::Input controls;
	//no race (no checkpoints): still
	if (race && race->size() && race->current() != Checkpoints::NONE)
	{
		//angle between the forward of the hovercraft and the target, on x/z (degrees, positive:
		//the target is on the right, +x)
		const Vec3  position = hovercraft->position(true);
		const Vec3  forward  = hovercraft->rotation(true) * Constants::axis_z;
		const Vec3  to_target = race->point(race->current()) - position;
		float angle = degrees(std::atan2(to_target.x, to_target.z) - std::atan2(forward.x, forward.z));
		while (angle >  180.0f) angle -= 360.0f;
		while (angle < -180.0f) angle += 360.0f;
		//always forward; atanfull(...) - yAng: left or right, straight within the dead zone
		controls.forward = true;
		controls.right   = angle >  m_settings.dead_zone;
		controls.left    = angle < -m_settings.dead_zone;
	}
	hovercraft_driver->input(controls);
}

//////////////////////////////////////////////////////////////////////////////////////////
//serialize
void HovercraftAI::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void HovercraftAI::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void HovercraftAI::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void HovercraftAI::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }
