//
//  DrawerPassForward.cpp
//  Square
//
//  Created by Gabriele Di Bari on 25/07/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#include <algorithm>
#include <limits>
#include "Square/Core/Context.h"
#include "Square/System/RenderSystem.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Material.h"
#include "Square/Render/Effect.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Viewport.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Light.h"
#include "Square/Render/ShadowBuffer.h"
#include "Square/Render/DrawerPassShadow.h"
#include "Square/Resource/Effect.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Geometry/AABoundingBox.h"

namespace Square
{
namespace Render
{
	DrawerPassShadow::DrawerPassShadow(Square::Context& context)
    : DrawerPass(context.allocator(),RPT_SHADOW)
    , m_context(context)
    {
		m_cb_camera = Render::stream_constant_buffer<Render::UniformBufferCamera>(&render());
		m_cb_transform = Render::stream_constant_buffer<Render::UniformBufferTransform>(&render());

		m_cb_direction_light = Render::stream_constant_buffer<Render::UniformDirectionShadowLight>(&render());
		m_cb_point_light = Render::stream_constant_buffer<Render::UniformPointShadowLight>(&render());
		m_cb_spot_light = Render::stream_constant_buffer<Render::UniformSpotShadowLight>(&render());
	}
    //context
    Square::Context& DrawerPassShadow::context(){ return m_context; }
    const Square::Context& DrawerPassShadow::context() const { return m_context; }
    //render
    Render::Context& DrawerPassShadow::render(){ return *System::get<RenderSystem>(context())->render(); }
    const Render::Context& DrawerPassShadow::render() const { return *System::get<RenderSystem>(context())->render(); }
    namespace AuxCascades
    {
        //a box out of the square of a cascade (-1, 1), its corners on its plane
        bool outside(const Vec2& min_xy, const Vec2& max_xy)
        {
            const bool on_the_left  = max_xy.x < -1.0f;
            const bool on_the_right = min_xy.x > 1.0f;
            const bool below        = max_xy.y < -1.0f;
            const bool above        = min_xy.y > 1.0f;
            return on_the_left || on_the_right || below || above;
        }

        //the cascades a box is in (bit i: cascade i), on the plane of the light
        uint32 mask(const Geometry::AABoundingBox& box, const UniformDirectionShadowLight& shadow)
        {
            const int cascades = std::clamp(shadow.m_options.y, 1, int(DIRECTION_SHADOW_CSM_NUMBER_OF_FACES));
            const auto corners = box.get_bounding_box();
            uint32 mask = 0;
            for (int i = 0; i != cascades; ++i)
            {
                const Mat4 view_projection = Mat4(shadow.m_projection[i]) * Mat4(shadow.m_view[i]);
                Vec2 min_xy(std::numeric_limits<float>::max());
                Vec2 max_xy(std::numeric_limits<float>::lowest());
                for (const auto& corner : corners)
                {
                    const Vec4 clip = view_projection * Vec4(corner, 1.0f);
                    min_xy = glm::min(min_xy, Vec2(clip));
                    max_xy = glm::max(max_xy, Vec2(clip));
                }
                if (!outside(min_xy, max_xy))
                {
                    mask |= MULTI_PASS_LAYER_BIT(i);
                }
            }
            return mask;
        }
    }

    namespace AuxShadowCache
    {
        //the draws of the pass a cache stays without its light drawn
        static constexpr size_t s_forget_after = 600;

        //the bytes of a uniform of a shadow (the key of a cache: its place, its views)
        template < typename T >
        std::vector<unsigned char> key(const T& uniform)
        {
            const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&uniform);
            return std::vector<unsigned char>(bytes, bytes + sizeof(T));
        }

        //bytes into a hash (FNV-1a)
        void hash(uint64& value, const void* data, size_t size)
        {
            const unsigned char* bytes = static_cast<const unsigned char*>(data);
            for (size_t i = 0; i != size; ++i)
            {
                value ^= uint64(bytes[i]);
                value *= 1099511628211ull;
            }
        }

        //the buffer of a cache as the shadow map of its light: its size, its type, its layers
        bool same(const ShadowBuffer& cache, const ShadowBuffer& shadow)
        {
            const bool built  = cache.texture() != nullptr;
            const bool size   = cache.size() == shadow.size();
            const bool type   = cache.type() == shadow.type();
            const bool layers = cache.layers() == shadow.layers();
            return built && size && type && layers;
        }

