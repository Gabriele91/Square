//
//  Wrapper_private.m
//  Square
//
//  Created by Gabriele Di Bari on 31/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Wrapper_private.h"
#include "Screen_private.h"
#include "Window_private.h"
#include "Input_private.h"


///////////////////////////////////////////////////////////////////////////////
// COCOA APPLICATION CLASS
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
    Square::Video::Cocoa::cocoa_send_close_event_to_all_windows();
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

namespace Square
{
namespace Video
{
namespace Cocoa
{
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Context
    WrapperContext s_os_context;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    // VIDEO
    //init key map
    void helper_input_init()
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
    void helper_screen_init()
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
            CGSize size;
            size.width = CGDisplayPixelsWide(displays[i]);
            size.height = CGDisplayPixelsHigh(displays[i]);
            //-----------------------------------------------------------
            s_os_context.m_screens.push_back(ScreenCocoa{ displays[i], unit_number, size });
            //-----------------------------------------------------------
        }
        
        free(displays);
    }
    
    void cocoa_send_close_event_to_all_windows()
    {
        for(auto input : s_os_context.m_input)
        {
            input.second->send_window_event(WindowEvent::CLOSE);
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////
    // WINDOW
    NSUInteger cocoa_display_style(const WindowInfo& info)
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
        //Version
        if(info.m_context.m_version[0] == 3)
        {
            attributes[i++] = NSOpenGLPFAOpenGLProfile;
            attributes[i++] = static_cast<NSOpenGLPixelFormatAttribute>(NSOpenGLProfileVersion3_2Core);
        }
        else if(info.m_context.m_version[0] >= 4)
        {
            attributes[i++] = NSOpenGLPFAOpenGLProfile;
            attributes[i++] = static_cast<NSOpenGLPixelFormatAttribute>(NSOpenGLProfileVersion4_1Core);
        }
        //else legacy
        attributes[i] = 0;
        
        NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
        NSOpenGLContext* open_gl_context = [[NSOpenGLContext alloc]  initWithFormat:pixelFormat shareContext:nil];
        //swap interval
        GLint value = 1;
        [open_gl_context setValues:&value forParameter:NSOpenGLCPSwapInterval];
        /////////////////////////
        return open_gl_context;
    }
    
    bool cocoa_create_window(WindowCocoa& wnd_cocoa,const WindowInfo& info)
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
    
    WindowCocoa* window_create(const WindowInfo& info, Window* window)
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
    
    void window_delete(WindowCocoa*& window)
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
    
    ////////////////////////////////////////////////////////////////////////////////////
    // LISTENER
    void cocoa_move_listener(WindowCocoa* wnd_cocoa, InputCocoa* ln_cocoa)
    {
        if(ln_cocoa->m_window_ref == wnd_cocoa)
        {
            //close
            [ln_cocoa->m_listener close];
            //change
            [ln_cocoa->m_listener listen:ln_cocoa :wnd_cocoa->m_window];
        }
    }
    InputCocoa* create_input(WindowCocoa* window, Input* input)
    {
        //...
        InputCocoa* output = new InputCocoa(window, input);
        //...
        s_os_context.m_input[output]=output;
        //...
        return output;
    }
    void delete_input(InputCocoa*& input)
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
}
    ////////////////////////////////////////////////////////////////////////////////////
    // Video
    void init()
    {
        //ini key map
        Cocoa::helper_input_init();
        //init screen
        Cocoa::helper_screen_init();
        //init application
        //Shadred
        Cocoa::s_os_context.m_application = [NSSquareApplication sharedApplication];
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
        for (auto inp_it : Cocoa::s_os_context.m_input)
        {
            inp_it.second->m_input_ref->destoy();
        }
        for (auto wnd_it : Cocoa::s_os_context.m_windows)
        {
            wnd_it.second->m_window_ref->destoy();
        }
        Cocoa::s_os_context.m_input.clear();
        Cocoa::s_os_context.m_windows.clear();
        //clear all
        Cocoa::s_os_context.m_screens.clear();
        //close
        Cocoa::s_os_context.m_application = nil;
    }
    ////////////////////////////////////////////////////////////////////////////////////
    // Screen
    unsigned int Screen::count()
    {
        return (unsigned int)(Cocoa::s_os_context.m_screens.size());
    }
    
    Screen Screen::by_index(unsigned int index)
    {
        Screen output;
        output.m_native = &Cocoa::s_os_context.m_screens[index];
        return output;
    }
    
    Screen::Screen() { /* void */ }
    
    Screen::~Screen() { /* void */ }
    
    void* Screen::conteiner() const
    {
        return m_native;
    }
    
    void Screen::get_size(unsigned int& width, unsigned int& height) const
    {
        width  = (unsigned int)((Cocoa::ScreenCocoa*)m_native)->m_size[0];
        height = (unsigned int)((Cocoa::ScreenCocoa*)m_native)->m_size[1];
    }
    
