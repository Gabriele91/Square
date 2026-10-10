#pragma once
#include "Square/Config.h"
#include "Square/Render/Pipeline/Drawer.h"

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
    enum DebugFlags : unsigned short
    {
        DF_DRAW_OBB               = 0b00000001,
        DF_DRAW_FUSTRUM           = 0b00000010,
        DF_DRAW_SPOT_LIGHT        = 0b00000100,
        DF_DRAW_POINT_LIGHT       = 0b00001000,
        //the software occlusion of the camera: its depth buffer in a corner (the occluders: the
        //lighter the nearer), the boxes it hides (red, seen through what hides them)
        DF_DRAW_OCCLUSION         = 0b100000000,
        DB_DRAW_ALL               = 0b00001111
    };

    class SQUARE_API DrawerPassDebug : public DrawerPass
    {
    public:
        //passo
        DrawerPassDebug(Square::Context& context, unsigned short flags = DebugFlags::DB_DRAW_ALL);
        virtual ~DrawerPassDebug();
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
        void draw_flags(unsigned short flags) { m_flags = flags; }
        unsigned short draw_flags() const { return m_flags; }

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
        Shared<Render::Mesh>        m_mesh_frustum; //box with z in [0,1], the clip space depth range
        Shared<Render::Mesh>        m_mesh_sphere;  //point light volume (shared with the deferred pass)
        Shared<Render::Mesh>        m_mesh_cone;    //spot light volume (shared with the deferred pass)
        //the depth buffer of the occlusion: a 2D texture on a quad
        Shared<Resource::Shader>    m_shader_texture_2D;
        Shared<Render::Mesh>        m_mesh_quad;
        //Draw Flags
        unsigned short              m_flags{ DebugFlags::DB_DRAW_ALL };
        //the depth buffer of the software occlusion as an image (DF_DRAW_OCCLUSION)
        Render::Texture*            m_occlusion_texture{ nullptr };
        IVec2                       m_occlusion_size{ 0, 0 };
        std::vector<unsigned char>  m_occlusion_pixels;
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
        void draw_light_volume
        (
              const Camera& camera
            , const Mat4& model
            , const Shared<Render::Mesh>& mesh
            , const Vec4& color
        );
        //the software occlusion of the camera (DF_DRAW_OCCLUSION): the boxes it hides, its depth
        //buffer in the bottom left corner
        void draw_occlusion(const Drawer& drawer, const Camera& camera, const Collection& collection);
        void draw_occlusion_image(const SoftwareOcclusion& occlusion, const Camera& camera);
    };
}
}