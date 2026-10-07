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
                //its square: -1, 1
                const bool on_the_left  = max_xy.x < -1.0f;
                const bool on_the_right = min_xy.x > 1.0f;
                const bool below        = max_xy.y < -1.0f;
                const bool above        = min_xy.y > 1.0f;
                const bool outside      = on_the_left || on_the_right || below || above;
                if (!outside) mask |= MULTI_PASS_LAYER_BIT(i);
            }
            return mask;
        }
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
        //transfor
		Render::UniformBufferTransform utransform;
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
		//the shadow of a directional light: its cascades (the casters are in some of them)
		Render::UniformDirectionShadowLight udirectionshadow;
		const bool cascades = light.type() == LightType::DIRECTION;
		//Update the shadow-camera buffer + viewport for this light.
		switch (light.type())
		{
		case LightType::SPOT:
		{
			Render::UniformSpotShadowLight uspotshadow;
			light.set(&uspotshadow);
			render().update_steam_CB(m_cb_spot_light.get(), (const unsigned char*)&uspotshadow, sizeof(uspotshadow));
			render().set_viewport_state(light.shadow_viewport());
		}
		break;
		case LightType::POINT:
		{
			Render::UniformPointShadowLight upointshadow;
			light.set(&upointshadow);
			render().update_steam_CB(m_cb_point_light.get(), (const unsigned char*)&upointshadow, sizeof(upointshadow));
			render().set_viewport_state(light.shadow_viewport());
		}
		break;
		case LightType::DIRECTION:
		{
			light.set(&udirectionshadow, &camera);
			render().update_steam_CB(m_cb_direction_light.get(), (const unsigned char*)&udirectionshadow, sizeof(udirectionshadow));
			render().set_viewport_state(light.shadow_viewport());
		}
		break;
		default:
			//not implemented, return
			return;
		break;
		}
		//render target
		render().enable_render_target(light.shadow_buffer().target());
		//clear
		render().clear(Render::CLEAR_DEPTH);
		//get technique name
		const auto& technique_name = techniques_table[(size_t)light.type()];
		//for each elements of opaque and translucent queues
		for(auto randerable : RenderableQuery(queues, { RQ_OPAQUE, RQ_TRANSLUCENT }))
		if (randerable)
		{
			//jump?
			if(!randerable->can_draw()) continue;
			//the cascades of the caster (none: not drawn)
			inputs.m_layer_mask = MULTI_PASS_ALL_LAYERS;
			if (cascades) inputs.m_layer_mask = AuxCascades::mask(randerable->bounding_box().to_aabb(), udirectionshadow);
			if (inputs.m_layer_mask == 0) continue;
			//update transform
			if (auto transform = randerable->transform().lock())
			{
				transform->set(&utransform);
				render().update_steam_CB(m_cb_transform.get(), (const unsigned char*)&utransform, sizeof(utransform));
			}
			//for each materials
			for (size_t material_id = 0; material_id != randerable->materials_count(); ++material_id)
			{
				//material
				auto weak_material = randerable->material(material_id);
				auto material = weak_material.lock();
				if (!material) continue;
				//effect
				auto effect = material->effect();
				auto technique = effect->technique(technique_name, randerable->instanced());
				if (!technique) continue;
				//draw for each pass; multi-pass passes are drawn draw_count times,
				//one per cube face / cascade (the shader routes via the pass index).
				for (auto[pass,draw_id] : technique->iterate_draws())
				{
					//multi-pass: a draw for each cascade of the caster
					const bool multi_pass = pass.m_draw_count > 1;
					const bool in_layer   = MULTI_PASS_HAS_LAYER(inputs.m_layer_mask, draw_id);
					if (multi_pass && !in_layer) continue;
					randerable->draw(render(), material_id, inputs, pass, draw_id);
				}
			}
		}
		//disable render target
		render().disable_render_target(light.shadow_buffer().target());
    }
}
}
