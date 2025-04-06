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
    class SQUARE_API RenderableIterator
    {
    public:
        RenderableIterator& operator++();

        bool operator == (const RenderableIterator& other) const;
        bool operator != (const RenderableIterator& other) const;

        Shared<Render::Renderable> operator*();
        Shared<Render::Renderable> operator*() const;

    private:
        RenderableIterator(const PoolQueues& queues, const std::array<bool, RQ_MAX>& queues_types, QueueType queue_current);
        void test_it_or_queue_next();

        const PoolQueues&               m_queues;
        const std::array<bool, RQ_MAX>& m_queues_types;
        QueueType                       m_queue_current;
        QueueIterator                   m_queue_it;
        
        friend class  RenderableQuery;
    };

    class SQUARE_API RenderableQuery
    {
    public:
        RenderableQuery(const PoolQueues& queues, const std::vector<QueueType>& select_queues);
        const RenderableIterator& begin() const;
        const RenderableIterator& end() const;
    private:
        std::array<bool, RQ_MAX> m_queues_types{ false };
        RenderableIterator m_begin;
        RenderableIterator m_end;
    };

    //Class/enum declare
    class Drawer;
    class DrawerPass;
    //define types
    enum DrawerPassType : unsigned char
    {
        RPT_RENDER = 0b0001, // 0
        RPT_SHADOW = 0b0010, // 1
        RPT_UI     = 0b0100, // 2
        RPT_DEBUG  = 0b1000, // 3
        RPT_ALL    = 0b1111, // -
        RPT_MAX    = 4       // ARRAY SIZE
    };
    //define interface
    class SQUARE_API DrawerPass : public BaseObject
                                , public SharedObject<DrawerPass>
    {
    public:
        SQUARE_OBJECT(DrawerPass)

        DrawerPass(Allocator* allocator, DrawerPassType type) : SharedObject_t(allocator), m_type(type) {}
        
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
            , const Camera&     camera
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
        void draw( const Collection& collection, unsigned char pass_types = DrawerPassType::RPT_ALL);
        void draw( const Vec4& clear_color, 
                   const Vec4& ambient_color, 
                   const Collection& collection,
                   unsigned char pass_types = DrawerPassType::RPT_ALL);
        
        //add a pass
        void add(Shared<DrawerPass> pass);

        template<class T, typename ...ARGS>
        Shared<T> create(ARGS&&... args)
        {
            auto t_shared_ptr = MakeShared<T>(context(), std::forward(args)...);
            add(t_shared_ptr);
            return t_shared_ptr;
        }

    protected:
        
        //CPU DATA
        Square::Context& m_context;
        std::vector< Shared<DrawerPass> > m_rendering_pass[RPT_MAX];

    };
}
}