        //the buffer of a cache made as the shadow map of its light, if it is not (true: made)
        bool fit(std::unique_ptr<ShadowBuffer>& cache, const ShadowBuffer& shadow, Square::Context& context)
        {
            bool made = false;
            if (!cache || !same(*cache, shadow))
            {
                cache = std::make_unique<ShadowBuffer>(context);
                cache->build(shadow.size(), shadow.type(), shadow.layers());
                made = true;
            }
            return made;
        }
    }

    void DrawerPassShadow::static_cache(bool enable)
    {
        m_static_cache = enable;
    }

    bool DrawerPassShadow::static_cache() const
    {
        return m_static_cache;
    }

    //draw
    void DrawerPassShadow::draw
    (
       Drawer&           drawer
     , int               num_of_pass
     , const Vec4&       clear_color
     , const Vec4&       ambient_light
	 , const Camera&     camera
	 , const Light&      light
     , const Collection& collection
     , const PoolQueues& queues
    )
    {
		++m_draws;
		//names
		static const std::string techniques_table[]
		{
			"None",
			"SpotShadow",
			"PointShadow",
			"DirectionShadow"
		};
		//parameters
		EffectPassInputs inputs
		{
			//render
			  m_cb_camera.get()
			, m_cb_transform.get()
			//light
			, ambient_light
			, nullptr
			, nullptr
			, nullptr
			, nullptr
			, m_cb_direction_light.get()
			, m_cb_point_light.get()
			, m_cb_spot_light.get()
		};
		const auto& technique_name = techniques_table[(size_t)light.type()];
		//the shadow-camera buffer and the viewport of the light; a light that does not move: its
		//cache, else all of its casters
		switch (light.type())
		{
		case LightType::SPOT:
		{
			Render::UniformSpotShadowLight uspotshadow;
			light.set(&uspotshadow);
			render().update_steam_CB(m_cb_spot_light.get(), (const unsigned char*)&uspotshadow, sizeof(uspotshadow));
			render().set_viewport_state(light.shadow_viewport());
			draw_cached(light, AuxShadowCache::key(uspotshadow), queues, technique_name, inputs);
		}
		break;
		case LightType::POINT:
		{
			Render::UniformPointShadowLight upointshadow;
			light.set(&upointshadow);
			render().update_steam_CB(m_cb_point_light.get(), (const unsigned char*)&upointshadow, sizeof(upointshadow));
			render().set_viewport_state(light.shadow_viewport());
			draw_cached(light, AuxShadowCache::key(upointshadow), queues, technique_name, inputs);
		}
		break;
		case LightType::DIRECTION:
		{
			//(its cascades follow the camera: all of its casters, every frame)
			Render::UniformDirectionShadowLight udirectionshadow;
			light.set(&udirectionshadow, &camera);
			render().update_steam_CB(m_cb_direction_light.get(), (const unsigned char*)&udirectionshadow, sizeof(udirectionshadow));
			render().set_viewport_state(light.shadow_viewport());
			render().enable_render_target(light.shadow_buffer().target());
			render().clear(Render::CLEAR_DEPTH);
			draw_casters(queues, technique_name, inputs, &udirectionshadow, Casters::ALL);
			render().disable_render_target(light.shadow_buffer().target());
		}
		break;
		default:
			//not implemented
		break;
		}
		forget_old_caches();
    }

    void DrawerPassShadow::draw_cached
    (
      const Light& light
    , const std::vector<unsigned char>& key
    , const PoolQueues& queues
    , const std::string& technique_name
    , EffectPassInputs& inputs
    )
    {
		const ShadowBuffer& shadow = light.shadow_buffer();
		if (m_static_cache)
		{
			StaticCache& cache = m_caches[&light];
			cache.m_used = m_draws;
			//out of date (new, the light moved, its static casters changed): drawn again
			const uint64 casters = static_casters(queues);
			const bool made = AuxShadowCache::fit(cache.m_buffer, shadow, m_context);
			const bool moved = cache.m_key != key;
			const bool changed = cache.m_casters != casters;
			if (made || moved || changed)
			{
				render().enable_render_target(cache.m_buffer->target());
				render().clear(Render::CLEAR_DEPTH);
				draw_casters(queues, technique_name, inputs, nullptr, Casters::STATIC);
				render().disable_render_target(cache.m_buffer->target());
				cache.m_key = key;
				cache.m_casters = casters;
			}
			//the static ones as they were, the dynamic ones over them
			render().copy_texture(cache.m_buffer->texture(), shadow.texture());
			render().enable_render_target(shadow.target());
			draw_casters(queues, technique_name, inputs, nullptr, Casters::DYNAMIC);
			render().disable_render_target(shadow.target());
		}
		else
		{
			//no cache: all of them, every frame
			render().enable_render_target(shadow.target());
			render().clear(Render::CLEAR_DEPTH);
			draw_casters(queues, technique_name, inputs, nullptr, Casters::ALL);
			render().disable_render_target(shadow.target());
		}
    }

