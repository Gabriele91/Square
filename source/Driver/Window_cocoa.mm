//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
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

namespace Square
{
    namespace Video
    {
        class ScreenCocoa;
        class WindowCocoa;
        class InputCocoa;
        //Global close
        static void cocoa_send_close_event_to_all_windows();
    }
}

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

@implementation NSSquareApplication

// From http://cocoadev.com/index.pl?GameKeyboardHandlingAlmost
// This works around an AppKit bug, where key up events while holding
// down the command key don't get sent to the key window.
- (void)sendEvent:(NSEvent *)event
{
    if ([event type] == NSEventTypeKeyUp && ([event modifierFlags] & NSEventModifierFlagCommand))
    {
        [[self keyWindow] sendEvent:event];
    }
    else
    {
        [super sendEvent:event];
    }
}

// No-op thread entry point
- (void)doNothing:(id)object
{
    
}

//set bundle
- (void)loadMainMenu
{
    [[NSBundle mainBundle] loadNibNamed:@"MainMenu" owner:NSApp  topLevelObjects: nibObjects];
}
@end

@implementation NSSquareApplicationDelegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    //Close all
    Square::Video::cocoa_send_close_event_to_all_windows();
    //block close
    return NSTerminateCancel;
}
- (void)applicationDidChangeScreenParameters:(NSNotification *) notification
{
    /* none */
}
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    [NSApp stop:nil];
    //Sent event
    NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                        location:NSMakePoint(0, 0)
                                   modifierFlags:0
                                       timestamp:0
                                    windowNumber:0
                                         context:nil
                                         subtype:0
                                           data1:0
                                           data2:0];
    [NSApp postEvent:event atStart:YES];
}
- (void)applicationDidHide:(NSNotification *)notification
{
    /* none */
}
@end
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

///////////////////////////////////////////////////////////////////////////////
// COCOA INPUT DELEGATE
@interface NSCocoaListener : NSResponder <NSWindowDelegate>
{
    NSWindow*                  m_window;
    Square::Video::InputCocoa* m_input;
}

- (id) init;

/* Window init */
-(void) listen: (Square::Video::InputCocoa *)input : (NSWindow*) window;
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

///////////////////////////////////////////////////////////////////////////////
// VIEW
@interface NSSquareView : NSView
// The default implementation doesn't pass rightMouseDown to responder chain
- (void)rightMouseDown:(NSEvent *)theEvent;
@end

@implementation NSSquareView
- (void)rightMouseDown:(NSEvent *)theEvent
{
    [[self nextResponder] rightMouseDown:theEvent];
}
@end
///////////////////////////////////////////////////////////////////////////////




