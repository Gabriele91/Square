//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Wrapper_private.h"
#include "Window_private.h"
#include "Input_private.h"

//////////////////////////////////////////////////////////////////////////////
//translate
static int cocoa_translate_flags(NSUInteger flags)
{
    int mods = 0;
    
    if (flags & NSEventModifierFlagShift)
        mods |= Square::Video::KeyboardModEvent::SHIFT;
    if (flags & NSEventModifierFlagControl)
        mods |= Square::Video::KeyboardModEvent::CONTROL;
    if (flags & NSEventModifierFlagOption)
        mods |= Square::Video::KeyboardModEvent::ALT;
    if (flags & NSEventModifierFlagCommand)
        mods |= Square::Video::KeyboardModEvent::SUPER;
    
    return mods;
}
//translate
static int cocoa_translate_key(unsigned int key)
{
    if (key >= sizeof(Square::Video::Cocoa::s_os_context.m_key_map.m_keyboard) /
        sizeof(Square::Video::Cocoa::s_os_context.m_key_map.m_keyboard[0]))
        return Square::Video::KeyboardEvent::KEY_UNKNOWN;
    return Square::Video::Cocoa::s_os_context.m_key_map.m_keyboard[key];
}
//////////////////////////////////////////////////////////////////////////////
// Listener
@implementation NSCocoaListener

- (id) init
{
    self=[super init];
    //to null
    m_input = nullptr;
    m_window= nullptr;
    //return
    return self;
}

-(void) listen
:(Square::Video::Cocoa::InputCocoa *) input
:(NSWindow*) window
{
    //ref
    m_input   = input;
    m_window  = window;
    //
    NSView *view = [m_window contentView];
    [m_window setDelegate:self];
    [m_window setNextResponder:self];
    [m_window setAcceptsMouseMovedEvents:YES];
    [view   setNextResponder:self];
    
}
- (void)close
{
    //remove
    NSView *view = [m_window contentView];
    if([m_window delegate] == self)      [m_window setDelegate:nil];
    if([m_window nextResponder] == self) [m_window setNextResponder:nil];
    if([view nextResponder] == self)     [view setNextResponder:nil];
    //ref
    m_input  = nullptr;
    m_window = nullptr;
}
/* Window delegate functionality */
-(BOOL) windowShouldClose:(id) sender
{
    m_input->send_window_event(Square::Video::WindowEvent::CLOSE);
    return NO;
}
-(void) windowDidResize:(NSNotification *) notification
{
    m_input->send_window_event(Square::Video::WindowEvent::RESIZE);
}
/* Window event handling */
-(void) mouseDown:(NSEvent *) event
{
    m_input->send_mouse_button_event(Square::Video::MouseButtonEvent::MOUSE_BUTTON_LEFT,
                                     Square::Video::ActionEvent::PRESS);
}
-(void) rightMouseDown:(NSEvent *) event
{
    m_input->send_mouse_button_event(Square::Video::MouseButtonEvent::MOUSE_BUTTON_RIGHT,
                                     Square::Video::ActionEvent::PRESS);
}
-(void) otherMouseDown:(NSEvent *) event
{
    m_input->send_mouse_button_event((Square::Video::MouseButtonEvent)[event buttonNumber],
                                     Square::Video::ActionEvent::PRESS);
}
-(void) mouseUp:(NSEvent *) event
{
    m_input->send_mouse_button_event(Square::Video::MouseButtonEvent::MOUSE_BUTTON_LEFT,
                                     Square::Video::ActionEvent::RELEASE);
    
}
-(void) rightMouseUp:(NSEvent *) event
{
    m_input->send_mouse_button_event(Square::Video::MouseButtonEvent::MOUSE_BUTTON_RIGHT,
                                     Square::Video::ActionEvent::RELEASE);
}
-(void) otherMouseUp:(NSEvent *) event
{
    m_input->send_mouse_button_event((Square::Video::MouseButtonEvent)[event buttonNumber],
                                     Square::Video::ActionEvent::RELEASE);
}
-(void) mouseEntered:(NSEvent *) event
{
    //none
}
-(void) mouseExited:(NSEvent *) event
{
    //none
}
-(void) mouseMoved:(NSEvent *) event
{
    const NSRect  content = [[m_window contentView] frame];
    const NSPoint point   = [event locationInWindow];
    m_input->send_mouse_move_event(point.x, content.size.height - point.y);
}
-(void) mouseDragged:(NSEvent *) event
{
    //none
}
-(void) rightMouseDragged:(NSEvent *) event
{
    //none
}
-(void) otherMouseDragged:(NSEvent *) event
{
    //none
}
-(void) scrollWheel:(NSEvent *) event
{
    double delta_x = [event scrollingDeltaX];
    double delta_y = [event scrollingDeltaY];
    if ([event hasPreciseScrollingDeltas])
    {
        delta_x *= 0.1;
        delta_y *= 0.1;
    }
    if (std::abs(delta_x) > 0.0 || std::abs(delta_y) > 0.0)
    {
        m_input->send_mouse_scroll_event(delta_y); //no delta x
    }
}

