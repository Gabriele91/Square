#include "Square/Core/Context.h"
#include "Square/Driver/Render.h"
#include "Square/Driver/RenderInspector.h"
#include "Square/Render/Material.h"
#include "Square/Render/Effect.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Viewport.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/Transform.h"
#include "Square/Render/ShadowBuffer.h"
#include "Square/Render/DrawerPassDebug.h"
#include "Square/Render/Mesh.h"
#include "Square/Resource/Effect.h"
#include "Square/Resource/Shader.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Math/Transformation.h"
#include "Square/Math/Tangent.h"
#include <algorithm>

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

    static Shared<Render::Mesh> create_cube(Square::Context& context, const float size = 1.0f)
    {
        // Output
        Shared<Render::Mesh> mesh = MakeShared<Render::Mesh>(context);
        // Geometry
        Render::Mesh::Vertex3DList vertexs
        {
            { Vec3(-size, size, -size) }, // +Y (top face)
            { Vec3(size, size, -size) },
            { Vec3(size, size,  size) },
            { Vec3(-size, size,  size) },

            { Vec3(-size, -size,  size) }, // -Y (bottom face)
            { Vec3(size, -size,  size) },
            { Vec3(size, -size, -size) },
            { Vec3(-size, -size, -size) },

            { Vec3(size,  size,  size) }, // +X (right face)
            { Vec3(size,  size, -size) },
            { Vec3(size, -size, -size) },
            { Vec3(size, -size,  size) },

            { Vec3(-size,  size, -size) }, // -X (left face)
            { Vec3(-size,  size,  size) },
            { Vec3(-size, -size,  size) },
            { Vec3(-size, -size, -size) },

            { Vec3(-size,  size, size) }, // +Z (front face)
            { Vec3(size,  size, size) },
            { Vec3(size, -size, size) },
            { Vec3(-size, -size, size) },

            { Vec3(size,  size, -size) }, // -Z (back face)
            { Vec3(-size,  size, -size) },
            { Vec3(-size, -size, -size) },
            { Vec3(size, -size, -size) }
        };
        Render::Mesh::IndexList indexes
        {
            0, 2, 1,
            0, 3, 2,

            4, 6, 5,
            4, 7, 6,

            8, 10, 9,
            8, 11, 10,

            12, 14, 13,
            12, 15, 14,

            16, 18, 17,
            16, 19, 18,

            20, 22, 21,
            20, 23, 22
        };
        // Submesh
        Render::SubMesh sub_mesh{ Render::DrawType::DRAW_TRIANGLES, uint32(indexes.size()) };
        //Build mesh
        if (mesh->build(vertexs, indexes, { sub_mesh }, false))
            return mesh;
        return nullptr;
    }

    //screen quad in NDC, wound clockwise (engine front-face convention)
    static Shared<Render::Mesh> create_screen_quad(Square::Context& context)
    {
        Render::Mesh::Vertex3DList vertexs
        {
            { Vec3(-1.0f, -1.0f, 0.0f) },
            { Vec3( 1.0f, -1.0f, 0.0f) },
            { Vec3( 1.0f,  1.0f, 0.0f) },
            { Vec3(-1.0f,  1.0f, 0.0f) },
        };
        Render::Mesh::IndexList indexes{ 0, 2, 1, 0, 3, 2 };
        auto mesh = MakeShared<Render::Mesh>(context);
        if (mesh->build(vertexs, indexes))
            return mesh;
        return nullptr;
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
            m_mesh_box->draw(render());
            pass.unbind();
        }
    }

    DrawerPassDebug::DrawerPassDebug(Square::Context& context, unsigned char flags)
    : DrawerPass(context.allocator(), RPT_DEBUG)
    , m_context(context)
    , m_flags(flags)
    {
		m_debug_effect = context.resource<Resource::Effect>("Debug");
        m_cb_camera    = Render::stream_constant_buffer<Render::UniformBufferCamera>(&render());
		m_cb_transform = Render::stream_constant_buffer<Render::UniformBufferTransform>(&render());
        m_mesh_box     = create_cube(context);
        if (!m_mesh_box)
        {
            context.logger()->warning("Unable to build debug mesh");
        }
        //texture panel
        m_shader_texture_2D    = context.resource<Resource::Shader>("DebugTexture2D");
        m_shader_texture_cube  = context.resource<Resource::Shader>("DebugTextureCube");
        m_shader_texture_array = context.resource<Resource::Shader>("DebugTextureArray");
        m_mesh_quad            = create_screen_quad(context);
        if (!m_mesh_quad)
        {
            context.logger()->warning("Unable to build debug quad mesh");
        }
    }
    //context
    Square::Context& DrawerPassDebug::context(){ return m_context; }
    const Square::Context& DrawerPassDebug::context() const { return m_context; }
    //render
    Render::Context& DrawerPassDebug::render(){ return *context().render(); }
    const Render::Context& DrawerPassDebug::render() const { return *context().render(); }
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
            for (auto weak_light : queues[RQ_SPOT_LIGHT])
                if (auto light = weak_light->lock< Render::Light >())
                {
                    Render::UniformSpotShadowLight spot_light_info;
                    light->set(&spot_light_info, false);
                    draw_fustrum(drawer,
                        camera,
                        spot_light_info.m_view,
                        spot_light_info.m_projection,
                        debug_colors(DB_BLUE),
                        false);
                }
        }
        if (bool(m_flags & DebugFlags::DF_DRAW_POINT_LIGHT))
        {
            for (auto weak_light : queues[RQ_POINT_LIGHT])
                if (auto light = weak_light->lock< Render::Light >())
                {
                    Render::UniformPointShadowLight point_light_info;
                    light->set(&point_light_info, false);
                    const uint8_t cude_size = 6;
                    for (int i = 0; i < cude_size; ++i)
                    {
                        draw_fustrum(drawer,
                            camera,
                            point_light_info.m_view[i],
                            point_light_info.m_projection,
                            debug_colors(DebugColor(i)),
                            false);
                    }
                }
        }
        if (bool(m_flags & DebugFlags::DF_DRAW_DIRECTIONAL_LIGHT))
        {
            for (auto weak_light : queues[RQ_DIRECTION_LIGHT])
                if (auto light = weak_light->lock< Render::Light >())
                {
                    Render::UniformDirectionShadowLight direction_light_info;
                    light->set(&direction_light_info, &camera, false);
                    for (int i = 0; i < DIRECTION_SHADOW_CSM_NUMBER_OF_FACES; ++i)
                    {
                        draw_fustrum(drawer,
                            camera,
                            direction_light_info.m_view[i],
                            direction_light_info.m_projection[i],
                            debug_colors(DebugColor(i)),
                            false);
                    }
                }
        }
        // Texture panel (images/TBO/RBO)
        if (bool(m_flags & (DebugFlags::DF_DRAW_IMAGES | DebugFlags::DF_DRAW_TBO | DebugFlags::DF_DRAW_RBO)))
        {
            draw_texture_panel(camera);
        }
    }

    void DrawerPassDebug::draw_texture_panel(const Camera& camera)
    {
#if !defined(TEXTURE_INTROSPECTION)
        //no driver registry to read: the panel cannot work
        static bool s_warned = false;
        if (!s_warned)
        {
            context().logger()->warning("DrawerPassDebug: the texture panel requires TEXTURE_INTROSPECTION (Square/Config.h)");
            s_warned = true;
        }
#else
        if (!m_mesh_quad) return;
        if (!render().inspector()) return;
        //viewport in pixels
        const Vec4& viewport = camera.viewport().viewport();
        const float view_height = viewport.w;
        if (viewport.z <= 0.0f || view_height <= 0.0f) return;
        //what the driver has alive, classified by category
        struct Entry
        {
            Render::Texture*           m_texture;
            const Render::TextureInfo* m_info;
            bool                       m_is_target; //attached to a render target
        };
        std::vector<Entry> entries;
        auto is_attached = [&](const Render::Texture* texture) -> bool
        {
            for (const auto& target_it : render().inspector()->targets())
            for (const auto& field : target_it.second.m_fields)
            {
                if (field.m_texture == texture) return true;
            }
            return false;
        };
        for (const auto& texture_it : render().inspector()->textures())
        {
            const TextureInfo& info = texture_it.second;
            const bool special  = info.is_depth() || info.m_shape != TS_TEXTURE_2D;
            const bool attached = is_attached(texture_it.first);
            //TBO = depth/cube/array (shadow maps), RBO = color attachments, images = the rest
            const unsigned char category = special  ? DF_DRAW_TBO
                                         : attached ? DF_DRAW_RBO
                                         : DF_DRAW_IMAGES;
            if (!(m_flags & category)) continue;
            entries.push_back({ texture_it.first, &info, attached });
        }
        if (entries.empty()) return;
        //panel layout in pixels
        const float panel_width = std::min(viewport.z * 0.25f, 256.0f);
        const float padding     = 4.0f;
        const float thumb_width = panel_width - padding * 2.0f;
        //thumbnail height: cube faces / array layers are drawn side by side
        auto thumb_height = [&](const Entry& entry) -> float
        {
            const float aspect  = entry.m_info->m_width ? float(entry.m_info->m_height) / float(entry.m_info->m_width) : 1.0f;
            const int   columns = entry.m_info->m_shape == TS_TEXTURE_2D ? 1 : std::max(1, entry.m_info->m_layers);
            return std::max(thumb_width * aspect / float(columns), 8.0f);
        };
        //content height, to clamp the scroll
        float content_height = padding;
        for (const auto& entry : entries)
        {
            content_height += thumb_height(entry) + padding;
        }
        m_panel_scroll = std::max(0.0f, std::min(m_panel_scroll, std::max(0.0f, content_height - view_height)));
        //the viewport is the panel box: everything outside is clipped by the hardware
        const float panel_x = (m_panel_side == PANEL_RIGHT) ? viewport.x + viewport.z - panel_width : viewport.x;
        render().set_viewport_state({ Vec4(panel_x, viewport.y, panel_width, view_height) });
        render().set_depth_buffer_state({ DM_DISABLE });
        render().set_blend_state({});
        render().set_cullface_state({ CF_BACK });
        //on GL the render targets have row 0 at the bottom, images/DX/Metal at the top
        const RenderDriver driver = render().get_render_driver();
        const bool gl_backend = driver == DR_OPENGL || driver == DR_OPENGL_ES;
        //stack the thumbnails from the top, shifted by the scroll
        float cursor_top = view_height + m_panel_scroll - padding;
        for (const auto& entry : entries)
        {
            const float height = thumb_height(entry);
            const float bottom = cursor_top - height;
            const float top    = cursor_top;
            cursor_top = bottom - padding;
            //out of the panel?
            if (bottom > view_height || top < 0.0f) continue;
            //shader by shape
            Resource::Shader* shader = nullptr;
            switch (entry.m_info->m_shape)
            {
            case TS_TEXTURE_2D:    shader = m_shader_texture_2D.get();    break;
            case TS_TEXTURE_CUBE:  shader = m_shader_texture_cube.get();  break;
            case TS_TEXTURE_ARRAY: shader = m_shader_texture_array.get(); break;
            default: break;
            }
            if (!shader || !shader->base_shader()) continue;
            //thumbnail rect in panel NDC (x,y = bottom-left corner, z,w = size)
            const Vec4 rect
            (
                (padding / panel_width) * 2.0f - 1.0f,
                (bottom / view_height) * 2.0f - 1.0f,
                (thumb_width / panel_width) * 2.0f,
                (height / view_height) * 2.0f
            );
            const float flip = (gl_backend && entry.m_is_target) ? 0.0f : 1.0f;
            const Vec4 params
            (
                entry.m_info->is_depth() ? 1.0f : 0.0f,
                flip,
                float(entry.m_info->m_layers),
                0.0f
            );
            //draw
            shader->bind();
            if (auto uniform_texture = shader->uniform("g_texture")) uniform_texture->set(entry.m_texture);
            if (auto uniform_rect = shader->uniform("rect")) uniform_rect->set(rect);
            if (auto uniform_params = shader->uniform("params")) uniform_params->set(params);
            m_mesh_quad->draw(render());
            shader->unbind();
        }
        //restore state for whoever draws next
        render().set_viewport_state({ camera.viewport().viewport() });
        render().set_depth_buffer_state({ DM_ENABLE_AND_WRITE });
#endif
    }
}
}
