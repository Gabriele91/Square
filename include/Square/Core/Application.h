//
//  Square
//
//  Created by Gabriele Di Bari on 10/11/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Context.h"
#include "Square/Driver/Input.h"
#include "Square/Driver/Window.h"
#include "Square/Driver/Render.h"

namespace Square
{
    struct WindowSize;
    struct WindowSizePixel;
    struct WindowSizePercentage;
    class Application;
    class Instance;
    
    
    struct SQUARE_API WindowSize
    {
        virtual IVec2 get_size(const Video::Screen& screen) const = 0;
    };
    
    struct SQUARE_API WindowSizePixel : WindowSize
    {
        IVec2 m_size;
        
        WindowSizePixel(const IVec2& size);
        virtual IVec2 get_size(const Video::Screen& screen) const override;
    };
    
    struct SQUARE_API WindowSizePercentage : WindowSize
    {
        DVec2 m_size;
        
        WindowSizePercentage(const DVec2& size);
        virtual IVec2 get_size(const Video::Screen& screen) const override;
    };
    
    class SQUARE_API AppInterface
    {
    public:
        virtual void key_event(Video::KeyboardEvent key,  short mode, Video::ActionEvent action) { }

        virtual void mouse_move_event(const DVec2& pos) { }
        virtual void mouse_button_event(Video::MouseButtonEvent button,  Video::ActionEvent action) { }
        virtual void mouse_scroll_event(double scroll) { }

        virtual void window_event(Video::WindowEvent event) { }

        virtual void start() = 0;
        virtual bool run(double delta_time) = 0;
        virtual bool end() = 0;
        
        virtual ~AppInterface() {}

		//help
		static Application&     application();
		static Context&		    context();
		static Allocator&       allocator();
		static Logger&		    logger();
		static Render::Context& render();
		static Video::Window&   window();
		static Video::Input&    input();
		static Scene::World&    world();
    };
    
    enum class WindowMode
    {
        RESIZABLE,
        NOT_RESIZABLE,
        FULLSCREEN
    };

    enum class GpuType 
    {
        GPU_DEFAULT,
        GPU_AMD,
        GPU_NV,
        GPU_INTEL,
        GPU_HIGTH,
        GPU_LOW
    };

	struct SQUARE_API WindowRenderDriver
	{
		Render::RenderDriver m_type{ Render::DR_OPENGL };
		int m_major_ctx{ 4 };
		int m_minor_ctx{ 1 };
		int m_depth_ctx  { 24 };
		int m_stencil_ctx{ 8  };
        GpuType m_gpu_type{ GpuType::GPU_DEFAULT };
		bool m_debug{ false };
	};
        
    class SQUARE_API Application
    {
    public:
        //application
        Application();
        ~Application();
        //actions
        void clear() const;
        void swap() const;
        //info
        bool   fullscreen() const;
        void   fullscreen(bool enable) const;
        bool   resizable() const;
        void   resizable(bool enable) const;
        //info
        IVec2  screen_size() const;
        double last_delta_time() const;
        //size
        IVec2  window_size() const;
        void   window_size(const IVec2& size) const;
        //position
        IVec2  window_position() const;
        void   window_position(const IVec2& size) const;
        //get attr
        AppInterface*    app_instance();
        Allocator*       allocator();
        Logger*          logger();
        Video::Window*   window();
		Video::Input*    input();
		Scene::World*    world();
		//ctx info
		Context*	     context();
		Render::Context* render();
        //get attr (const)
        const AppInterface*   app_instance() const;
        Allocator*            allocator() const;
        Logger*               logger() const;
        const Video::Window*  window() const;
		const Video::Input*   input() const;
		const Scene::World*   world() const;
		//ctx info (const)
		const Context*	     context() const;
		const Render::Context* render() const;
		//execute a instance
        bool execute
        (
            const WindowSize& size,
            WindowMode mode,
			WindowRenderDriver driver,
            const std::string& app_name,
            AppInterface* app,
            size_t n_workers = 4
        );
        
        //sigleton
		static Application*     instance();

    private:
        //info screen
        Video::Window*      m_window{ nullptr };
		Video::Input*       m_input{ nullptr };
		Scene::World*       m_world{ nullptr };
		//info instance
        AppInterface*       m_instance{ nullptr };
		//context (delta of update)
        double              m_last_delta_time{ 0 };
		//context (application context)
		Context				m_context;
		Render::Context*	m_render;
        //global
        static Application* s_instance;
    };
}
