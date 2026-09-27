//
//  HovercraftInput.cpp
//  Rush
//
#include "HovercraftInput.h"
#include "Hovercraft.h"

using namespace Square;

SQUARE_CLASS_OBJECT_REGISTRATION(HovercraftInput);

void HovercraftInput::object_registration(Context& ctx)
{
	//factory: actor->component<HovercraftInput>()
	ctx.add_object<HovercraftInput>();
}

HovercraftInput::HovercraftInput(Context& context) : Component(context)
{
}

Shared<HovercraftDriver> HovercraftInput::driver() const
{
	auto hovercraft = actor().lock();
	return hovercraft && hovercraft->contains<HovercraftDriver>() ? hovercraft->component<HovercraftDriver>() : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
//a frame: the actions held become the controls of the driver
void HovercraftInput::on_update(double delta_time)
{
	auto input = System::get<InputSystem>(context());
	auto hovercraft = driver();
	if (!input || !hovercraft) return;
	HovercraftDriver::Input controls;
	controls.forward  = input->action(m_actions.forward);
	controls.backward = input->action(m_actions.backward);
	controls.left     = input->action(m_actions.left);
	controls.right    = input->action(m_actions.right);
	hovercraft->input(controls);
}

//////////////////////////////////////////////////////////////////////////////////////////
//serialize
void HovercraftInput::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void HovercraftInput::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void HovercraftInput::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void HovercraftInput::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }
