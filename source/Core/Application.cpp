//
//  Square
//
//  Created by Gabriele Di Bari on 10/11/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include <algorithm>
#include "Square/Core/Application.h"
#include "Square/Core/Time.h"
#include "Square/Driver/Render.h"

namespace Square
{
    ///Global instance
    Application* Application::s_instance = nullptr;
    //singleton
    Application* Application::instance()
    {
        return s_instance;
    }
    /////
    WindowSizePixel::WindowSizePixel(const IVec2& size)
    {
        m_size = size;
    }
    
    IVec2 WindowSizePixel::get_size(const Video::Screen& screen) const
    {
        return m_size;
    }
    
    WindowSizePercentage::WindowSizePercentage(const DVec2& size)
    {
        m_size = size;
    }
    
    IVec2 WindowSizePercentage::get_size(const Video::Screen& screen) const
    {
        unsigned int width = 0, height = 0;
        screen.get_size(width, height);
        
        return IVec2
        {
            (int)((double)width * m_size.x / 100.0),
            (int)((double)height * m_size.y / 100.0)
        };
    }
    
    Application::Application()
    {
        //test
        assert(s_instance == nullptr);
        //init
        Video::init();
        //registration
        s_instance = this;
    }
    
    Application::~Application()
    {
        //unregister
        s_instance = nullptr;
        //close
        Video::close();
    }
    
    void Application::clear() const
    {
        m_window->clear();
    }
    
    void Application::swap() const
    {
        m_window->swap();
    }
    
    bool Application::fullscreen() const
    {
        return m_window->is_fullscreen();
    }
    
    void Application::fullscreen(bool enable) const
    {
        return m_window->enable_fullscreen(enable);
    }
    
    bool Application::resizable() const
    {
        return m_window->is_resizable();
    }
    
    void Application::resizable(bool enable) const
    {
        return m_window->enable_resize(enable);
    }
    
    double Application::last_delta_time() const
    {
        return m_last_delta_time;
    }
    
    IVec2 Application::screen_size() const
    {
        unsigned int width = 0, height = 0;
        Video::Screen::by_index(0).get_size(width, height);
        //return
        return IVec2(int(width),int(height));
    }
    
    IVec2 Application::window_size() const
    {
        
        unsigned int width = 0, height = 0;
        m_window->get_size(width, height);
        return IVec2(int(width),int(height));
    }
    
    void Application::window_size(const IVec2& size) const
    {
        m_window->set_size((unsigned int)size.x, (unsigned int)size.y);
    }
    
    IVec2 Application::window_position() const
    {
        int x, y;
        m_window->get_position(x, y);
        return IVec2(x, y);
    }
    
    void Application::window_position(const IVec2& position) const
    {
        m_window->set_position(position.x, position.y);
    }
    
    
    //get attr
    AppInterface* Application::app_instance()
    {
        return m_instance;
    }
    
    Video::Window* Application::window()
    {
        return m_window;
    }
    
    Video::Input* Application::input()
    {
        return m_input;
    }
    
    const AppInterface* Application::app_instance() const
    {
        return m_instance;
    }
    
    const Video::Window* Application::window() const
    {
        return m_window;
    }
    
    const Video::Input* Application::input() const
    {
        return m_input;
    }

    bool Application::execute
    (
         const WindowSize& size,
         WindowMode mode,
         int major_gl_ctx,
         int minor_gl_ctx,
         const std::string& app_name,
         AppInterface* app,
         size_t n_workers
    )
    {
        ////////////////////////////////////////////////////////////////////////
        //Screen
        Video::Screen screen = Video::Screen::by_index(0);
        unsigned int screen_width = 0, screen_height = 0;
        screen.get_size(screen_width, screen_height);
        //compute window size
        IVec2 window_size = size.get_size(screen);
        //window
        Video::WindowInfo winfo(&screen);
        winfo.m_context.m_color = 32;
        winfo.m_context.m_version[0] = major_gl_ctx;
        winfo.m_context.m_version[1] = minor_gl_ctx;
        winfo.m_size[0] = window_size.x;
        winfo.m_size[1] = window_size.y;
        winfo.m_fullscreen = mode == WindowMode::FULLSCREEN;
        winfo.m_resize     = mode == WindowMode::RESIZABLE;
        m_window = new Video::Window(winfo);
        //input
        m_input = new Video::Input(m_window);
        //save instance
        m_instance = app;
        ////////////////////////////////////////////////////////////////////////
        
        //center
        m_window->set_position((screen_width - window_size.x) / 2,
                               (screen_height - window_size.y) / 2);
        //enable render context and  disable vSync (auto by Video::Window)
        m_window->acquire_context();
		//init render
		if (!Render::init()) return false;
        //flush errors
        Render::print_errors();
        //show info
        Render::print_info();
        //close event
        bool close_event = false;
        //set events
        m_input->subscrive_keyboard_listener([this](Video::KeyboardEvent key,  short mode, Video::ActionEvent action)
        {
            app_instance()->key_event(key, mode, action);
        });
        
        m_input->subscrive_mouse_move_listener([this](double x, double y)
        {
            app_instance()->mouse_move_event(DVec2(x,y));
        });
        
        m_input->subscrive_mouse_button_listener([this](Video::MouseButtonEvent button, Video::ActionEvent action)
        {
            app_instance()->mouse_button_event(button, action);
        });
        
        m_input->subscrive_mouse_scroll_listener([this](double scroll)
        {
            app_instance()->mouse_scroll_event(scroll);
        });
        
        m_input->subscrive_window_listener([this,&close_event](Video::WindowEvent event)
        {
           app_instance()->window_event(event);
            //close event?
            if(event == Video::WindowEvent::CLOSE) close_event = true;
        });
        //start
        m_instance->start();
        Render::print_errors();
        //time
        double old_time = 0;
        double last_time = Time::get_time();
        //loop
        while (!close_event)
        {
            //compute delta time
            old_time = last_time;
            last_time = Time::get_time();
            //print
            Render::print_errors();
            //update delta time
            m_last_delta_time = std::max(last_time - old_time, 0.0001);
            //update
            if (!m_instance->run(m_last_delta_time)) break;
            //print
            Render::print_errors();
            //update window
            Video::Input::pull_events();
            //swap
            m_window->swap();
        }
        //end state
        bool end_state = m_instance->end();
        
        //dealloc input
        delete m_input;
        m_input = nullptr;
        
        //dealloc
        delete m_instance;
        m_instance = nullptr;
        
        //delete context
        Render::close();
        
        //dealloc window
        delete m_window;
        m_window =nullptr;
        
        //return status
        return end_state;
    }
}