namespace Square
{
namespace Video
{
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Input map
    struct CoaoaInputKeyMap
    {
        unsigned int  m_scancodes[512];
        KeyboardEvent m_keyboard[512];
    };
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Context
    struct WrapperContext
    {
        NSSquareApplication*                           m_application;
        std::vector < ScreenCocoa >                    m_screens;
        std::unordered_map<WindowCocoa*, WindowCocoa*> m_windows;
        std::unordered_map<InputCocoa*, InputCocoa*>   m_input;
        //key map
        CoaoaInputKeyMap m_key_map;
    };
    static WrapperContext s_os_context;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DECLARE
    static InputCocoa* create_input(WindowCocoa* window, Input* input);
    static bool cocoa_create_window(WindowCocoa& wnd_cocoa,const WindowInfo& info);
    static InputCocoa* create_input(WindowCocoa* window, Input* input);
    static void cocoa_move_listener(WindowCocoa* wnd_cocoa, InputCocoa* ln_cocoa);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    class ScreenCocoa
    {
    public:
        
        uint32_t m_id{ 0 };
        uint32_t m_unit_number{ 0 };
        float    m_size[2]{ 0, 0 };
        
        ScreenCocoa()
        {
        }
        
        ScreenCocoa(uint32_t id, uint32_t unit_number, const CGSize size)
        {
            m_id = id;
            m_unit_number = unit_number;
            m_size[0] = size.width;
            m_size[1] = size.height;
        }
        
        void get_position(unsigned int size[2])
        {
            const CGRect bounds = CGDisplayBounds(m_id);
            size[0] =  bounds.origin.x;
            size[1] =  bounds.origin.x;
        }
        
        struct ScreenMode
        {
            size_t width;
            size_t height;
            size_t depth;
        };
        
        static size_t display_bits_per_pixel_for_mode(CGDisplayModeRef mode)
        {
            size_t depth = 0;
            CFStringRef pix_enc = CGDisplayModeCopyPixelEncoding (mode);

            if(CFStringCompare(pix_enc, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
                depth = 32;
            else if(CFStringCompare(pix_enc, CFSTR(IO16BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
                depth = 16;
            else if(CFStringCompare(pix_enc, CFSTR(IO8BitIndexedPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
                depth = 8;

            return depth;
        }
        
        CGDisplayModeRef best_match_for_mode( ScreenMode screenMode )
        {
            
            bool exactMatch = false;
            
            // Get a copy of the current display mode
            CGDisplayModeRef displayMode = CGDisplayCopyDisplayMode(m_id);
            
            // Loop through all display modes to determine the closest match.
            // CGDisplayBestModeForParameters is deprecated on 10.6 so we will emulate it's behavior
            // Try to find a mode with the requested depth and equal or greater dimensions first.
            // If no match is found, try to find a mode with greater depth and same or greater dimensions.
            // If still no match is found, just use the current mode.
            CFArrayRef allModes = CGDisplayCopyAllDisplayModes(kCGDirectMainDisplay, NULL);
            for(int i = 0; i < CFArrayGetCount(allModes); i++)
            {
                CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(allModes, i);
                
                if(display_bits_per_pixel_for_mode( mode ) != screenMode.depth)
                    continue;
                
                if((CGDisplayModeGetWidth(mode) >= screenMode.width) &&
                   (CGDisplayModeGetHeight(mode) >= screenMode.height) &&
                   (CGDisplayModeGetWidth(displayMode) >= CGDisplayModeGetWidth(mode))&&
                   (CGDisplayModeGetHeight(displayMode) >= CGDisplayModeGetHeight(mode)))
                {
                    displayMode = mode;
                    exactMatch = true;
                }
            }
            
            // No depth match was found
            if(!exactMatch)
            {
                for(int i = 0; i < CFArrayGetCount(allModes); i++)
                {
                    CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(allModes, i);
                    if(display_bits_per_pixel_for_mode( mode )  >= screenMode.depth)
                        continue;
                    
                    
                    if((CGDisplayModeGetWidth(mode) >= screenMode.width) &&
                       (CGDisplayModeGetHeight(mode) >= screenMode.height) &&
                       (CGDisplayModeGetWidth(displayMode) >= CGDisplayModeGetWidth(mode))&&
                       (CGDisplayModeGetHeight(displayMode) >= CGDisplayModeGetHeight(mode)))
                    {
                        displayMode = mode;
                    }
                }
            }
            
            return displayMode;
        }
    };
    
    class WindowCocoa
    {
    public:
        WindowInfo       m_info;
        NSSquareWindow*  m_window    { nullptr };   /* window */
        NSSquareView*    m_view      { nullptr };   /* view */
        NSOpenGLContext* m_context   { nullptr };   /* OpenGL context */
        Window*          m_window_ref{ nullptr };
        
        WindowCocoa()
        {
        }
        
        WindowCocoa(  NSSquareWindow* window
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
        
        ~WindowCocoa()
        {
            //remove ref
            m_window = nullptr;
            m_view   = nullptr;
            m_context= nullptr;
        }
        
        virtual void swap() const
        {
            //glSwapAPPLE();
            [m_context flushBuffer];
        }
        
        virtual void acquire_context() const
        {
            [m_context makeCurrentContext];
        }
        
        virtual bool is_fullscreen() const
        {
            return m_info.m_fullscreen;
        }
        
        virtual bool is_resizable() const
        {
            return m_info.m_resize;
        }
        
        virtual void get_size(unsigned int size[2]) const
        {
            size[0] = m_view.frame.size.width;
            size[1] = m_view.frame.size.height;
        }
        
        virtual void get_position(int position[2]) const
        {
            position[0] = m_window.frame.origin.x;
            position[1] = m_window.frame.origin.y;
        }
        
        virtual void set_size(unsigned int size[2])
        {
            [m_view setFrameSize:CGSizeMake(size[0], size[1])];
        }
        
        virtual void set_position(int position[2])
        {
            [m_window setFrameOrigin:NSMakePoint(position[0], position[1])];
        }
        
        virtual void set_mouse_position(int position[2])
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
        
        virtual bool enable_resize(bool enable)
        {
            return false;
        }
        
        virtual bool enable_fullscreen(bool enable)
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
            }
            return false;
        }
        
        virtual NSSquareView* narive() const
        {
            return m_view;
        }
        
    protected:
        
    };
    
    struct InputCocoa
    {
        WindowCocoa*                                                                      m_window_ref{ nullptr };
        Input*                                                                            m_input_ref{ nullptr };
        NSCocoaListener*                                                                    m_listener { nullptr };
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
        
        InputCocoa(WindowCocoa* window, Input* input)
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
        
        ~InputCocoa()
        {
            //close
            [m_listener close];
            //to null
            m_window_ref = nullptr;
            m_input_ref  = nullptr;
            m_listener   = nullptr;
        }
        
        void send_keyboard_event(KeyboardEvent kevent, short mode, ActionEvent action)
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
        void send_character_event(int character, short mode, bool plain)
        {
            if (m_character) m_character(character, mode, plain);
        }
        void send_mouse_button_event(MouseButtonEvent mevent, ActionEvent action)
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
        void send_mouse_move_event(double x, double y)
        {
            if (m_mouse_move) m_mouse_move(x, y);
        }
        void send_mouse_scroll_event(double scroll)
        {
            if (m_scroll) m_scroll(scroll);
        }
        void send_touch_event(FingerEvent fevent, ActionEvent action)
        {
            if (m_touch) m_touch(fevent, action);
        }
        void send_window_event(WindowEvent wevent)
        {
            if (m_window) m_window(wevent);
        }
        void send_window_move_event(double x, double y)
        {
            if (m_window_move) m_window_move(x, y);
        }
        
    };
    ////////////////////////////////////////////////////////////////////////////////////
    // VIDEO
    //init key map
    static void helper_input_init()
    {
        std::memset(s_os_context.m_key_map.m_scancodes, -1, sizeof(s_os_context.m_key_map.m_scancodes));
        std::memset(s_os_context.m_key_map.m_keyboard,  KeyboardEvent::KEY_UNKNOWN, sizeof(s_os_context.m_key_map.m_keyboard));
        
        s_os_context.m_key_map.m_keyboard[0x1D] = KeyboardEvent::KEY_0;
        s_os_context.m_key_map.m_keyboard[0x12] = KeyboardEvent::KEY_1;
        s_os_context.m_key_map.m_keyboard[0x13] = KeyboardEvent::KEY_2;
        s_os_context.m_key_map.m_keyboard[0x14] = KeyboardEvent::KEY_3;
        s_os_context.m_key_map.m_keyboard[0x15] = KeyboardEvent::KEY_4;
        s_os_context.m_key_map.m_keyboard[0x17] = KeyboardEvent::KEY_5;
        s_os_context.m_key_map.m_keyboard[0x16] = KeyboardEvent::KEY_6;
        s_os_context.m_key_map.m_keyboard[0x1A] = KeyboardEvent::KEY_7;
        s_os_context.m_key_map.m_keyboard[0x1C] = KeyboardEvent::KEY_8;
        s_os_context.m_key_map.m_keyboard[0x19] = KeyboardEvent::KEY_9;
        s_os_context.m_key_map.m_keyboard[0x00] = KeyboardEvent::KEY_A;
        s_os_context.m_key_map.m_keyboard[0x0B] = KeyboardEvent::KEY_B;
        s_os_context.m_key_map.m_keyboard[0x08] = KeyboardEvent::KEY_C;
        s_os_context.m_key_map.m_keyboard[0x02] = KeyboardEvent::KEY_D;
        s_os_context.m_key_map.m_keyboard[0x0E] = KeyboardEvent::KEY_E;
        s_os_context.m_key_map.m_keyboard[0x03] = KeyboardEvent::KEY_F;
        s_os_context.m_key_map.m_keyboard[0x05] = KeyboardEvent::KEY_G;
        s_os_context.m_key_map.m_keyboard[0x04] = KeyboardEvent::KEY_H;
        s_os_context.m_key_map.m_keyboard[0x22] = KeyboardEvent::KEY_I;
        s_os_context.m_key_map.m_keyboard[0x26] = KeyboardEvent::KEY_J;
        s_os_context.m_key_map.m_keyboard[0x28] = KeyboardEvent::KEY_K;
        s_os_context.m_key_map.m_keyboard[0x25] = KeyboardEvent::KEY_L;
        s_os_context.m_key_map.m_keyboard[0x2E] = KeyboardEvent::KEY_M;
        s_os_context.m_key_map.m_keyboard[0x2D] = KeyboardEvent::KEY_N;
        s_os_context.m_key_map.m_keyboard[0x1F] = KeyboardEvent::KEY_O;
        s_os_context.m_key_map.m_keyboard[0x23] = KeyboardEvent::KEY_P;
        s_os_context.m_key_map.m_keyboard[0x0C] = KeyboardEvent::KEY_Q;
        s_os_context.m_key_map.m_keyboard[0x0F] = KeyboardEvent::KEY_R;
        s_os_context.m_key_map.m_keyboard[0x01] = KeyboardEvent::KEY_S;
        s_os_context.m_key_map.m_keyboard[0x11] = KeyboardEvent::KEY_T;
        s_os_context.m_key_map.m_keyboard[0x20] = KeyboardEvent::KEY_U;
        s_os_context.m_key_map.m_keyboard[0x09] = KeyboardEvent::KEY_V;
        s_os_context.m_key_map.m_keyboard[0x0D] = KeyboardEvent::KEY_W;
        s_os_context.m_key_map.m_keyboard[0x07] = KeyboardEvent::KEY_X;
        s_os_context.m_key_map.m_keyboard[0x10] = KeyboardEvent::KEY_Y;
        s_os_context.m_key_map.m_keyboard[0x06] = KeyboardEvent::KEY_Z;
        
        s_os_context.m_key_map.m_keyboard[0x27] = KeyboardEvent::KEY_APOSTROPHE;
        s_os_context.m_key_map.m_keyboard[0x2A] = KeyboardEvent::KEY_BACKSLASH;
        s_os_context.m_key_map.m_keyboard[0x2B] = KeyboardEvent::KEY_COMMA;
        s_os_context.m_key_map.m_keyboard[0x18] = KeyboardEvent::KEY_EQUAL;
        s_os_context.m_key_map.m_keyboard[0x32] = KeyboardEvent::KEY_GRAVE_ACCENT;
        s_os_context.m_key_map.m_keyboard[0x21] = KeyboardEvent::KEY_LEFT_BRACKET;
        s_os_context.m_key_map.m_keyboard[0x1B] = KeyboardEvent::KEY_MINUS;
        s_os_context.m_key_map.m_keyboard[0x2F] = KeyboardEvent::KEY_PERIOD;
        s_os_context.m_key_map.m_keyboard[0x1E] = KeyboardEvent::KEY_RIGHT_BRACKET;
        s_os_context.m_key_map.m_keyboard[0x29] = KeyboardEvent::KEY_SEMICOLON;
        s_os_context.m_key_map.m_keyboard[0x2C] = KeyboardEvent::KEY_SLASH;
        s_os_context.m_key_map.m_keyboard[0x0A] = KeyboardEvent::KEY_WORLD_1;
        
        s_os_context.m_key_map.m_keyboard[0x33] = KeyboardEvent::KEY_BACKSPACE;
        s_os_context.m_key_map.m_keyboard[0x39] = KeyboardEvent::KEY_CAPS_LOCK;
        s_os_context.m_key_map.m_keyboard[0x75] = KeyboardEvent::KEY_DELETE;
        s_os_context.m_key_map.m_keyboard[0x7D] = KeyboardEvent::KEY_DOWN;
        s_os_context.m_key_map.m_keyboard[0x77] = KeyboardEvent::KEY_END;
        s_os_context.m_key_map.m_keyboard[0x24] = KeyboardEvent::KEY_ENTER;
        s_os_context.m_key_map.m_keyboard[0x35] = KeyboardEvent::KEY_ESCAPE;
        s_os_context.m_key_map.m_keyboard[0x7A] = KeyboardEvent::KEY_F1;
        s_os_context.m_key_map.m_keyboard[0x78] = KeyboardEvent::KEY_F2;
        s_os_context.m_key_map.m_keyboard[0x63] = KeyboardEvent::KEY_F3;
        s_os_context.m_key_map.m_keyboard[0x76] = KeyboardEvent::KEY_F4;
        s_os_context.m_key_map.m_keyboard[0x60] = KeyboardEvent::KEY_F5;
        s_os_context.m_key_map.m_keyboard[0x61] = KeyboardEvent::KEY_F6;
        s_os_context.m_key_map.m_keyboard[0x62] = KeyboardEvent::KEY_F7;
        s_os_context.m_key_map.m_keyboard[0x64] = KeyboardEvent::KEY_F8;
        s_os_context.m_key_map.m_keyboard[0x65] = KeyboardEvent::KEY_F9;
        s_os_context.m_key_map.m_keyboard[0x6D] = KeyboardEvent::KEY_F10;
        s_os_context.m_key_map.m_keyboard[0x67] = KeyboardEvent::KEY_F11;
        s_os_context.m_key_map.m_keyboard[0x6F] = KeyboardEvent::KEY_F12;
        s_os_context.m_key_map.m_keyboard[0x69] = KeyboardEvent::KEY_F13;
        s_os_context.m_key_map.m_keyboard[0x6B] = KeyboardEvent::KEY_F14;
        s_os_context.m_key_map.m_keyboard[0x71] = KeyboardEvent::KEY_F15;
        s_os_context.m_key_map.m_keyboard[0x6A] = KeyboardEvent::KEY_F16;
        s_os_context.m_key_map.m_keyboard[0x40] = KeyboardEvent::KEY_F17;
        s_os_context.m_key_map.m_keyboard[0x4F] = KeyboardEvent::KEY_F18;
        s_os_context.m_key_map.m_keyboard[0x50] = KeyboardEvent::KEY_F19;
        s_os_context.m_key_map.m_keyboard[0x5A] = KeyboardEvent::KEY_F20;
        s_os_context.m_key_map.m_keyboard[0x73] = KeyboardEvent::KEY_HOME;
        s_os_context.m_key_map.m_keyboard[0x72] = KeyboardEvent::KEY_INSERT;
        s_os_context.m_key_map.m_keyboard[0x7B] = KeyboardEvent::KEY_LEFT;
        s_os_context.m_key_map.m_keyboard[0x3A] = KeyboardEvent::KEY_LEFT_ALT;
        s_os_context.m_key_map.m_keyboard[0x3B] = KeyboardEvent::KEY_LEFT_CONTROL;
        s_os_context.m_key_map.m_keyboard[0x38] = KeyboardEvent::KEY_LEFT_SHIFT;
        s_os_context.m_key_map.m_keyboard[0x37] = KeyboardEvent::KEY_LEFT_SUPER;
        s_os_context.m_key_map.m_keyboard[0x6E] = KeyboardEvent::KEY_MENU;
        s_os_context.m_key_map.m_keyboard[0x47] = KeyboardEvent::KEY_NUM_LOCK;
        s_os_context.m_key_map.m_keyboard[0x79] = KeyboardEvent::KEY_PAGE_DOWN;
        s_os_context.m_key_map.m_keyboard[0x74] = KeyboardEvent::KEY_PAGE_UP;
        s_os_context.m_key_map.m_keyboard[0x7C] = KeyboardEvent::KEY_RIGHT;
        s_os_context.m_key_map.m_keyboard[0x3D] = KeyboardEvent::KEY_RIGHT_ALT;
        s_os_context.m_key_map.m_keyboard[0x3E] = KeyboardEvent::KEY_RIGHT_CONTROL;
        s_os_context.m_key_map.m_keyboard[0x3C] = KeyboardEvent::KEY_RIGHT_SHIFT;
        s_os_context.m_key_map.m_keyboard[0x36] = KeyboardEvent::KEY_RIGHT_SUPER;
        s_os_context.m_key_map.m_keyboard[0x31] = KeyboardEvent::KEY_SPACE;
        s_os_context.m_key_map.m_keyboard[0x30] = KeyboardEvent::KEY_TAB;
        s_os_context.m_key_map.m_keyboard[0x7E] = KeyboardEvent::KEY_UP;
        
        s_os_context.m_key_map.m_keyboard[0x52] = KeyboardEvent::KEY_KP_0;
        s_os_context.m_key_map.m_keyboard[0x53] = KeyboardEvent::KEY_KP_1;
        s_os_context.m_key_map.m_keyboard[0x54] = KeyboardEvent::KEY_KP_2;
        s_os_context.m_key_map.m_keyboard[0x55] = KeyboardEvent::KEY_KP_3;
        s_os_context.m_key_map.m_keyboard[0x56] = KeyboardEvent::KEY_KP_4;
        s_os_context.m_key_map.m_keyboard[0x57] = KeyboardEvent::KEY_KP_5;
        s_os_context.m_key_map.m_keyboard[0x58] = KeyboardEvent::KEY_KP_6;
        s_os_context.m_key_map.m_keyboard[0x59] = KeyboardEvent::KEY_KP_7;
        s_os_context.m_key_map.m_keyboard[0x5B] = KeyboardEvent::KEY_KP_8;
        s_os_context.m_key_map.m_keyboard[0x5C] = KeyboardEvent::KEY_KP_9;
        s_os_context.m_key_map.m_keyboard[0x45] = KeyboardEvent::KEY_KP_ADD;
        s_os_context.m_key_map.m_keyboard[0x41] = KeyboardEvent::KEY_KP_DECIMAL;
        s_os_context.m_key_map.m_keyboard[0x4B] = KeyboardEvent::KEY_KP_DIVIDE;
        s_os_context.m_key_map.m_keyboard[0x4C] = KeyboardEvent::KEY_KP_ENTER;
        s_os_context.m_key_map.m_keyboard[0x51] = KeyboardEvent::KEY_KP_EQUAL;
        s_os_context.m_key_map.m_keyboard[0x43] = KeyboardEvent::KEY_KP_MULTIPLY;
        s_os_context.m_key_map.m_keyboard[0x4E] = KeyboardEvent::KEY_KP_SUBTRACT;
        
        for (int scancode = 0;  scancode < 256;  ++scancode)
        {
            // Store the reverse translation for faster key name lookup
            if (s_os_context.m_key_map.m_keyboard[scancode] >= 0)
                s_os_context.m_key_map.m_scancodes[s_os_context.m_key_map.m_keyboard[scancode]] = scancode;
        }
    }
    //init screen
    static void helper_screen_init()
    {
        uint32_t display_count = 0;
        CGGetOnlineDisplayList(0, NULL, &display_count);
        CGDirectDisplayID* displays = (CGDirectDisplayID*)std::calloc(display_count, sizeof(CGDirectDisplayID));
        CGGetOnlineDisplayList(display_count, displays, &display_count);

        for (uint32_t i = 0;  i != display_count;  ++i)
        {
            //-----------------------------------------------------------
            const uint32_t unit_number = CGDisplayUnitNumber(displays[i]);
            //-----------------------------------------------------------
            if (CGDisplayIsAsleep(displays[i])) continue;
            //-----------------------------------------------------------
            const CGSize size = CGDisplayScreenSize(displays[i]);
            //-----------------------------------------------------------
            s_os_context.m_screens.push_back(ScreenCocoa{ displays[i], unit_number, size });
            //-----------------------------------------------------------
        }

        free(displays);
    }
    
    static void cocoa_send_close_event_to_all_windows()
    {
        for(auto input : s_os_context.m_input)
        {
            input.second->send_window_event(WindowEvent::CLOSE);
        }
    }
    
    void init()
    {
        //ini key map
        helper_input_init();
        //init screen
        helper_screen_init();
        //init application
        //Shadred
        s_os_context.m_application = [NSSquareApplication sharedApplication];
        //Not loop
        [NSThread detachNewThreadSelector:@selector(doNothing:)
                                 toTarget:NSApp
                               withObject:nil];
        //open app on dock
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        //load Bundle
#if 0
        if ([[NSBundle mainBundle] pathForResource:@"MainMenu" ofType:@"nib"])
            [NSApp loadMainMenu];
        else
            [NSApp loadMainMenu];
#endif
        //as delegate
        [NSApp setDelegate: [[NSSquareApplicationDelegate alloc] init]];
        [NSApp run];
    }
    
    void close()
    {
        //remove all inputs/windows
        for (auto inp_it : s_os_context.m_input)
        {
            inp_it.second->m_input_ref->destoy();
        }
        for (auto wnd_it : s_os_context.m_windows)
        {
            wnd_it.second->m_window_ref->destoy();
        }
        s_os_context.m_input.clear();
        s_os_context.m_windows.clear();
        //clear all
        s_os_context.m_screens.clear();
        //close
        s_os_context.m_application = nil;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////
    // SCREEN
    unsigned int Screen::count()
    {
        return (unsigned int)(s_os_context.m_screens.size());
    }
    
    Screen Screen::by_index(unsigned int index)
    {
        Screen output;
        output.m_native = &s_os_context.m_screens[index];
        return output;
    }
    
    Screen::Screen() { /* void */ }
    
    Screen::~Screen() { /* void */ }
    
    void* Screen::conteiner() const
    {
        return m_native;
    }
    
    void Screen::get_size(unsigned int& width, unsigned int& height)
    {
        width  = (unsigned int)((ScreenCocoa*)m_native)->m_size[0];
        height = (unsigned int)((ScreenCocoa*)m_native)->m_size[1];
    }
    ////////////////////////////////////////////////////////////////////////////////////
    // WINDOW
    static NSUInteger cocoa_display_style(const WindowInfo& info)
    {
        NSUInteger mask = 0;
        
        if (info.m_fullscreen)
        {
            mask |= NSWindowStyleMaskBorderless;
        }
        else
        {
            mask
            |= NSWindowStyleMaskTitled
            |  NSWindowStyleMaskClosable
            |  NSWindowStyleMaskMiniaturizable;
            
            if (info.m_resize)  mask |= NSWindowStyleMaskResizable;
        }
        
        return mask;
    }
    
    NSOpenGLContext* cocoa_create_context(const WindowInfo& info)
    {
        //type context
        int i=0;
        NSOpenGLPixelFormatAttribute attributes[32];
        attributes[i++]   = NSOpenGLPFANoRecovery;
        //attributes[i++]   = NSOpenGLPFAAccelerated;
        attributes[i++]   = NSOpenGLPFADoubleBuffer;
        attributes[i++]   = NSOpenGLPFAColorSize;
        attributes[i++]   = info.m_context.m_color;
        //depth
        if(info.m_context.m_depth)
        {
            attributes[i++]   = NSOpenGLPFADepthSize;
            attributes[i++]   = info.m_context.m_depth;
        }
        //stencil
        if(info.m_context.m_stencil)
        {
            attributes[i++]   = NSOpenGLPFAStencilSize;
            attributes[i++]   = info.m_context.m_stencil;
        }
        //msaa
        if(info.m_context.m_anti_aliasing != ContextInfo::NOAA)
        {
            attributes[i++]=NSOpenGLPFASampleBuffers;
            attributes[i++]=1;
            attributes[i++]=NSOpenGLPFASamples;
            attributes[i++]=static_cast<NSOpenGLPixelFormatAttribute>(info.m_context.m_anti_aliasing);
        }
        //
        attributes[i] = 0;
        
        NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
        NSOpenGLContext* open_gl_context = [[NSOpenGLContext alloc]  initWithFormat:pixelFormat shareContext:nil];
        //swap interval
        GLint value = 1;
        [open_gl_context setValues:&value forParameter:NSOpenGLCPSwapInterval];
        /////////////////////////
        return open_gl_context;
    }
    
    static bool cocoa_create_window(WindowCocoa& wnd_cocoa,const WindowInfo& info)
    {
        //rect
        NSRect content_rect;
        //get screen
        auto* screen = (ScreenCocoa*)info.m_screen->conteiner();
        //full screen
        if(info.m_fullscreen)
        {
            ScreenCocoa::ScreenMode mode;
            mode.width  = info.m_size[0];
            mode.height = info.m_size[1];
            mode.depth  = info.m_context.m_color;
            //position
            unsigned int position[2] = {0,0};
            screen->get_position(position);
            //mode
            CGDisplayModeRef cocoa_mode = screen->best_match_for_mode(mode);
            //mode
            content_rect  = NSMakeRect(position[0],
                                       position[1],
                                       CGDisplayModeGetWidth(cocoa_mode),
                                       CGDisplayModeGetHeight(cocoa_mode));
        }
        else
        {
            content_rect = NSMakeRect(0,0, info.m_size[0], info.m_size[1]);
        }
        //screen
        NSSquareWindow* window = [[NSSquareWindow alloc]
                                  initWithContentRect:content_rect
                                  styleMask: cocoa_display_style(info)
                                  backing:NSBackingStoreBuffered
                                  defer:NO];
        //test
        if (window == nil) return nullptr;
        //forground
        
        if (info.m_fullscreen)
        {
            [window setLevel:NSMainMenuWindowLevel + 1];
            // Perform any other window configuration you desire
            [window setOpaque:YES];
            [window setHidesOnDeactivate:YES];
        }
        else
        {
            //center of screen
            [window center];
            //ui resize
            if (info.m_resize)
            {
                [window setCollectionBehavior:
                   NSWindowCollectionBehaviorFullScreenPrimary
                 | NSWindowCollectionBehaviorManaged
                ];
            }
        }
        //screen title
        [window setFrameAutosaveName: [NSString stringWithUTF8String:info.m_title.c_str()]];
        [window setTitle: [NSString stringWithUTF8String:info.m_title.c_str()]];

        //Force show
        [window makeKeyAndOrderFront: s_os_context.m_application];
        [NSApp activateIgnoringOtherApps:YES];
        
        //View
        NSSquareView* view = [[NSSquareView alloc] initWithFrame:content_rect];
        [window setContentView: view];
        [window makeFirstResponder: view];

        //Fullscreen
        if(info.m_fullscreen)
        {
            [view setNeedsDisplay: YES];
        }
        
        wnd_cocoa.m_window = window;
        wnd_cocoa.m_view   = view;
        wnd_cocoa.m_info   = info;
        
        return true;
    }
    
    static WindowCocoa* window_create(const WindowInfo& info, Window* window)
    {
        if (info.m_context.m_type == ContextInfo::CTX_DIRECTX) return nullptr;
        //alloc
        WindowCocoa* window_cocoa = new WindowCocoa();
        //window
        cocoa_create_window(*window_cocoa, info);
        //opengl
        window_cocoa->m_context = cocoa_create_context(info);
        //ref to conteiner
        window_cocoa->m_window_ref = window;
        //search
        s_os_context.m_windows[window_cocoa] = window_cocoa;
        //OpenGL Context
        return window_cocoa;
    }

    static void window_delete(WindowCocoa*& window)
    {
        //search
        auto it_wnd = s_os_context.m_windows.find(window);
        //remove
        if (it_wnd != s_os_context.m_windows.end())
        {
            s_os_context.m_windows.erase(it_wnd);
        }
        //Hide/close window
        [window->m_window orderOut: s_os_context.m_application];
        [window->m_window close];
        //delete
        delete  window;
        //to null
        window = nullptr;
    }

    Window::Window(const WindowInfo& info)
    {
        m_native = window_create(info, this);
    }
    
    Window::~Window()
    {
        destoy();
    }
    
    //native
    void* Window::native() const
    {
        //cast
        const auto* window = (const WindowCocoa*)m_native;
        //
        return (__bridge void*)window->narive();
    }
    
    void* Window::conteiner() const
    {
        return m_native;
    }
    
    bool Window::valid() const
    {
        return m_native != nullptr;
    }
    
    void Window::destoy()
    {
        if (m_native)
        {
            //cast
            auto* window = (WindowCocoa*)m_native;
            window_delete(window);
            m_native = (void*)window;
        }
    }
    //actions
    void Window::clear()
    {
        //none
    }
    
    void Window::swap()
    {
        //cast
        const auto* window = (const WindowCocoa*)m_native;
        //
        window->swap();
    }
    
    void Window::acquire_context()
    {
        //cast
        const auto* window = (const WindowCocoa*)m_native;
        //
        window->acquire_context();
    }
    
    //info
    bool   Window::is_fullscreen()
    {
        //cast
        const auto* window = (const WindowCocoa*)m_native;
        //
        return window->is_fullscreen();
    }
    
    bool   Window::is_resizable()
    {
        //cast
        const auto* window = (const WindowCocoa*)m_native;
        //
        return window->is_resizable();
    }
    
    void   Window::get_size(unsigned int& w, unsigned int& h)
    {
        //cast
        const auto* window = (const WindowCocoa*)m_native;
        //
        unsigned int size[2];
        window->get_size(size);
        w = size[0];
        h = size[1];
    }
    
    void  Window::get_position(int& x, int& y)
    {
        //cast
        const auto* window = (const WindowCocoa*)m_native;
        //
        int pos[2];
        window->get_position(pos);
        x = pos[0];
        y = pos[1];
    }
    
    //set
    void   Window::set_size(unsigned int w, unsigned int h)
    {
        //cast
        auto* window = (WindowCocoa*)m_native;
        //
        unsigned int size[2]{ w, h };
        window->set_size(size);
    }
    
    void   Window::set_position(int x, int y)
    {
        //cast
        auto* window = (WindowCocoa*)m_native;
        //
        int position[2]{ x,y };
        window->set_position(position);
    }
    
    void   Window::enable_resize(bool enable)
    {
        //cast
        auto* window = (WindowCocoa*)m_native;
        //
        window->enable_resize(enable);
    }
    
    void   Window::enable_fullscreen(bool enable)
    {
        //cast
        auto* window = (WindowCocoa*)m_native;
        //
        window->enable_fullscreen(enable);
    }

    }
}
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
    if (key >= sizeof(Square::Video::s_os_context.m_key_map.m_keyboard) /
               sizeof(Square::Video::s_os_context.m_key_map.m_keyboard[0]))
        return Square::Video::KeyboardEvent::KEY_UNKNOWN;
    return Square::Video::s_os_context.m_key_map.m_keyboard[key];
}


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
        :(Square::Video::InputCocoa *) input
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
    ////////////////////////////////////////////////////////////////////////////////////
    // listener
    static void cocoa_move_listener(WindowCocoa* wnd_cocoa, InputCocoa* ln_cocoa)
    {
        if(ln_cocoa->m_window_ref == wnd_cocoa)
        {
            //close
            [ln_cocoa->m_listener close];
            //change
            [ln_cocoa->m_listener listen:ln_cocoa :wnd_cocoa->m_window];
        }
    }
    static InputCocoa* create_input(WindowCocoa* window, Input* input)
    {
        //...
        InputCocoa* output = new InputCocoa(window, input);
        //...
        s_os_context.m_input[output]=output;
        //...
        return output;
    }
    static void delete_input(InputCocoa*& input)
    {
        //search
        auto it_input = s_os_context.m_input.find(input);
        //remove
        if (it_input != s_os_context.m_input.end())
        {
            s_os_context.m_input.erase(it_input);
        }
        //delete
        delete input;
        //to null
        input = nullptr;
    }
    // Input
    Input::Input(Window* window)
    {
        m_native = create_input((WindowCocoa*)window->conteiner(),this);
    }
    
    Input::~Input()
    {
        destoy();
    }
    
    //event pull
    void Input::pull_events()
    {
        for (;;)
        {
            NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                untilDate:[NSDate distantPast]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];
            if (event == nil)  break;
            [NSApp sendEvent:event];
        }
    }
    
    // Info
    void* Input::conteiner() const
    {
        return m_native;
    }
    
    bool Input::valid() const
    {
        return m_native != nullptr;
    }
    
    void Input::destoy()
    {
        if (m_native)
        {
            InputCocoa* input = (InputCocoa*)m_native;
            delete_input (input);
            m_native = (void*)input;
        }
    }
    // Subscrive events
    void Input::subscrive_keyboard_listener(std::function<void(KeyboardEvent kevent, short mode, ActionEvent action)> listener)
    {
        auto* input = (InputCocoa*)m_native;
        input->m_keyboard = listener;
    }
    
    void Input::subscrive_character_listener(std::function<void(int character, short mode, int plain)> listener)
    {
        auto* input = (InputCocoa*)m_native;
        input->m_character = listener;
    }
    
    void Input::subscrive_mouse_button_listener(std::function<void(MouseButtonEvent mevent, ActionEvent action)> listener)
    {
        auto* input = (InputCocoa*)m_native;
        input->m_mouse_button = listener;
    }
    
    void Input::subscrive_mouse_move_listener(std::function<void(double x, double y)> listener)
    {
        auto* input = (InputCocoa*)m_native;
        input->m_mouse_move = listener;
    }
    
    void Input::subscrive_mouse_scroll_listener(std::function<void(double scroll)> listener)
    {
        auto* input = (InputCocoa*)m_native;
        input->m_scroll = listener;
    }
    
    void Input::subscrive_touch_listener(std::function<void(FingerEvent fevent, ActionEvent action)> listener)
    {
        auto* input = (InputCocoa*)m_native;
        input->m_touch = listener;
    }
    
    void Input::subscrive_window_listener(std::function<void(WindowEvent wevent)> listener)
    {
        auto* input = (InputCocoa*)m_native;
        input->m_window = listener;
    }
    
    void Input::subscrive_window_move_listener(std::function<void(double x, double y)> listener)
    {
        auto* input = (InputCocoa*)m_native;
        input->m_window_move = listener;
    }
    ////////////////////////////////////////////////////////////////////////////////////

}
}
