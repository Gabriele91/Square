//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Window.h"
#include "Square/Driver/Input.h"
#include "Wrapper_private.h"

namespace Square
{
namespace Video
{
namespace Cocoa
{
    class SQUARE_API InputCocoa
    {
    public:
        WindowCocoa*                                                                      m_window_ref{ nullptr };
        Input*                                                                            m_input_ref{ nullptr };
        NSCocoaListener*                                                                  m_listener { nullptr };
        std::function<void(KeyboardEvent kevent, short mode, ActionEvent action)>         m_keyboard{ nullptr };
        std::function<void(int character, short mode, int plain)>                         m_character{ nullptr };
        std::function<void(MouseButtonEvent mevent, ActionEvent action)>                  m_mouse_button{ nullptr };
        std::function<void(double x, double y)>                                           m_mouse_move{ nullptr };
        std::function<void(double scroll)>                                                m_scroll{ nullptr };
        std::function<void(FingerEvent fevent, ActionEvent action)>                       m_touch{ nullptr };
        std::function<void(WindowEvent wevent)>                                           m_window{ nullptr };
        std::function<void(double x, double y)>                                           m_window_move{ nullptr };
        //save actions
        ActionEvent m_action_keyboard_state[KeyboardEvent::KEY_LAST];
        ActionEvent m_action_mouse_state[MouseButtonEvent::MOUSE_BUTTON_LAST];
        
        InputCocoa(WindowCocoa* window, Input* input);
        
        ~InputCocoa();
        
        void send_keyboard_event(KeyboardEvent kevent, short mode, ActionEvent action);
        
        void send_character_event(int character, short mode, bool plain);
        
        void send_mouse_button_event(MouseButtonEvent mevent, ActionEvent action);
        
        void send_mouse_move_event(double x, double y);
        
        void send_mouse_scroll_event(double scroll);
        
        void send_touch_event(FingerEvent fevent, ActionEvent action);
        
        void send_window_event(WindowEvent wevent);
        
        void send_window_move_event(double x, double y);
    };
}
}
}
