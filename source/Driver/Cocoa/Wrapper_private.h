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
#include <memory>
#include <cstring>
#include <unordered_map>
///////////////////////////////////////////////////////////////////////////////
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CIContext.h>
#import  <AppKit/NSOpenGLView.h>
#import  <AppKit/NSWindow.h>
#import  <AppKit/NSScreen.h>
///////////////////////////////////////////////////////////////////////////////
#include <Foundation/Foundation.h>
#include <CoreVideo/CVBase.h>
#include <CoreVideo/CVDisplayLink.h>

///////////////////////////////////////////////////////////////////////////////
// COCOA APPLICATION CLASS
@interface NSSquareApplication : NSApplication
{
    NSArray *__autoreleasing  _Nullable * _Nullable nibObjects;
}
@end

@interface NSSquareApplicationDelegate :  NSObject
{
}
@end

///////////////////////////////////////////////////////////////////////////////
// COCOA CPP INTERFACE
namespace Square
{
namespace Video
{
namespace Cocoa
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CLASS DECLARE
    class ScreenCocoa;
    class WindowCocoa;
    class InputCocoa;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Input map
    struct SQUARE_API CoaoaInputKeyMap
    {
        unsigned int  m_scancodes[512];
        KeyboardEvent m_keyboard[512];
    };
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Context
    struct SQUARE_API WrapperContext
    {
        NSSquareApplication*                           m_application;
        std::vector < ScreenCocoa >                    m_screens;
        std::unordered_map<WindowCocoa*, WindowCocoa*> m_windows;
        std::unordered_map<InputCocoa*, InputCocoa*>   m_input;
        //key map
        CoaoaInputKeyMap m_key_map;
    };
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Global context
    extern WrapperContext s_os_context;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DECLARE
    InputCocoa* create_input(WindowCocoa* window, Input* input);
    bool cocoa_create_window(WindowCocoa& wnd_cocoa,const WindowInfo& info);
    InputCocoa* create_input(WindowCocoa* window, Input* input);
    void cocoa_move_listener(WindowCocoa* wnd_cocoa, InputCocoa* ln_cocoa);
    void cocoa_send_close_event_to_all_windows();
}
}
}

///////////////////////////////////////////////////////////////////////////////
// COCOA WINDOW CLASS
@interface  NSSquareWindow : NSWindow
{
@public
    bool m_enable_send_app_on_close;
}

- (id) init;
//! These are needed for borderless/fullscreen windows
- (BOOL)canBecomeKeyWindow;
- (BOOL)canBecomeMainWindow;
//! In addition to closing the window, also terminate the app.
- (void) close;
//! @return yes
- (BOOL) acceptsFirstResponder;

@end
//VIEW
@interface NSSquareView : NSView
// The default implementation doesn't pass rightMouseDown to responder chain
- (void)rightMouseDown:(NSEvent *)theEvent;
@end

///////////////////////////////////////////////////////////////////////////////
// COCOA INPUT DELEGATE
@interface NSCocoaListener : NSResponder <NSWindowDelegate>
{
    NSWindow*                         m_window;
    Square::Video::Cocoa::InputCocoa* m_input;
}

- (id) init;

/* Window init */
-(void) listen: (Square::Video::Cocoa::InputCocoa *)input : (NSWindow*) window;
-(void) close;

/* Window delegate functionality */
-(BOOL) windowShouldClose:(id) sender;
-(void) windowDidResize:(NSNotification *) notification;
/* Window event handling */
-(void) mouseDown:(NSEvent *) event;
-(void) rightMouseDown:(NSEvent *) event;
-(void) otherMouseDown:(NSEvent *) event;
-(void) mouseUp:(NSEvent *) event;
-(void) rightMouseUp:(NSEvent *) event;
-(void) otherMouseUp:(NSEvent *) event;
-(void) mouseEntered:(NSEvent *) event;
-(void) mouseExited:(NSEvent *) event;
-(void) mouseMoved:(NSEvent *) event;
-(void) mouseDragged:(NSEvent *) event;
-(void) rightMouseDragged:(NSEvent *) event;
-(void) otherMouseDragged:(NSEvent *) event;
-(void) scrollWheel:(NSEvent *) event;
-(void)keyDown:(NSEvent *)event;
-(void)keyUp:(NSEvent *)event;


@end