- (void)keyDown:(NSEvent *)event
{
    //get
    const int   key  = cocoa_translate_key([event keyCode]);
    const short mode = cocoa_translate_flags([event modifierFlags]);
    //send
    m_input->send_keyboard_event((Square::Video::KeyboardEvent)key, mode, Square::Video::ActionEvent::PRESS);
    //resend (translate in char)
    [self interpretKeyEvents:[NSArray arrayWithObject:event]];
}

- (void)keyUp:(NSEvent *)event
{
    //get
    const int   key  = cocoa_translate_key([event keyCode]);
    const short mode = cocoa_translate_flags([event modifierFlags]);
    //send
    m_input->send_keyboard_event((Square::Video::KeyboardEvent)key, mode, Square::Video::ActionEvent::RELEASE);
}

@end

namespace Square
{
namespace Video
{
namespace Cocoa
{
    InputCocoa::InputCocoa(WindowCocoa* window, Input* input)
    {
        //Safe ref
        m_window_ref = window;
        m_input_ref  = input;
        //init array
        std::memset(m_action_keyboard_state, ActionEvent::RELEASE,sizeof(m_action_keyboard_state)/sizeof(m_action_keyboard_state[0]));
        std::memset(m_action_mouse_state, ActionEvent::RELEASE, sizeof(m_action_mouse_state)/sizeof(m_action_mouse_state[0]));
        //init cocoa listener
        m_listener = [[NSCocoaListener alloc] init];
        [m_listener listen:this :window->m_window];
    }
    
    InputCocoa::~InputCocoa()
    {
        //close
        [m_listener close];
        //to null
        m_window_ref = nullptr;
        m_input_ref  = nullptr;
        m_listener   = nullptr;
    }
    
    void InputCocoa::send_keyboard_event(KeyboardEvent kevent, short mode, ActionEvent action)
    {
        if (action ==  m_action_keyboard_state[kevent] && action == ActionEvent::PRESS)
        {
            m_action_keyboard_state[kevent] = ActionEvent::REPEAT;
        }
        else
        {
            m_action_keyboard_state[kevent] = action;
        }
        if (m_keyboard) m_keyboard(kevent, mode, m_action_keyboard_state[kevent]);
    }
    
    void InputCocoa::send_character_event(int character, short mode, bool plain)
    {
        if (m_character) m_character(character, mode, plain);
    }
    
    void InputCocoa::send_mouse_button_event(MouseButtonEvent mevent, ActionEvent action)
    {
        if (action ==  m_action_mouse_state[mevent] && action == ActionEvent::PRESS)
        {
            m_action_mouse_state[mevent] = ActionEvent::REPEAT;
        }
        else
        {
            m_action_mouse_state[mevent] = action;
        }
        if (m_mouse_button) m_mouse_button(mevent, m_action_mouse_state[mevent]);
    }
    
    void InputCocoa::send_mouse_move_event(double x, double y)
    {
        if (m_mouse_move) m_mouse_move(x, y);
    }
    
    void InputCocoa::send_mouse_scroll_event(double scroll)
    {
        if (m_scroll) m_scroll(scroll);
    }
    void InputCocoa::send_touch_event(FingerEvent fevent, ActionEvent action)
    {
        if (m_touch) m_touch(fevent, action);
    }
    
    void InputCocoa::send_window_event(WindowEvent wevent)
    {
        if (m_window) m_window(wevent);
    }
    
    void InputCocoa::send_window_move_event(double x, double y)
    {
        if (m_window_move) m_window_move(x, y);
    }
}
}
}
