#include "Square/Core/Context.h"
#include "Square/System/RenderSystem.h"
#include "Square/Driver/Render.h"
#include "Square/Driver/RenderInspector.h"
#include "Square/Render/Material.h"
#include "Square/Render/Effect.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Viewport.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/Transform.h"
#include "Square/Render/ShadowBuffer.h"
#include "Square/Render/Pipeline/DrawerPassDebug.h"
#include "Square/Render/Mesh.h"
#include "Square/Resource/Effect.h"
#include "Square/Resource/Shader.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Math/Transformation.h"
#include "Square/Math/Tangent.h"
#include "Square/Render/Pipeline/LightVolume.h"
#include "Square/Render/BasicMesh.h"
#include <algorithm>
#include <cmath>
#include "Square/Geometry/Intersection.h"

namespace Square
{
namespace Render
{
    enum DebugColor : unsigned char
    {
        DB_BLACK,
        DB_DARK_RED,
        DB_RED,
        DB_ORANGE,
        DB_BROWN,
        DB_YELLOW,

        DB_DARK_GREEN,
        DB_GREEN,
        DB_LIGHT_GREEN,

        DB_DARK_BLUE,
        DB_BLUE,
        DB_LIGHT_BLUE,
        DB_CYAN,

        DB_VIOLET,
        DB_MAGENTA,
        DB_PINK,

        DB_GRAY,
        DB_WHITE,
        DB_SIZE_COLORS
    };

    static const Vec4& debug_colors(DebugColor ccode)
    {
        static const Vec4 colors[]
        {
            {0.0, 0.0, 0.0, 1.0},     // BLACK
            {0.55, 0.0, 0.0, 1.0},    // DARK_RED
            {1.0, 0.0, 0.0, 1.0},     // RED
            {1.0, 0.5, 0.0, 1.0},     // ORANGE
            {0.6, 0.3, 0.0, 1.0},     // BROWN
            {1.0, 1.0, 0.0, 1.0},     // YELLOW

            {0.0, 0.39, 0.0, 1.0},    // DARK_GREEN
            {0.0, 1.0, 0.0, 1.0},     // GREEN
            {0.56, 0.93, 0.56, 1.0},  // LIGHT_GREEN

            {0.0, 0.0, 0.55, 1.0},    // DARK_BLUE
            {0.0, 0.0, 1.0, 1.0},     // BLUE
            {0.68, 0.85, 0.9, 1.0},   // LIGHT_BLUE
            {0.0, 1.0, 1.0, 1.0},     // CYAN

            {0.5, 0.0, 1.0, 1.0},     // VIOLET
            {1.0, 0.0, 1.0, 1.0},     // MAGENTA
            {1.0, 0.75, 0.8, 1.0},    // PINK

            {0.5, 0.5, 0.5, 1.0},     // GRAY
            {1.0, 1.0, 1.0, 1.0}      // WHITE
        };
        return colors[ccode % DB_SIZE_COLORS];
    }

    void DrawerPassDebug::draw_obb
    (
          Drawer& drawer
        , const Camera& camera
        , const Collection& collection
        , const PoolQueues& queues
    )
    {
        if(!m_mesh_box)
        {
            context().logger()->warning("Debug mesh does not exist");
            return;
        }
        //buffers
        Render::UniformBufferCamera ucamera;
		Render::UniformBufferTransform utransform;
		//parameters
		EffectPassInputs inputs
		{
			//render
			  m_cb_camera.get()
			, m_cb_transform.get()
			//light
			, Vec4()
			, nullptr
			, nullptr
			, nullptr
			//shadow
			, nullptr
			, nullptr
			, nullptr
			, nullptr
		};
        //update camera
        camera.set(&ucamera);
        render().update_steam_CB(m_cb_camera.get(), (const unsigned char*)&ucamera, sizeof(ucamera));
        // Get obb debug technique
        auto* wireframe  = m_debug_effect->technique("wireframe");
        auto* line_color = m_debug_effect->parameter("line_color");
        if (!wireframe || !line_color)
        {
            context().logger()->warning("Unable to find wireframe debug technique");
            return;
        }
        // Colors / size
        line_color->set(Vec4(1.0, 1.0, 1.0, 1.0));
        //for each elements of opaque  and translucent queues
		for(auto randerable : RenderableQuery(queues, { RQ_OPAQUE, RQ_TRANSLUCENT }))
        if (randerable && randerable->can_draw() && randerable->support_culling())
        {
            // OBB
            auto& obb = randerable->bounding_box();
            utransform.m_position = obb.get_position();
            utransform.m_rotation = obb.get_rotation_matrix();
            utransform.m_scale    = obb.get_extension();
            // T*R*S
            utransform.m_model  = Square::translate(Constants::identity<Mat4>(), utransform.m_position);
            utransform.m_model *= utransform.m_rotation;
            utransform.m_model  = Square::scale(utransform.m_model, utransform.m_scale * 1.001);
            utransform.m_inv_model = Square::inverse(utransform.m_model);
            // Update UCB
            render().update_steam_CB(m_cb_transform.get(), (const unsigned char*)&utransform, sizeof(utransform));
            // Draw
            for (auto& pass : *wireframe)
            {
                pass.bind(render(), inputs, m_debug_effect->parameters());
                m_mesh_box->draw(render());
                pass.unbind();
            }
        }
    }
      
