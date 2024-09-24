//
//  Drawer.cpp
//  Square
//
//  Created by Gabriele Di Bari on 24/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include <set>
#include "Square/Core/Context.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Drawer.h"
#include "Square/Render/Viewport.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Light.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/ShadowBuffer.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Geometry/AABoundingBox.h"

namespace Square
{
namespace Render
{
#pragma region RenderableIterator
    RenderableIterator::RenderableIterator(const PoolQueues& queues, const std::array<bool, RQ_MAX>& queues_types, QueueType queue_current)
    : m_queues(queues)
    , m_queues_types(queues_types)
    , m_queue_current(queue_current)
    {
        if (m_queue_current != RQ_MAX)
        {
            m_queue_it = m_queues[m_queue_current].begin();
            test_it_or_queue_next();
        }
    }

    RenderableIterator& RenderableIterator::operator++()
    {
        // Next element
        ++m_queue_it;
        // Test
        test_it_or_queue_next();
        // return this as copy
        return *this;
    }

    bool RenderableIterator::operator == (const RenderableIterator& other) const
    {
        if (&m_queues != &other.m_queues) return false;
        if (&m_queues_types != &other.m_queues_types) return false;
        if (m_queue_current != other.m_queue_current) return false;
        if (m_queue_current != RQ_MAX && m_queue_it != other.m_queue_it) return false;
        return true;
    }

    bool RenderableIterator::operator != (const RenderableIterator& other) const
    {
        return !(*this == other);
    }

    Shared<Render::Renderable> RenderableIterator::operator*()
    {
        if (auto randerable = (*m_queue_it)->lock< Render::Renderable >())
        {
            return randerable;
        }
        return nullptr;
    }

    Shared<Render::Renderable> RenderableIterator::operator* () const
    {
        if (auto randerable = (*m_queue_it)->lock< Render::Renderable >())
        {
            return randerable;
        }
        return nullptr;
    }

    void RenderableIterator::test_it_or_queue_next()
    {
        if (m_queue_it == m_queues[m_queue_current].end())
        {
            for (unsigned char next = m_queue_current + 1; next < RQ_MAX; ++next)
            {
                if (m_queues_types[next])
                {
                    // Next queue
                    m_queue_current = QueueType(next);
                    m_queue_it = m_queues[m_queue_current].begin();
                    // Test if it is void
                    if (m_queue_it != m_queues[m_queue_current].end())
                    {
                        return;
                    }
                }
            }
            m_queue_current = RQ_MAX;
        }
    }
#pragma endregion

#pragma region RenderableQuery
    static std::array<bool, RQ_MAX> query_from_select_queues(const std::vector<QueueType>& select_queues)
    {
        square_assert(select_queues.size());
        static std::array<bool, RQ_MAX> queues_types{ false };
        for (QueueType qtype : select_queues)
        {
            queues_types[qtype] = true;
        }
        return queues_types;
    }

    RenderableQuery::RenderableQuery(const PoolQueues& queues, const std::vector<QueueType>& select_queues)
    : m_queues_types(query_from_select_queues(select_queues))
    , m_begin(queues, m_queues_types, select_queues.size() ? select_queues[0] : RQ_MAX)
    , m_end(queues, m_queues_types, RQ_MAX)
    {
    }

    const RenderableIterator& RenderableQuery::begin() const { return m_begin; }
    const RenderableIterator& RenderableQuery::end() const { return m_end; }
#pragma endregion

    // Convert flag to int
    static inline size_t draw_pass_type_to_index(unsigned char draw_pass_type)
    {
#ifdef _MSC_VER
        return draw_pass_type ? _tzcnt_u32(draw_pass_type) : 0;
#else
        return draw_pass_type ? __builtin_ctz(draw_pass_type) : 0;
#endif
    }

    //Init
    Drawer::Drawer(Square::Context& context):m_context(context){}
    
    //context
    Square::Context& Drawer::context(){ return m_context; }
    const Square::Context& Drawer::context() const { return m_context; }
    
    Render::Context& Drawer::render(){ return *context().render(); }
    const Render::Context& Drawer::render() const { return *context().render(); }
    
