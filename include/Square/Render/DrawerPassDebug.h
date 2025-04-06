#pragma once
#include "Square/Config.h"
#include "Square/Render/Drawer.h"

namespace Square
{

namespace Resource
{
    class Effect;
}

namespace Render
{
    class Mesh;
}

namespace Render
{
    enum DebugFlags : unsigned char
    {
        DF_DRAW_OBB               = 0b00000001,
        DF_DRAW_FUSTRUM           = 0b00000010,
        DF_DRAW_SPOT_LIGHT        = 0b00000100,
        DF_DRAW_POINT_LIGHT       = 0b00001000,
        DF_DRAW_DIRECTIONAL_LIGHT = 0b00010000,
        DB_DRAW_ALL               = 0b00011111
    };

    class SQUARE_API DrawerPassDebug : public DrawerPass
    {
    public:
        //passo
        DrawerPassDebug(Square::Context& context, unsigned char flags = DebugFlags::DB_DRAW_ALL);
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
        // Draw flag
        void draw_flags(unsigned char flags) { m_flags = flags; }
        unsigned char draw_flags() const { return m_flags; }
    
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
        Shared<Resource::Effect>    m_debug_effect;
        Shared<Render::Mesh>        m_mesh_box;
        //Draw Flags
        unsigned char               m_flags{ DebugFlags::DB_DRAW_ALL };
        //Helps
        void draw_obb
        (
              Drawer& drawer
            , const Camera& camera
            , const Collection& collection
            , const PoolQueues& queues
        );
        void draw_fustrum
        (
              Drawer& drawer
            , const Camera& camera
            , const Mat4& view
            , const Mat4& projection
            , const Vec4& color
            , bool volume = false
        );
    };
}
}