    void DrawerPassDebug::draw_fustrum
    (
          Drawer& drawer
        , const Camera& camera
        , const Mat4& view
        , const Mat4& projection
        , const Vec4& color
        , bool volume
    )
    {
        if(!m_mesh_frustum)
        {
            context().logger()->warning("Debug mesh does not exist");
            return;
        }
        //buffers
        Render::UniformBufferCamera ucamera;
		Render::UniformBufferTransform utransform;
		//parameters
		EffectPassInputs inputs
		{
			//render
			  m_cb_camera.get()
			, m_cb_transform.get()
			//light
			, Vec4()
			, nullptr
			, nullptr
			, nullptr
			//shadow
			, nullptr
			, nullptr
			, nullptr
			, nullptr
		};
        //update camera
        camera.set(&ucamera);
        render().update_steam_CB(m_cb_camera.get(), (const unsigned char*)&ucamera, sizeof(ucamera));;
        // Set matrix
        Mat4 view_projection = projection * view;
        utransform.m_model = inverse(view_projection);
        utransform.m_inv_model = view_projection;
        render().update_steam_CB(m_cb_transform.get(), (const unsigned char*)&utransform, sizeof(utransform));
        // Effect ptr
        EffectTechnique* technique = nullptr;
        // Get obb debug technique
        if (volume)
        {
            technique = m_debug_effect->technique("volume");
            auto* mesh_color = m_debug_effect->parameter("mesh_color");
            if (!technique || !mesh_color)
            {
                context().logger()->warning("Unable to find base debug technique");
                return;
            }
            // Colors
            mesh_color->set(color);
        }
        else 
        {
            technique = m_debug_effect->technique("wireframe");
            auto* line_color = m_debug_effect->parameter("line_color");
            if (!technique || !line_color)
            {
                context().logger()->warning("Unable to find wireframe debug technique");
                return;
            }
            // Colors / size
            line_color->set(color);
        }
        // Draw wireframe
        for (auto& pass : *technique)
        {
            pass.bind(render(), inputs, m_debug_effect->parameters());
            m_mesh_frustum->draw(render());
            pass.unbind();
        }
    }

    void DrawerPassDebug::draw_light_volume
    (
          const Camera& camera
        , const Mat4& model
        , const Shared<Render::Mesh>& mesh
        , const Vec4& color
    )
    {
        if (!mesh) return;
        auto* wireframe  = m_debug_effect->technique("wireframe");
        auto* line_color = m_debug_effect->parameter("line_color");
        if (!wireframe || !line_color)
        {
            context().logger()->warning("Unable to find wireframe debug technique");
            return;
        }
        //camera + transform only
        EffectPassInputs inputs
        {
              m_cb_camera.get()
            , m_cb_transform.get()
            , Vec4()
            , nullptr
            , nullptr
            , nullptr
            , nullptr
            , nullptr
            , nullptr
            , nullptr
        };
        Render::UniformBufferCamera ucamera;
        camera.set(&ucamera);
        render().update_steam_CB(m_cb_camera.get(), (const unsigned char*)&ucamera, sizeof(ucamera));
        Render::UniformBufferTransform utransform;
        utransform.m_model     = model;
        utransform.m_inv_model = Square::inverse(model);
        render().update_steam_CB(m_cb_transform.get(), (const unsigned char*)&utransform, sizeof(utransform));
        line_color->set(color);
        for (auto& pass : *wireframe)
        {
            pass.bind(render(), inputs, m_debug_effect->parameters());
            mesh->draw(render());
            pass.unbind();
        }
    }

    DrawerPassDebug::~DrawerPassDebug()
    {
        if (m_occlusion_texture)
        {
            if (auto* render_system = System::get<RenderSystem>(context()))
            {
                if (auto* render_driver = render_system->render())
                {
                    render_driver->delete_texture(m_occlusion_texture);
                }
            }
        }
    }