    //draw
    void Drawer::draw(const Collection& collection, unsigned char draw_types)
    {
        draw(Constants::identity<Vec4>(), Constants::identity<Vec4>(), collection, draw_types);
    }
    void Drawer::draw(const Vec4& clear_color,
                      const Vec4& ambient_color, 
                      const Collection& collection, 
                      unsigned char draw_types)
    {
        //objects queues
        std::vector<PoolQueues> queues;
        init_vector(queues, collection.m_cameras.size(), context().allocator());
        //shadow rendered
        std::set<Light*> m_shadows_draw;
        //filter
        auto can_draw_shadow = [&](Light* light) -> bool
        {  
            return light->shadow() && m_shadows_draw.find(light) == m_shadows_draw.end(); 
        };
        //build queues
        for(auto [camera_index, weak_camera] : enumerate(collection.m_cameras))
        if (auto camera = weak_camera.lock())
        {
            CollectionQuery::lights(collection, queues[camera_index], *camera);
            CollectionQuery::renderables(collection, queues[camera_index], *camera);
        }
        //compute scene size
        auto scene_size = [](PoolQueues& queues) -> Geometry::AABoundingBox
        {
            //compute scene size
            Geometry::AABoundingBox scene_aabb
            (
                Vec3{ std::numeric_limits<float>::max() },
                Vec3{ std::numeric_limits<float>::lowest() }
            );
            //for each elements of opaque and translucent queues
            for (auto randerable : RenderableQuery(queues, { RQ_OPAQUE, RQ_TRANSLUCENT }))
            {
                if (randerable)
                {
                    //jump?
                    if (!randerable->can_draw()) continue;
                    //get box
                    scene_aabb = scene_aabb.merge(randerable->bounding_box().to_aabb());
                }
            }
            return scene_aabb;
        };
        //shadow caster
        if(bool(draw_types & DrawerPassType::RPT_SHADOW))
        {
            for(auto [camera_index, weak_camera] : enumerate(collection.m_cameras))
            if (auto camera = weak_camera.lock())
            {
                //pass counter
                int pass_shadow_count { 0 };
                //for each pass
                for(auto& pass : m_rendering_pass[draw_pass_type_to_index(RPT_SHADOW)])
                {
                    //draw a shadow map
                    auto draw_shadow = [&](const Shared<Light>& light, QueueType queue_type)
                    {
                        if (light)
                        if (can_draw_shadow(light.get()))
                        {
						    //opaque queue
						    PoolQueues queues(context().allocator());
						    //compute queue
						    CollectionQuery::renderables(collection, queues, *light);
                            //set scene size
                            if(queue_type == RQ_DIRECTION_LIGHT)
                                light->set_scene_size(scene_size(queues));
                            //draw
                            pass->draw(  *this
                                        , pass_shadow_count++
                                        , clear_color
                                        , ambient_color
                                        , *camera
                                        , *light
                                        , collection
                                        , queues
                                        );
						    //drawed
                            m_shadows_draw.insert(light.get());
                        }
                    };
                    //for spot light objects
                    for(auto e_light : queues[camera_index][RQ_SPOT_LIGHT]) draw_shadow(e_light->lock<Light>(), RQ_SPOT_LIGHT);
                    for(auto e_light : queues[camera_index][RQ_POINT_LIGHT]) draw_shadow(e_light->lock<Light>(), RQ_POINT_LIGHT);
                    for(auto e_light : queues[camera_index][RQ_DIRECTION_LIGHT]) draw_shadow(e_light->lock<Light>(), RQ_DIRECTION_LIGHT);
                }
            }
        }
        //for each cameras
        for(auto [camera_index, weak_camera] : enumerate(collection.m_cameras))
        if (auto camera = weak_camera.lock())
        {
            // RENDER Scene
            if (bool(draw_types & DrawerPassType::RPT_RENDER))
            {  
                //pass counter
                int pass_render_count { 0 };
                //draw all
                for(auto& pass : m_rendering_pass[draw_pass_type_to_index(RPT_RENDER)])
                {
                    pass->draw(  *this
                               , pass_render_count++
                               , clear_color
                               , ambient_color
                               , *camera
                               , collection
                               , queues[camera_index]
                               );
                }
            }
            // UI
            if (bool(draw_types & DrawerPassType::RPT_UI))
            {
                //pass counter
                int pass_ui_count { 0 };
                //draw all
                for(auto& pass : m_rendering_pass[draw_pass_type_to_index(RPT_UI)])
                {
                    pass->draw(  *this
                               , pass_ui_count++
                               , clear_color
                               , ambient_color
                               , *camera
                               , collection
                               , queues[camera_index]
                               );
                }
            }
#if _DEBUG
            // DEBUG
            if (bool(draw_types & DrawerPassType::RPT_DEBUG))
            {
                //pass counter
                int pass_debug_count{ 0 };
                //draw all
                for (auto& pass : m_rendering_pass[draw_pass_type_to_index(RPT_DEBUG)])
                {
                    pass->draw(*this
                            , pass_debug_count++
                            , clear_color
                            , ambient_color
                            , *camera
                            , collection
                            , queues[camera_index]
                        );
                }
            }
#endif
        }
    }
    
    //add a pass
    void Drawer::add(Shared<Square::Render::DrawerPass> pass)
    {
        m_rendering_pass[draw_pass_type_to_index(pass->type())].push_back(pass);
    }
}
}
