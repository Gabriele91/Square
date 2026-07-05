#pragma once
#include "Square/Config.h"
#include "Square/Render/Drawer.h"

namespace Square
{

namespace Resource
{
    class Effect;
    class Shader;
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
        //texture panel (scrollable side box, works with forward and deferred)
        DF_DRAW_IMAGES            = 0b00100000, //loaded images (plain 2D color textures)
        DF_DRAW_TBO               = 0b01000000, //depth/cube/array textures (e.g. shadow maps)
        DF_DRAW_RBO               = 0b10000000, //render target color attachments (e.g. G-Buffer)
        DB_DRAW_ALL               = 0b00011111,
        DB_DRAW_TEXTURES          = 0b11100000
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
        // Texture panel (DF_DRAW_IMAGES/TBO/RBO): which side of the screen
        enum PanelSide : unsigned char
        {
            PANEL_RIGHT,
            PANEL_LEFT
        };
        void panel_side(PanelSide side) { m_panel_side = side; }
        PanelSide panel_side() const { return m_panel_side; }
        // Texture panel scroll (e.g. wire it to the mouse wheel)
        void panel_scroll(float delta) { m_panel_scroll += delta; }

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
        //texture panel
        Shared<Resource::Shader>    m_shader_texture_2D;
        Shared<Resource::Shader>    m_shader_texture_cube;
        Shared<Resource::Shader>    m_shader_texture_array;
        Shared<Render::Mesh>        m_mesh_quad;
        PanelSide                   m_panel_side{ PANEL_RIGHT };
        float                       m_panel_scroll{ 0.0f };
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
        //texture panel: images/TBO/RBO taken from the driver registries,
        //drawn as thumbnails in a scrollable side box
        void draw_texture_panel(const Camera& camera);
    };
}
}