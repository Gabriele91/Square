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
    class SQUARE_API WindowCocoa
    {
    public:
        WindowInfo       m_info;
        NSSquareWindow*  m_window    { nullptr };   /* window */
        NSSquareView*    m_view      { nullptr };   /* view */
        NSOpenGLContext* m_context   { nullptr };   /* OpenGL context */
        Window*          m_window_ref{ nullptr };
        
        WindowCocoa();
        
        WindowCocoa(  NSSquareWindow* window
                    , NSSquareView* view
                    , NSOpenGLContext* context
                    , const WindowInfo& info
                    );
        
        ~WindowCocoa();
        
        virtual void swap() const;
        
        virtual void acquire_context() const;
        
        virtual bool is_fullscreen() const;
        
        virtual bool is_resizable() const;
        
        virtual void get_size(unsigned int size[2]) const;
        
        virtual void get_position(int position[2]) const;
        
        virtual void set_size(unsigned int size[2]);
        
        virtual void set_position(int position[2]);
        
        virtual void set_mouse_position(int position[2]);
        
        virtual bool enable_resize(bool enable);
        
        virtual bool enable_fullscreen(bool enable);
        
        virtual NSSquareView* narive() const;
    };
}
}
}
