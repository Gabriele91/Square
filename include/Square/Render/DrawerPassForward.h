//
//  DrawerPassForward.h
//  Square
//
//  Created by Gabriele Di Bari on 25/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Render/Drawer.h"

namespace Square
{
namespace Render
{
    class SQUARE_API DrawerPassForward : public DrawerPass
    {
    public:
        //passo
        DrawerPassForward(Square::Context& context);
        //disegna
        virtual void draw
        (
          Drawer& drawer
        , int num_of_pass
        , const Vec4&  clear_color
        , const Vec4&  ambient_color
        , const Camera& camera
        , const Collection& collection
        , const PoolQueues& queues
        )
        override;
    
    protected:
        //context
        Square::Context& context();
        const Square::Context& context() const;
        //render
        Render::Context& render();
        const Render::Context& render() const;
        //CPU DATA
        Square::Context& m_context;
        //GPU DATA
        Shared<Render::ConstBuffer> m_cb_camera;
		Shared<Render::ConstBuffer> m_cb_transform;
		Shared<Render::ConstBuffer> m_cb_direction_light;
		Shared<Render::ConstBuffer> m_cb_point_light;
		Shared<Render::ConstBuffer> m_cb_spot_light;

    };
}
}
