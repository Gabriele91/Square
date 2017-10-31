//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Window_private.h"


//VIEW ENABLE RIGHT CLICK
@implementation NSSquareView
- (void)rightMouseDown:(NSEvent *)theEvent
{
    [[self nextResponder] rightMouseDown:theEvent];
}
@end

//Implement Window
@implementation NSSquareWindow

- (id) init
{
    self=[super init];
    m_enable_send_app_on_close=true;
    return self;
}

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

- (BOOL) acceptsFirstResponder
{
    return YES;
}

- (void) close
{
    // close app
    if(m_enable_send_app_on_close)  [NSApp terminate:self];
    // If the app refused to terminate, this window should still close.
    [super close];
}

@end

namespace Square
{
namespace Video
{
namespace Cocoa
{
        WindowCocoa::WindowCocoa()
        {
        }
        
        WindowCocoa::WindowCocoa
                   (  NSSquareWindow* window
                    , NSSquareView* view
                    , NSOpenGLContext* context
                    , const WindowInfo& info
                    )
        {
            m_info   = info;
            m_window = window;
            m_view   = view;
            m_context= context;
        }
        
        WindowCocoa::~WindowCocoa()
        {
            //remove ref
            m_window = nullptr;
            m_view   = nullptr;
            m_context= nullptr;
        }
        
        void WindowCocoa::swap() const
        {
            //glSwapAPPLE();
            [m_context flushBuffer];
        }
        
        void WindowCocoa::acquire_context() const
        {
            [m_context makeCurrentContext];
        }
        
        bool WindowCocoa::is_fullscreen() const
        {
            return m_info.m_fullscreen;
        }
        
        bool WindowCocoa::is_resizable() const
        {
            return m_info.m_resize;
        }
        
        void WindowCocoa::get_size(unsigned int size[2]) const
        {
            size[0] = m_view.frame.size.width;
            size[1] = m_view.frame.size.height;
        }
        
        void WindowCocoa::get_position(int position[2]) const
        {
            position[0] = m_window.frame.origin.x;
            position[1] = m_window.frame.origin.y;
        }
        
        void WindowCocoa::set_size(unsigned int size[2])
        {
            [m_view setFrameSize:CGSizeMake(size[0], size[1])];
        }
        
        void WindowCocoa::set_position(int position[2])
        {
            [m_window setFrameOrigin:NSMakePoint(position[0], position[1])];
        }
        
        void WindowCocoa::set_mouse_position(int position[2])
        {
            //calc inverse top view
            float screenH=m_window.screen.frame.origin.y+m_window.screen.frame.size.height;
            float windowH=m_window.frame.origin.y+m_window.frame.size.height;
            float viewTop=m_window.frame.size.height-m_view.frame.size.height;
            //size
            unsigned int size[2];
            get_size(size);
            //calc mouse point in screen
            CGPoint cgpoint;
            cgpoint.x =   m_window.frame.origin.x + std::max<int>(std::min<int>(position[0],size[0]),0);
            cgpoint.y =   (screenH-windowH+viewTop) + std::max<int>(std::min<int>(position[1],size[1]),1.0);
            //send event
            CGEventRef event = CGEventCreateMouseEvent(nullptr, kCGEventMouseMoved, cgpoint, kCGMouseButtonLeft);
            CGEventPost(kCGHIDEventTap, event);
            CFRelease(event);
        }
        
        bool WindowCocoa::enable_resize(bool enable)
        {
            if(m_info.m_resize != enable)
            {
                //set to true
                m_info.m_resize = enable;
                //deattach context
                [m_context setView:nil];
                //close window
                [m_window orderOut: s_os_context.m_application];
                [m_window close];
                //create a new window (and dealloc old window)
                cocoa_create_window(*this, m_info);
                //move
                for(auto listener : s_os_context.m_input)
                {
                    cocoa_move_listener(this, listener.second);
                }
                //attach context
                [m_context setView:[m_window contentView]];
                //aquire
                acquire_context();
                //ok
                return true;
            }
            return false;
        }
        
        bool WindowCocoa::enable_fullscreen(bool enable)
        {
            if(m_info.m_fullscreen != enable)
            {
                //set to true
                m_info.m_fullscreen = enable;
                //deattach context
                [m_context setView:nil];
                //close window
                [m_window orderOut: s_os_context.m_application];
                [m_window close];
                //create a new window (and dealloc old window)
                cocoa_create_window(*this, m_info);
                //move
                for(auto listener : s_os_context.m_input)
                {
                    cocoa_move_listener(this, listener.second);
                }
                //attach context
                [m_context setView:[m_window contentView]];
                //aquire
                acquire_context();
                //ok
                return true;
            }
            return false;
        }
        
        NSSquareView* WindowCocoa::narive() const
        {
            return m_view;
        }
}
}
}
