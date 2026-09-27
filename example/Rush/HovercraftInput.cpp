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
//keys
bool HovercraftInput::key(Video::KeyboardEvent key, Video::ActionEvent action)
{
	bool* control = nullptr;
	auto  hovercraft = driver();
	if (!hovercraft) return false;
	auto& input = hovercraft->input();
	     if (key == m_bindings.forward)  control = &input.forward;
	else if (key == m_bindings.backward) control = &input.backward;
	else if (key == m_bindings.left)     control = &input.left;
	else if (key == m_bindings.right)    control = &input.right;
	if (!control) return false;
	//held: down on press, up on release (repeat changes nothing)
	if (action != Video::ActionEvent::REPEAT) *control = action == Video::ActionEvent::PRESS;
	return true;
}

void HovercraftInput::release()
{
	if (auto hovercraft = driver()) hovercraft->input() = HovercraftDriver::Input();
}

//////////////////////////////////////////////////////////////////////////////////////////
//serialize
void HovercraftInput::serialize(Data::Archive& archive)           { Data::serialize(archive, this); }
void HovercraftInput::serialize_json(Data::JsonValue& archive)    { Data::serialize_json(archive, this); }
void HovercraftInput::deserialize(Data::Archive& archive)         { Data::deserialize(archive, this); }
void HovercraftInput::deserialize_json(Data::JsonValue& archive)  { Data::deserialize_json(archive, this); }
