//
//  Drawer.h
//  Square
//
//  Created by Gabriele Di Bari on 24/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Object.h"
#include "Square/Core/Context.h"
#include "Square/Render/GBuffer.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Collection.h"

namespace Square
{
namespace Render
{
    //Class/enum declare
    class Drawer;
    class DrawerPass;
    //define types
    enum DrawerPassType
    {
        RPT_SHADOW,
        RPT_RENDER,
        RPT_UI,
        RPT_MAX
    };
    //define interface
    class SQUARE_API DrawerPass : public BaseObject
                                , public SharedObject<DrawerPass>
    {
    public:
        SQUARE_OBJECT(DrawerPass)

        DrawerPass(DrawerPassType type) : m_type(type) {}
        
		virtual void draw
		(
			  Drawer&           drawer
			, int               num_of_pass
			, const Vec4&       clear_color
			, const Vec4&       ambient_color
			, const Camera&     camera
			, const Collection& collection
			, const PoolQueues& queues
		) {};

		virtual void draw
		(
			  Drawer&           drawer
			, int               num_of_pass
			, const Vec4&       clear_color
			, const Vec4&       ambient_color
			, const Light&      light
			, const Collection& collection
			, const PoolQueues& queues
		) {};

        DrawerPassType type()
        {
            return m_type;
        }
        
    private:
        
        DrawerPassType m_type;
    };
    
    //drawer
    class SQUARE_API Drawer : public BaseObject
    {
    public:
        SQUARE_OBJECT(Drawer)
        //Init
        Drawer(Square::Context& context);
        
        //context
        Square::Context& context();
        const Square::Context& context() const;

        Render::Context& render();
        const Render::Context& render() const;
        
        //draw
        void draw( const Collection& collection );
        void draw( const Vec4& clear_color, const Vec4& ambient_color, const Collection& collection);
        
        //add a pass
        void add(Shared<DrawerPass> pass);
        
        //gpu buffer
        Shared<Render::ConstBuffer>& transform_buffer();
        Shared<Render::ConstBuffer>& camera_buffer();

    protected:
        
        //CPU DATA
        Square::Context& m_context;
        std::vector< Shared<DrawerPass> > m_rendering_pass[RPT_MAX];
        //GPU DATA
        Shared<Render::ConstBuffer> m_transform;
        Shared<Render::ConstBuffer> m_camera;

    };
}
}