    bool DrawerPassShadow::takes(const Renderable& renderable, Casters casters) const
    {
		switch (casters)
		{
		case Casters::STATIC:  return renderable.is_static();
		case Casters::DYNAMIC: return !renderable.is_static();
		case Casters::ALL:
		default:               return true;
		}
    }

    uint64 DrawerPassShadow::static_casters(const PoolQueues& queues) const
    {
		//(FNV-1a: its offset basis)
		uint64 value = 14695981039346656037ull;
		for (auto randerable : RenderableQuery(queues, { RQ_OPAQUE, RQ_TRANSLUCENT }))
		{
			const bool caster = randerable && randerable->is_static() && randerable->can_draw();
			if (caster)
			{
				//which one, where, at which level of detail
				const Renderable* identity = randerable.get();
				const float fade = randerable->lod_fade();
				AuxShadowCache::hash(value, &identity, sizeof(identity));
				AuxShadowCache::hash(value, &fade, sizeof(fade));
				if (auto transform = randerable->transform().lock())
				{
					const Mat4& model = transform->global_model_matrix();
					AuxShadowCache::hash(value, &model, sizeof(model));
				}
			}
		}
		return value;
    }

    void DrawerPassShadow::forget_old_caches()
    {
		auto it = m_caches.begin();
		while (it != m_caches.end())
		{
			const bool old = m_draws - it->second.m_used > AuxShadowCache::s_forget_after;
			if (old)
			{
				it = m_caches.erase(it);
			}
			else
			{
				++it;
			}
		}
    }

    void DrawerPassShadow::draw_casters
    (
      const PoolQueues& queues
    , const std::string& technique_name
    , EffectPassInputs& inputs
    , const UniformDirectionShadowLight* cascades
    , Casters casters
    )
    {
		Render::UniformBufferTransform utransform;
		//for each elements of opaque and translucent queues
		for (auto randerable : RenderableQuery(queues, { RQ_OPAQUE, RQ_TRANSLUCENT }))
		{
			const bool drawn = randerable && randerable->can_draw() && takes(*randerable, casters);
			if (drawn)
			{
				//the cascades of the caster (none: not drawn)
				inputs.m_layer_mask = MULTI_PASS_ALL_LAYERS;
				if (cascades)
				{
					inputs.m_layer_mask = AuxCascades::mask(randerable->bounding_box().to_aabb(), *cascades);
				}
				if (inputs.m_layer_mask != 0)
				{
					draw_caster(*randerable, technique_name, inputs, utransform);
				}
			}
		}
    }

    void DrawerPassShadow::draw_caster
    (
      Renderable& randerable
    , const std::string& technique_name
    , EffectPassInputs& inputs
    , UniformBufferTransform& utransform
    )
    {
		//its transform
		if (auto transform = randerable.transform().lock())
		{
			transform->set(&utransform);
			utransform.m_lod_fade = randerable.lod_fade();
			render().update_steam_CB(m_cb_transform.get(), (const unsigned char*)&utransform, sizeof(utransform));
		}
		//each of its materials: the technique of its effect (the clip variant: by its material, or
		//its level of detail fading)
		const bool fading = randerable.lod_fade() < 1.0f;
		for (size_t material_id = 0; material_id != randerable.materials_count(); ++material_id)
		{
			auto material = randerable.material(material_id).lock();
			EffectTechnique* technique = nullptr;
			if (material)
			{
				technique = material->effect()->technique(technique_name, randerable.instanced(), *material->parameters(), fading);
			}
			if (technique)
			{
				//each pass; a multi-pass one drawn draw_count times, one per cube face / cascade
				//of the caster (the shader routes via the pass index)
				for (auto[pass, draw_id] : technique->iterate_draws())
				{
					const bool multi_pass = pass.m_draw_count > 1;
					const bool in_layer = MULTI_PASS_HAS_LAYER(inputs.m_layer_mask, draw_id);
					if (!multi_pass || in_layer)
					{
						randerable.draw(render(), material_id, inputs, pass, draw_id);
					}
				}
			}
		}
    }
}
}