    DrawerPassDebug::DrawerPassDebug(Square::Context& context, unsigned short flags)
    : DrawerPass(context.allocator(), RPT_DEBUG)
    , m_context(context)
    , m_flags(flags)
    {
		m_debug_effect = context.resource<Resource::Effect>("Debug");
        m_cb_camera    = Render::stream_constant_buffer<Render::UniformBufferCamera>(&render());
		m_cb_transform = Render::stream_constant_buffer<Render::UniformBufferTransform>(&render());
        m_mesh_box     = BasicMesh::build_box(context);
        m_mesh_frustum = BasicMesh::build_frustum_box(context);
        m_mesh_sphere  = LightVolume::build_sphere(context);
        m_mesh_cone    = LightVolume::build_cone(context);
        if (!m_mesh_box || !m_mesh_frustum)
        {
            context.logger()->warning("Unable to build debug mesh");
        }
        //the depth buffer of the occlusion
        m_shader_texture_2D    = context.resource<Resource::Shader>("DebugTexture2D");
        m_mesh_quad            = BasicMesh::build_quad(context);
        if (!m_mesh_quad)
        {
            context.logger()->warning("Unable to build debug quad mesh");
        }
    }
    //context
    Square::Context& DrawerPassDebug::context(){ return m_context; }
    const Square::Context& DrawerPassDebug::context() const { return m_context; }
    //render
    Render::Context& DrawerPassDebug::render(){ return *System::get<RenderSystem>(context())->render(); }
    const Render::Context& DrawerPassDebug::render() const { return *System::get<RenderSystem>(context())->render(); }
    //draw
    void DrawerPassDebug::draw
    (
       Drawer&           drawer
     , int               num_of_pass
     , const Vec4&       clear_color
     , const Vec4&       ambient_light
     , const Camera&     camera
     , const Collection& collection
     , const PoolQueues& queues
    )
    {
        // No debug effect?
        if (!m_debug_effect)
        {
            context().logger()->warning("Unable to load debug effect");
            return;
        }
        //start to draw
        if(num_of_pass == 0)
        {
            //set viewport (2D, On Screen)
            render().set_viewport_state({ camera.viewport().viewport() });
        }
        // Draw OBB
        if (bool(m_flags & DebugFlags::DF_DRAW_OBB))
            draw_obb(drawer, camera, collection, queues);
        if (bool(m_flags & DebugFlags::DF_DRAW_FUSTRUM))
            draw_fustrum(drawer, 
                         camera, 
                         camera.view(), 
                         camera.projection(), 
                         debug_colors(DB_GREEN),
                         false);

        if (bool(m_flags & DebugFlags::DF_DRAW_SPOT_LIGHT))
        {
            //the same cone the deferred light pass rasterizes
            for (auto weak_light : queues[RQ_SPOT_LIGHT])
                if (auto light = weak_light->lock< Render::SpotLight >())
                {
                    Render::UniformSpotLight uspot_light;
                    light->set(&uspot_light);
                    draw_light_volume(camera, LightVolume::spot_light_model(uspot_light), m_mesh_cone, debug_colors(DB_YELLOW));
                }
        }
        if (bool(m_flags & DebugFlags::DF_DRAW_POINT_LIGHT))
        {
            //the same sphere the deferred light pass rasterizes
            for (auto weak_light : queues[RQ_POINT_LIGHT])
                if (auto light = weak_light->lock< Render::PointLight >())
                {
                    Render::UniformPointLight upoint_light;
                    light->set(&upoint_light);
                    draw_light_volume(camera, LightVolume::point_light_model(upoint_light), m_mesh_sphere, debug_colors(DB_ORANGE));
                }
        }
        // The software occlusion
        if (bool(m_flags & DebugFlags::DF_DRAW_OCCLUSION))
        {
            draw_occlusion(drawer, camera, collection);
        }
    }

    void DrawerPassDebug::draw_occlusion(const Drawer& drawer, const Camera& camera, const Collection& collection)
    {
        const SoftwareOcclusion& occlusion = drawer.occlusion();
        auto* wireframe  = m_debug_effect->technique("wireframe");
        auto* line_color = m_debug_effect->parameter("line_color");
        if (occlusion.active() && wireframe && line_color && m_mesh_box)
        {
            EffectPassInputs inputs{ m_cb_camera.get(), m_cb_transform.get(), Vec4() };
            Render::UniformBufferCamera ucamera;
            camera.set(&ucamera);
            render().update_steam_CB(m_cb_camera.get(), (const unsigned char*)&ucamera, sizeof(ucamera));
            line_color->set(debug_colors(DB_RED));
            //the boxes in the frustum it hides (the wireframe: seen through what hides them)
            const Geometry::Frustum& frustum = camera.frustum();
            Render::UniformBufferTransform utransform;
            for (const Weak<Renderable>& weak_renderable : collection.m_renderables)
            {
                auto renderable = weak_renderable.lock();
                const bool culled = renderable && renderable->can_draw() && renderable->support_culling();
                if (culled && Geometry::Intersection::check(frustum, renderable->bounding_box()) != Geometry::Intersection::OUTSIDE)
                {
                    const Geometry::OBoundingBox& obb = renderable->bounding_box();
                    if (occlusion.hidden(obb.to_aabb(), false))
                    {
                        utransform.m_model = Square::translate(Constants::identity<Mat4>(), obb.get_position());
                        utransform.m_model *= Mat4(obb.get_rotation_matrix());
                        utransform.m_model = Square::scale(utransform.m_model, obb.get_extension() * 1.001f);
                        utransform.m_inv_model = Square::inverse(utransform.m_model);
                        render().update_steam_CB(m_cb_transform.get(), (const unsigned char*)&utransform, sizeof(utransform));
                        for (auto& pass : *wireframe)
                        {
                            pass.bind(render(), inputs, m_debug_effect->parameters());
                            m_mesh_box->draw(render());
                            pass.unbind();
                        }
                    }
                }
            }
        }
        draw_occlusion_image(occlusion, camera);
    }

