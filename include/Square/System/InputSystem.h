//
//  InputSystem.h
//  Square
//
//  The input of the window (Video::Input): reads the events at the start of the frame
//  (update) and sends them to the application (AppInterface).
//
#pragma once
#include "Square/Config.h"
#include "Square/System/System.h"

namespace Square
{
	//..................
	//declaretion
	namespace Video
	{
		class Input;
	}
	//..................
	class SQUARE_API InputSystem : public System
	{
	public:
		//A square system, and its ring
		SQUARE_SYSTEM(InputSystem, SystemRing::DRIVER)

		//Registration in context
		static void object_registration(Context& ctx);

		//Init
		InputSystem(Context& context);
		virtual ~InputSystem();

		//System
		virtual bool initialize() override;
		virtual void shutdown() override;
		virtual void update(double delta_time) override;

		//the input of the window
		Video::Input* input() const;

		//the window was asked to close
		bool close_requested() const;

	protected:
		Video::Input* m_input{ nullptr };
		bool          m_close_requested{ false };
	};
}
