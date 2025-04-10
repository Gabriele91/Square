//
//  Square
//
//  Created by Gabriele Di Bari on 10/11/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include <algorithm>
#include "Square/Core/Time.h"
#include "Square/Core/Object.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Core/Application.h"
#include "Square/Driver/Render.h"
#include "Square/Scene/World.h"

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
    
	//help
	Application&     AppInterface::application() { return *Application::instance(); }
	Context&		 AppInterface::context()     { return *application().context(); }
	Allocator&		 AppInterface::allocator()   { return *application().allocator(); }
	Logger&		     AppInterface::logger()      { return *application().logger(); }
	Render::Context& AppInterface::render()      { return *application().render(); }
	Video::Window&   AppInterface::window()      { return *application().window(); }
	Video::Input&    AppInterface::input()       { return *application().input(); }
	Scene::World&    AppInterface::world()       { return *application().world(); }

    Application::Application()
    {
        //test
        square_assert(s_instance == nullptr);
        //init
        Video::init();
        //registration
        s_instance = this;
        //Self reg
        m_context.m_application = this;
        // Logger
        m_context.m_logger = get_logger_intance(LoggerType::LOGGER_OS_DEFAULT);
        // Allocator
        {
            static DefaultAllocator s_default_allocator;
            #if defined(_DEBUG)            
            static DebugAllocator s_debug_allocator(&s_default_allocator, m_context.m_logger);
            m_context.m_allocator = &s_debug_allocator;
            #else 
            m_context.m_allocator = &s_default_allocator;
            #endif
        }
		//Add static object factory
		for (const auto& item : ClassObjectRegistration::item_list())
		{
			item.m_registration(m_context);
		}
    }
    
    Application::~Application()
    {
        // Unregister items
        m_context.clear();
        // Unregister
        s_instance = nullptr;
        // Close
        Video::close();
		// Unregister
        m_context.m_application = nullptr;
        m_context.m_allocator = nullptr;
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

    Allocator* Application::allocator()
    {
        return context() ? context()->allocator() : nullptr;
    }

    Logger* Application::logger()
    {
        return context() ? context()->logger() : nullptr;
    }

    Video::Window* Application::window()
    {
        return m_window;
    }

	Video::Input* Application::input()
	{
		return m_input;
	}

	Scene::World* Application::world()
	{
		return m_world;
	}

	Context* Application::context()
	{
		return  &m_context;
	}
	
	Render::Context* Application::render()
	{
		return  m_render;
	}

    const AppInterface* Application::app_instance() const
    {
        return m_instance;
    }

    Allocator* Application::allocator() const
    {
        return context() ? context()->allocator() : nullptr;
    }

    Logger* Application::logger() const
    {
        return context() ? context()->logger() : nullptr;
    }

    const Video::Window* Application::window() const
    {
        return m_window;
    }
    
	const Video::Input* Application::input() const
	{
		return m_input;
	}

	const Scene::World* Application::world() const
	{
		return m_world;
	}

	const Context* Application::context() const
	{
		return  &m_context;
	}

	const Render::Context* Application::render() const
	{
		return  m_render;
	}
	
	static inline Video::ContextInfo::context_type get_context_type(Render::RenderDriver type)
	{
		switch (type)
		{
		case Square::Render::DR_VULKAN:
		case Square::Render::DR_OPENGL:
			return Video::ContextInfo::CTX_OPENGL;
		break;
		case Square::Render::DR_OPENGL_ES:
			return Video::ContextInfo::CTX_OPENGL_ES;
		break;
		case Square::Render::DR_DIRECTX:
			return Video::ContextInfo::CTX_DIRECTX;
		break;
		default: 
            return Video::ContextInfo::CTX_UNKNOWN;
        break;
		}
	}

    Video::ContextInfo::gpu_type get_gpu_type(GpuType type)
    {
        switch (type)
        {
        default:
        case Square::GpuType::GPU_DEFAULT: 
            return Video::ContextInfo::GPU_TYPE_DEFAULT;

        case Square::GpuType::GPU_HIGH:
        case Square::GpuType::GPU_AMD:
        case Square::GpuType::GPU_NV:
            return Video::ContextInfo::GPU_TYPE_HIGH;

        case Square::GpuType::GPU_INTEL:
        case Square::GpuType::GPU_LOW:
            return Video::ContextInfo::GPU_TYPE_LOW;
        }
    }

    bool Application::execute
    (
         const WindowSize& size,
         WindowMode mode,
		 WindowRenderDriver driver,
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
		winfo.m_context.m_type = get_context_type(driver.m_type);
		winfo.m_context.m_color = 32;
		winfo.m_context.m_depth = driver.m_depth_ctx;
		winfo.m_context.m_stencil = driver.m_stencil_ctx;
		winfo.m_context.m_version[0] = driver.m_major_ctx;
		winfo.m_context.m_version[1] = driver.m_minor_ctx;
        winfo.m_context.m_gpu_type = get_gpu_type(driver.m_gpu_type);
        winfo.m_context.m_debug = driver.m_debug;
        winfo.m_size[0] = window_size.x;
        winfo.m_size[1] = window_size.y;
        winfo.m_fullscreen = mode == WindowMode::FULLSCREEN;
        winfo.m_resize     = mode == WindowMode::RESIZABLE;
        m_window = SQ_NEW(allocator(), Video::Window, AllocType::ALCT_DEFAULT) Video::Window(winfo);
        //input
        m_input = SQ_NEW(allocator(), Video::Input, AllocType::ALCT_DEFAULT) Video::Input(m_window);
        //save instance
        m_instance = app;
        ////////////////////////////////////////////////////////////////////////        
        //center
        m_window->set_position((screen_width - window_size.x) / 2,
                               (screen_height - window_size.y) / 2);
        //enable render context and  disable vSync (auto by Video::Window)
        m_window->acquire_context();
		//Get render
		m_render = Render::create_render_driver(context()->allocator(), context()->logger(), driver.m_type);
		//init render
		if (!m_render || !m_render->init(m_window->device()))
		{
            logger()->error("Unable to load render driver");
			return false;
		}
        //flush errors
		m_render->print_errors();
        //show info
		m_render->print_info();
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

        //set world
        m_world = SQ_NEW(allocator(), Scene::World, AllocType::ALCT_DEFAULT) Scene::World(m_context);

        //start
        m_instance->start();
		m_render->print_errors();

        //time
        double old_time = 0;
        double last_time = Time::get_ms_time();
        m_last_delta_time = 0;

        //send event
        {
            static std::string init_finished ("application::init::finished" );
            static const VariantRef init_finished_ref(init_finished);
            m_world->send_message(init_finished_ref, true);
        }

        //loop
        while (!close_event)
        {
            //compute delta time
            old_time = last_time;
            last_time = Time::get_ms_time();
            //print
			m_render->print_errors();
            //update delta time
            m_last_delta_time = std::max(last_time - old_time,  1E-16) / 1000.0;
            //update
            if (!m_instance->run(m_last_delta_time)) break;
            //print
			m_render->print_errors();
            //update window
            Video::Input::pull_events();
            //swap
            m_window->swap();
        }

        //send event
        {
            static std::string loop_finished("application::loop::finished");
            static const VariantRef loop_finished_ref(loop_finished);
            m_world->send_message(loop_finished_ref, true);
        }

        //end state
        bool end_state = m_instance->end();

        //dealloc input
        SQ_DELETE_NAMESPACE(allocator(), Video, Input, m_input);
        m_input = nullptr;

        //clear context
        context()->clear();

        //dealloc
        delete m_instance;
        m_instance = nullptr;

        //delete world
        SQ_DELETE_NAMESPACE(allocator(), Scene, World, m_world);
        m_world = nullptr;
        
        //delete render context
        m_render->close();
        Render::delete_render_driver(m_render);

        //dealloc window
        SQ_DELETE_NAMESPACE(allocator(), Video, Window, m_window);
        m_window = nullptr;
        
        //return status
        return end_state;
    }
}