    void DrawerPassDebug::draw_occlusion_image(const SoftwareOcclusion& occlusion, const Camera& camera)
    {
        const IVec2 size = occlusion.size();
        const std::vector<float>& depth = occlusion.depth();
        const bool drawable = occlusion.active() && m_shader_texture_2D && m_shader_texture_2D->base_shader() && m_mesh_quad
                           && size.x > 0 && size.y > 0 && depth.size() == size_t(size.x) * size_t(size.y);
        if (drawable)
        {
            //the depth as colors: nothing dark blue, an occluder grey, lighter the nearer (log of
            //the distance, 1 to 1000 m)
            m_occlusion_pixels.resize(depth.size() * 4);
            for (size_t i = 0; i != depth.size(); ++i)
            {
                unsigned char* pixel = &m_occlusion_pixels[i * 4];
                if (depth[i] > 0.0f)
                {
                    const float distance = 1.0f / depth[i];
                    const float t = std::clamp(std::log10(std::max(distance, 1.0f)) / 3.0f, 0.0f, 1.0f);
                    const unsigned char grey = (unsigned char)(255.0f * (1.0f - t * 0.85f));
                    pixel[0] = grey; pixel[1] = grey; pixel[2] = grey;
                }
                else
                {
                    pixel[0] = 10; pixel[1] = 20; pixel[2] = 60;
                }
                pixel[3] = 255;
            }
            const TextureRawDataInformation data
            {
                TF_RGBA8, (unsigned int)size.x, (unsigned int)size.y, m_occlusion_pixels.data(), TT_RGBA, TTF_UNSIGNED_BYTE, false
            };
            //its texture (again at a new size), its pixels
            if (!m_occlusion_texture || m_occlusion_size != size)
            {
                if (m_occlusion_texture)
                {
                    render().delete_texture(m_occlusion_texture);
                }
                m_occlusion_texture = render().create_texture(data, { TMIN_NEAREST, TMAG_NEAREST, TEDGE_CLAMP, TEDGE_CLAMP, TEDGE_CLAMP });
                m_occlusion_size = size;
            }
            else
            {
                render().update_texture(m_occlusion_texture, data);
            }
            //in the bottom left corner, a third of the width of the screen (the rows of the
            //viewport go up on GL, down on the others)
            const Vec4& viewport = camera.viewport().viewport();
            const float width = viewport.z / 3.0f;
            const float height = width * float(size.y) / float(size.x);
            const RenderDriver driver = render().get_render_driver();
            const bool rows_up = driver == DR_OPENGL || driver == DR_OPENGL_ES;
            const float bottom = rows_up ? viewport.y + 16.0f : viewport.y + viewport.w - height - 16.0f;
            render().set_viewport_state({ Vec4(viewport.x + 16.0f, bottom, width, height) });
            render().set_depth_buffer_state({ DM_DISABLE });
            render().set_blend_state({});
            render().set_cullface_state({ CF_BACK });
            Resource::Shader* shader = m_shader_texture_2D.get();
            shader->bind();
            if (auto uniform_texture = shader->uniform("g_texture")) uniform_texture->set(m_occlusion_texture);
            //(not used: bound, a texture not bound is an error on some drivers)
            if (auto uniform_minmax = shader->uniform("g_minmax")) uniform_minmax->set(m_occlusion_texture);
            if (auto uniform_rect = shader->uniform("rect")) uniform_rect->set(Vec4(-1.0f, -1.0f, 2.0f, 2.0f));
            if (auto uniform_params = shader->uniform("params")) uniform_params->set(Vec4(0.0f, 1.0f, 1.0f, 0.0f));
            m_mesh_quad->draw(render());
            shader->unbind();
            render().set_viewport_state({ viewport });
            render().set_depth_buffer_state({ DM_ENABLE_AND_WRITE });
        }
    }
}
}