    ////////////////////////////////////////////////////////////////////////////////////
    // Window
    Window::Window(const WindowInfo& info)
    {
        m_native = Cocoa::window_create(info, this);
    }
    
    Window::~Window()
    {
        destoy();
    }
    
    //native
    void* Window::native() const
    {
        //cast
        const auto* window = (const Cocoa::WindowCocoa*)m_native;
        //
        return (__bridge void*)window->narive();
    }
    
    void* Window::conteiner() const
    {
        return m_native;
    }
    
	DeviceResources*  Window::device() const
	{
		return nullptr;
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
            auto* window = (Cocoa::WindowCocoa*)m_native;
            Cocoa::window_delete(window);
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
        const auto* window = (const Cocoa::WindowCocoa*)m_native;
        //
        window->swap();
    }
    
    void Window::acquire_context()
    {
        //cast
        const auto* window = (const Cocoa::WindowCocoa*)m_native;
        //
        window->acquire_context();
    }
    
    //info
    bool   Window::is_fullscreen()
    {
        //cast
        const auto* window = (const Cocoa::WindowCocoa*)m_native;
        //
        return window->is_fullscreen();
    }
    
    bool   Window::is_resizable()
    {
        //cast
        const auto* window = (const Cocoa::WindowCocoa*)m_native;
        //
        return window->is_resizable();
    }
    
    void   Window::get_size(unsigned int& w, unsigned int& h)
    {
        //cast
        const auto* window = (const Cocoa::WindowCocoa*)m_native;
        //
        unsigned int size[2];
        window->get_size(size);
        w = size[0];
        h = size[1];
    }
    
    void  Window::get_position(int& x, int& y)
    {
        //cast
        const auto* window = (const Cocoa::WindowCocoa*)m_native;
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
        auto* window = (Cocoa::WindowCocoa*)m_native;
        //
        unsigned int size[2]{ w, h };
        window->set_size(size);
    }
    
    void   Window::set_position(int x, int y)
    {
        //cast
        auto* window = (Cocoa::WindowCocoa*)m_native;
        //
        int position[2]{ x,y };
        window->set_position(position);
    }
    
    void   Window::enable_resize(bool enable)
    {
        //cast
        auto* window = (Cocoa::WindowCocoa*)m_native;
        //
        window->enable_resize(enable);
    }
    
    void   Window::enable_fullscreen(bool enable)
    {
        //cast
        auto* window = (Cocoa::WindowCocoa*)m_native;
        //
        window->enable_fullscreen(enable);
    }
    
    ////////////////////////////////////////////////////////////////////////////////////
    // Input
    Input::Input(Window* window)
    {
        m_native = create_input((Cocoa::WindowCocoa*)window->conteiner(),this);
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
            Cocoa::InputCocoa* input = (Cocoa::InputCocoa*)m_native;
            delete_input (input);
            m_native = (void*)input;
        }
    }
    // Subscrive events
    void Input::subscrive_keyboard_listener(std::function<void(KeyboardEvent kevent, short mode, ActionEvent action)> listener)
    {
        auto* input = (Cocoa::InputCocoa*)m_native;
        input->m_keyboard = listener;
    }
    
    void Input::subscrive_character_listener(std::function<void(int character, short mode, int plain)> listener)
    {
        auto* input = (Cocoa::InputCocoa*)m_native;
        input->m_character = listener;
    }
    
    void Input::subscrive_mouse_button_listener(std::function<void(MouseButtonEvent mevent, ActionEvent action)> listener)
    {
        auto* input = (Cocoa::InputCocoa*)m_native;
        input->m_mouse_button = listener;
    }
    
    void Input::subscrive_mouse_move_listener(std::function<void(double x, double y)> listener)
    {
        auto* input = (Cocoa::InputCocoa*)m_native;
        input->m_mouse_move = listener;
    }
    
    void Input::subscrive_mouse_scroll_listener(std::function<void(double scroll)> listener)
    {
        auto* input = (Cocoa::InputCocoa*)m_native;
        input->m_scroll = listener;
    }
    
    void Input::subscrive_touch_listener(std::function<void(FingerEvent fevent, ActionEvent action)> listener)
    {
        auto* input = (Cocoa::InputCocoa*)m_native;
        input->m_touch = listener;
    }
    
    void Input::subscrive_window_listener(std::function<void(WindowEvent wevent)> listener)
    {
        auto* input = (Cocoa::InputCocoa*)m_native;
        input->m_window = listener;
    }
    
    void Input::subscrive_window_move_listener(std::function<void(double x, double y)> listener)
    {
        auto* input = (Cocoa::InputCocoa*)m_native;
        input->m_window_move = listener;
    }
}
}
