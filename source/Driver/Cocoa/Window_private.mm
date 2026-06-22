//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Window_private.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

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
    struct SQUARE_API DeviceResourcesGL : public DeviceResources
    {
    public:
        
        DeviceResourcesGL(NSSquareView* view, NSOpenGLContext* glcontext, const ContextInfo& info_context)
        : m_view(view)
        , m_glcontext(glcontext)
        , m_info_context(info_context)
        {
        }
        virtual ~DeviceResourcesGL() {}
        //implement
        virtual unsigned int width() override
        {
            return m_view.frame.size.width;
        }
        virtual unsigned int height() override
        {
            return m_view.frame.size.height;
        }
        virtual const ContextInfo& get_context_info() override
		{
			return m_info_context;
		}

        
        virtual void callback_target_changed(std::function<void(DeviceResources*)> callback) override
        {
            //none
        }
        
		virtual bool get_vsync() override
        {
            return m_vsync;
        }
		virtual void set_vsync(bool vsync) override
		{ 
			GLint interval = (GLint)vsync;
            [m_glcontext setValues: (const GLint * _Nonnull)&interval forParameter: NSOpenGLCPSwapInterval];
			m_vsync = vsync;
		}

        virtual void* get_device()                       override { return (__bridge void*)m_glcontext; }
        virtual void* get_device_context(size_t i = 0) override { return (void*)nullptr; }
        virtual void* get_swap_chain()                   override { return (void*)nullptr; }
        
        virtual void* get_render_target()        override { return (void*)nullptr; }
        virtual void* get_depth_stencil_target() override { return (void*)nullptr; }
        
        virtual void* get_render_resource()        override { return (void*)nullptr; }
        virtual void* get_depth_stencil_resource() override { return (void*)nullptr; }
        
        virtual size_t number_of_device_context()  override { return 0; }
        
        NSOpenGLContext* m_glcontext;
        NSSquareView*  m_view;
		bool  m_vsync{ true };
		const ContextInfo& m_info_context;
    };
    
    // ── DeviceResourcesMetal ────────────────────────────────────────────────
    struct SQUARE_API DeviceResourcesMetal : public DeviceResources
    {
    public:
        DeviceResourcesMetal(NSView* view, id<MTLDevice> device, CAMetalLayer* layer, const ContextInfo& info)
        : m_view(view), m_metal_device(device), m_layer(layer), m_info_context(info) {}

        virtual ~DeviceResourcesMetal() {}

        virtual unsigned int width()  override { return (unsigned int)m_layer.drawableSize.width;  }
        virtual unsigned int height() override { return (unsigned int)m_layer.drawableSize.height; }
        virtual const ContextInfo& get_context_info() override { return m_info_context; }

        virtual bool get_vsync() override { return m_vsync; }
        virtual void set_vsync(bool vsync) override
        {
            m_vsync = vsync;
            m_layer.displaySyncEnabled = vsync ? YES : NO;
        }

        // get_device()         → id<MTLDevice>   (the Metal device)
        // get_device_context() → CAMetalLayer*   (the Metal layer)
        virtual void* get_device()                      override { return (__bridge void*)m_metal_device; }
        virtual void* get_device_context(size_t i = 0) override { return (__bridge void*)m_layer; }
        virtual void* get_swap_chain()                  override { return (__bridge void*)m_layer; }

        virtual void* get_render_target()          override { return nullptr; }
        virtual void* get_depth_stencil_target()   override { return nullptr; }
        virtual void* get_render_resource()        override { return nullptr; }
        virtual void* get_depth_stencil_resource() override { return nullptr; }
        virtual size_t number_of_device_context()  override { return 1; }

        // Reused as a swap-notification hook: ContextMTL registers its commit_frame lambda here.
        virtual void callback_target_changed(std::function<void(DeviceResources*)> callback) override
        {
            m_swap_fn = [this, callback](){ callback(this); };
        }

        void present_frame() { if (m_swap_fn) m_swap_fn(); }

        id<MTLDevice>   m_metal_device { nil };
        CAMetalLayer*   m_layer        { nil };
        NSView*         m_view         { nil };
        bool            m_vsync        { true };
        const ContextInfo& m_info_context;
        std::function<void()> m_swap_fn;
    };

    // ── WindowCocoa implementation ──────────────────────────────────────────
    WindowCocoa::WindowCocoa()
    {
        m_window = nullptr;
        m_view   = nullptr;
        m_context= nullptr;
        m_device = new DeviceResourcesGL(m_view,m_context,m_info.m_context);
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
        m_device = new DeviceResourcesGL(view,context,m_info.m_context);
    }

    WindowCocoa::~WindowCocoa()
    {
        if (m_is_metal)
        {
            if (m_device) delete (DeviceResourcesMetal*)m_device;
        }
        else
        {
            if (m_device) delete (DeviceResourcesGL*)m_device;
        }
        m_window = nullptr;
        m_view   = nullptr;
        m_context= nullptr;
        m_device = nullptr;
    }

    void WindowCocoa::swap() const
    {
        if (m_is_metal)
        {
            ((DeviceResourcesMetal*)m_device)->present_frame();
        }
        else
        {
            [m_context flushBuffer];
        }
    }

    void WindowCocoa::acquire_context() const
    {
        if (!m_is_metal)
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
    
    WindowCocoa* cocoa_create_metal_window(const WindowInfo& info, Window* window)
    {
        WindowCocoa* wnd = new WindowCocoa();
        wnd->m_is_metal = true;

        // Build the NSWindow + NSView (reuse the existing helper)
        cocoa_create_window(*wnd, info);

        // Now replace the GL device created by default constructor
        delete (DeviceResourcesGL*)wnd->m_device;
        wnd->m_device = nullptr;

        // Create Metal device and CAMetalLayer
        id<MTLDevice> mtl_device = MTLCreateSystemDefaultDevice();
        CAMetalLayer* metal_layer = [CAMetalLayer layer];
        metal_layer.device          = mtl_device;
        metal_layer.pixelFormat     = info.m_context.m_srgb
                                    ? MTLPixelFormatBGRA8Unorm_sRGB
                                    : MTLPixelFormatBGRA8Unorm;
        metal_layer.framebufferOnly = YES;
        metal_layer.drawableSize    = CGSizeMake(info.m_size[0], info.m_size[1]);

        // Attach layer to the NSView
        [wnd->m_view setWantsLayer:YES];
        [wnd->m_view setLayer:metal_layer];

        wnd->m_device      = new DeviceResourcesMetal(wnd->m_view, mtl_device, metal_layer, wnd->m_info.m_context);
        wnd->m_context     = nil;
        wnd->m_window_ref  = window;

        s_os_context.m_windows[wnd] = wnd;
        return wnd;
    }

    DeviceResources* WindowCocoa::device() const
    {
        if (m_is_metal)
        {
            auto* mtl = (DeviceResourcesMetal*)m_device;
            mtl->m_view = m_view;
            return mtl;
        }
        auto* gl = (DeviceResourcesGL*)m_device;
        gl->m_view = m_view;
        gl->m_glcontext = m_context;
        return gl;
    }

}
}
}
