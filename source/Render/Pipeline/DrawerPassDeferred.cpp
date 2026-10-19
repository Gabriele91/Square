//
//  DrawerPassDeferred.cpp
//  Square
//
//  See DrawerPassDeferred.h for the high level description.
//
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
#include "Square/Render/Pipeline/DrawerPassDeferred.h"
#include "Square/Render/BasicMesh.h"
#include "Square/Resource/Shader.h"
#include "Square/Render/Pipeline/LightVolume.h"
#include "Square/Render/Pipeline/ForwardShading.h"
#include "Square/Render/Profiler.h"
#include <cmath>
#include <algorithm>

namespace Square
{
namespace Render
{
	//light volume model matrix
	struct UniformLightVolume
	{
		Mat4 m_model;
	};

	//the velocity pass: the model of a renderable and the camera in the last frame
	struct UniformVelocity
	{
		Mat4 m_previous_model;
		Mat4 m_previous_view;
		Mat4 m_previous_projection;
	};

	//a camera that moves farther than this in a frame is a cut: no motion from the last frame
	static constexpr float s_velocity_camera_cut = 20.0f;

	//a camera as another one drawn at a share of its size (Drawer::resolution_scale): its viewport
	//scaled, the rest of it as the camera
	class ScaledCamera : public Camera
	{
	public:

		ScaledCamera(const Camera& camera, float scale)
		: m_camera(camera)
		, m_viewport(camera.viewport())
		{
			const IVec4& full = camera.viewport().viewport();
			m_viewport.viewport(IVec4
			(
				  int(std::round(float(full.x) * scale))
				, int(std::round(float(full.y) * scale))
				, std::max(int(std::round(float(full.z) * scale)), 1)
				, std::max(int(std::round(float(full.w) * scale)), 1)
			));
			time(camera.time());
			enable(camera.enable());
		}

		virtual const Mat4& model() const override { return m_camera.model(); }
		virtual const Mat4& view() const override { return m_camera.view(); }
		virtual const Mat4& projection() const override { return m_camera.projection(); }
		virtual const Render::Viewport& viewport() const override { return m_viewport; }
		virtual const Geometry::Frustum& frustum() const override { return m_camera.frustum(); }

		virtual void set(UniformBufferCamera* gpubuffer) const override
		{
			m_camera.set(gpubuffer);
			gpubuffer->m_viewport = Vec4(m_viewport.viewport());
		}

	private:

		const Camera&    m_camera;
		Render::Viewport m_viewport;
	};

	
	//////////////////////////////////////////////////////////////////////
	// Draw volume meshes 
	/////////////////////////////////////////////////////////////////////
	//draw the volume that bounds a light (sphere for point lights, cone for spot lights)
	static void draw_light_volume(Square::Render::Context& render,
								  const Shared<Square::Render::ConstBuffer>& cb_volume,
								  const Shared<Mesh>& volume,
								  const Mat4& model)
	{
		UniformLightVolume ulight_volume;
		ulight_volume.m_model = model;
		Render::update_constant_buffer(&render, cb_volume.get(), &ulight_volume);
		volume->draw(render);
	}

	//////////////////////////////////////////////////////////////////////
	// DrawerPassDeferred
	//////////////////////////////////////////////////////////////////////
	DrawerPassDeferred::DrawerPassDeferred(Square::Context& context)
	: DrawerPass(context.allocator(), RPT_RENDER)
	, m_context(context)
	, m_post_effects(context)
	{
		//constant buffers
		m_cb_camera          = Render::stream_constant_buffer<Render::UniformBufferCamera>(&render());
		m_cb_transform       = Render::stream_constant_buffer<Render::UniformBufferTransform>(&render());
		m_cb_direction_light = Render::stream_constant_buffer<Render::UniformDirectionLight>(&render());
		m_cb_point_light     = Render::stream_constant_buffer<Render::UniformPointLight>(&render());
		m_cb_spot_light      = Render::stream_constant_buffer<Render::UniformSpotLight>(&render());
		m_cb_light_volume    = Render::stream_constant_buffer<UniformLightVolume>(&render());
		m_cb_velocity        = Render::stream_constant_buffer<UniformVelocity>(&render());
		#if defined(SQUARE_DEBUG_TOOLS)
		m_gbuffer_draws.test("SQUARE_TEST_GBUFFER");
		#endif
		m_cb_direction_shadow_light = Render::stream_constant_buffer<Render::UniformDirectionShadowLight>(&render());
		m_cb_point_shadow_light     = Render::stream_constant_buffer<Render::UniformPointShadowLight>(&render());
		m_cb_spot_shadow_light      = Render::stream_constant_buffer<Render::UniformSpotShadowLight>(&render());
		//light shaders
		m_shader_ambient   = context.resource<Resource::Shader>("DeferredAmbientLight");
		m_shader_direction = context.resource<Resource::Shader>("DeferredDirectionLight");
		m_shader_point     = context.resource<Resource::Shader>("DeferredPointLight");
		m_shader_spot      = context.resource<Resource::Shader>("DeferredSpotLight");
		m_shader_direction_shadow = context.resource<Resource::Shader>("DeferredDirectionShadowLight");
		m_shader_point_shadow     = context.resource<Resource::Shader>("DeferredPointShadowLight");
		m_shader_spot_shadow      = context.resource<Resource::Shader>("DeferredSpotShadowLight");
		m_shader_present   = context.resource<Resource::Shader>("DeferredPresent");
		m_shader_velocity  = context.resource<Resource::Shader>("DeferredVelocity");
		//volume meshes
		m_quad   = BasicMesh::build_quad(context);
		m_sphere = LightVolume::build_sphere(context);
		m_cone   = LightVolume::build_cone(context);
	}

	DrawerPassDeferred::~DrawerPassDeferred()
	{
		if (auto render_driver = System::get<RenderSystem>(context())->render())
		{
			if (m_occlusion_target) render_driver->delete_render_target(m_occlusion_target);
			if (m_velocity_target)  render_driver->delete_render_target(m_velocity_target);
			if (m_velocity_texture) render_driver->delete_texture(m_velocity_texture);
			if (m_light_target)  render_driver->delete_render_target(m_light_target);
			if (m_light_texture) render_driver->delete_texture(m_light_texture);
		}
	}

	//context
	Square::Context& DrawerPassDeferred::context() { return m_context; }
	const Square::Context& DrawerPassDeferred::context() const { return m_context; }
	//render
	Render::Context& DrawerPassDeferred::render() { return *System::get<RenderSystem>(context())->render(); }
	const Render::Context& DrawerPassDeferred::render() const { return *System::get<RenderSystem>(context())->render(); }

	bool DrawerPassDeferred::build_buffers(const IVec2& size)
	{
		//already the right size?
		if (m_gbuffer && m_size == size) return true;
		//save size
		m_size = size;
		//the occlusion target is on the G-Buffer that is replaced
		if (m_occlusion_target) render().delete_render_target(m_occlusion_target);
		//G-Buffer: depth/normal/albedo/emissive + depth (24 bytes per pixel): the depth along the
		//view as a 32 bit float (the world position comes back from it and the camera:
		//GBufferPosition.hlsl, precise far from the camera); the normal (octahedral), the
		//roughness/shininess, the shading model and the HDR emissive fit 16 bit floats; albedo and
		//metallic are in [0, 1]
		Vector<GBuffer::BufferFormat> formats
		{
			GBuffer::BufferFormat(TF_R32F,    TT_R,    TTF_FLOAT, RT_COLOR),          // GB_POSITION (depth)
			GBuffer::BufferFormat(TF_RGBA16F, TT_RGBA, TTF_FLOAT, RT_COLOR),          // GB_NORMAL
			GBuffer::BufferFormat(TF_RGBA8,   TT_RGBA, TTF_UNSIGNED_BYTE, RT_COLOR),  // GB_ALBEDO
			GBuffer::BufferFormat(TF_RGBA16F, TT_RGBA, TTF_FLOAT, RT_COLOR),          // GB_EMISSIVE
			//same depth format as the screen (24 bit + 8 stencil): copying the depth to the screen at the end
			//of the pass needs matching formats (D3D11 CopySubresourceRegion, GL glBlitFramebuffer)
			GBuffer::BufferFormat(TF_DEPTH24_STENCIL8, TT_DEPTH_STENCIL, TTF_UNSIGNED_INT_24_8, RT_DEPTH) // GB_DEPTH
		};
		m_gbuffer = MakeShared<GBuffer>(context(), size, formats);
		if (!m_gbuffer->target()) return false;
		//light accumulation texture (HDR, linear) sharing the G-Buffer depth
		if (m_light_target)  render().delete_render_target(m_light_target);
		if (m_light_texture) render().delete_texture(m_light_texture);
		m_light_texture = render().create_texture(
			{ TF_RGBA16F, (unsigned int)size.x, (unsigned int)size.y, nullptr, TT_RGBA, TTF_FLOAT, false },
			{ TMIN_NEAREST, TMAG_NEAREST, TEDGE_CLAMP, TEDGE_CLAMP, TEDGE_CLAMP }
		);
		m_light_target = render().create_render_target(
		{
			  Render::TargetField{ m_light_texture, RT_COLOR }
			, Render::TargetField{ m_gbuffer->texture(GB_DEPTH), RT_DEPTH }
		});
		//the G-Buffer occlusion alone (GT3: emissive | occlusion), for the G-Buffer post effects
		m_occlusion_target = render().create_render_target({ Render::TargetField{ m_gbuffer->texture(GB_EMISSIVE), RT_COLOR } });
		//the velocity (uv on the screen) sharing the G-Buffer depth
		if (m_velocity_target)  render().delete_render_target(m_velocity_target);
		if (m_velocity_texture) render().delete_texture(m_velocity_texture);
		m_velocity_texture = render().create_texture(
			{ TF_RG16F, (unsigned int)size.x, (unsigned int)size.y, nullptr, TT_RG, TTF_FLOAT, false },
			{ TMIN_NEAREST, TMAG_NEAREST, TEDGE_CLAMP, TEDGE_CLAMP, TEDGE_CLAMP }
		);
		m_velocity_target = render().create_render_target(
		{
			  Render::TargetField{ m_velocity_texture, RT_COLOR }
			, Render::TargetField{ m_gbuffer->texture(GB_DEPTH), RT_DEPTH }
		});
		return m_light_target != nullptr && m_occlusion_target != nullptr;
	}

	void DrawerPassDeferred::bind_gbuffer(Resource::Shader* shader)
	{
		static const char* s_uniform_names[]{ "g_position", "g_normal", "g_albedo", "g_emissive" };
		for (size_t texture_id = 0; texture_id != 4; ++texture_id)
		{
			//a light shader may not read every G-Buffer texture (e.g. ambient ignores the normal):
			//the compiler strips the unused ones, so a missing uniform is not an error
			if (auto uniform_texture = shader->uniform(s_uniform_names[texture_id]))
			{
				uniform_texture->set(m_gbuffer->texture(texture_id));
			}
		}
	}

	void DrawerPassDeferred::geometry_pass(const Vec4& clear_color, int num_of_pass, const Camera& camera, const PoolQueues& queues, const SoftwareOcclusion* occlusion)
	{
		//bind G-Buffer
		render().enable_render_target(m_gbuffer->target());
		render().set_viewport_state({ camera.viewport().viewport() });
		//clear on the first camera pass: the clear color (the distance along the view of a surface
		//is written negative, the clear >= 0 is the background; alpha 0: the background shading
		//model id, in the normal)
		if (num_of_pass == 0)
		{
			render().set_clear_color_state({ Vec4(clear_color.x, clear_color.y, clear_color.z, 0.0f) });
			render().clear();
		}
		//buffers (the transforms: by the list of the draws)
		Render::UniformBufferCamera ucamera;
		//parameters (only camera + transform are needed by the geometry technique)
		EffectPassInputs inputs{ m_cb_camera.get(), m_cb_transform.get(), Vec4(1.0f) };
		//what the camera sees: the parts of a renderable out of it, or hidden, not drawn
		inputs.m_frustum = &camera.frustum();
		inputs.m_occlusion = occlusion;
		//update camera
		camera.set(&ucamera);
		render().update_steam_CB(m_cb_camera.get(), (const unsigned char*)&ucamera, sizeof(ucamera));
		//the opaque queue in batches (by pass and material; front to back in a batch), each
		//renderable its transform (and the cross-fade of its level of detail)
		static const String s_technique{ "deferred" };
		m_gbuffer_draws.clear();
		for (auto randerable : RenderableQuery(queues, { RQ_OPAQUE }))
		{
			if (randerable && randerable->can_draw())
			{
				m_gbuffer_draws.add(randerable, s_technique);
			}
		}
		m_gbuffer_draws.draw(render(), inputs);
		//unbind G-Buffer
		render().disable_render_target(m_gbuffer->target());
	}

	bool DrawerPassDeferred::velocity_pass(const Camera& camera, const PoolQueues& queues)
	{
		if (!m_velocity_target || !m_shader_velocity || !m_shader_velocity->base_shader()) return false;
		//the camera of the last frame (the first one, a cut: this one, no motion of the camera)
		const Mat4& view = camera.view();
		const Mat4& projection = camera.projection();
		const Vec3  eye = Vec3(inverse(view)[3]);
		if (!m_previous_camera || length(eye - m_previous_eye) > s_velocity_camera_cut)
		{
			m_previous_view = view;
			m_previous_projection = projection;
			m_previous_models.clear();
		}
		//the target: 0 everywhere, the G-Buffer depth read only (the same projection: equal)
		render().enable_render_target(m_velocity_target);
		render().set_viewport_state({ camera.viewport().viewport() });
		render().set_clear_color_state({ Vec4(0.0f) });
		render().clear(CLEAR_COLOR);
		render().set_depth_buffer_state({ DT_LESS_EQUAL, DM_ENABLE_ONLY_READ });
		render().set_blend_state({});
		render().set_cullface_state({ CF_BACK });
		m_shader_velocity->bind();
		render().bind_uniform_CB(m_cb_camera.get(), m_shader_velocity->base_shader(), "Camera");
		render().bind_uniform_CB(m_cb_transform.get(), m_shader_velocity->base_shader(), "Transform");
		render().bind_uniform_CB(m_cb_velocity.get(), m_shader_velocity->base_shader(), "Velocity");
		Render::UniformBufferTransform utransform;
		UniformVelocity uvelocity;
		uvelocity.m_previous_view = m_previous_view;
		uvelocity.m_previous_projection = m_previous_projection;
		bool drawn = false;
		m_current_models.clear();
		for (auto randerable : RenderableQuery(queues, { RQ_OPAQUE }))
		{
			if (!randerable || !randerable->motion_blur() || !randerable->can_draw()) continue;
			auto transform = randerable->transform().lock();
			if (!transform) continue;
			transform->set(&utransform);
			render().update_steam_CB(m_cb_transform.get(), (const unsigned char*)&utransform, sizeof(utransform));
			//its model in the last frame (new: this one)
			const Mat4& model = transform->global_model_matrix();
			auto previous = m_previous_models.find(randerable.get());
			uvelocity.m_previous_model = previous != m_previous_models.end() ? previous->second : model;
			Render::update_constant_buffer(&render(), m_cb_velocity.get(), &uvelocity);
			drawn |= randerable->draw_geometry(render());
			m_current_models[randerable.get()] = model;
		}
		m_shader_velocity->unbind();
		render().disable_render_target(m_velocity_target);
		//this frame: the last one of the next
		std::swap(m_previous_models, m_current_models);
		m_previous_view = view;
		m_previous_projection = projection;
		m_previous_eye = eye;
		m_previous_camera = true;
		return drawn;
	}

	void DrawerPassDeferred::light_pass(const Vec4& ambient_color, const Camera& camera, const PoolQueues& queues)
	{
		//all the light shaders are required
		if (!m_shader_ambient   || !m_shader_ambient->base_shader()
		||  !m_shader_direction || !m_shader_direction->base_shader()
		||  !m_shader_point     || !m_shader_point->base_shader()
		||  !m_shader_spot      || !m_shader_spot->base_shader())
		{
			context().logger()->warning("DrawerPassDeferred: missing deferred light shaders, light pass skipped");
			return;
		}
		//bind and clear the light accumulation target
		render().enable_render_target(m_light_target);
		render().set_viewport_state({ camera.viewport().viewport() });
		render().set_clear_color_state({ Vec4(0.0f, 0.0f, 0.0f, 1.0f) });
		render().clear(Render::CLEAR_COLOR);
		//additive blending; the volume passes rely on the GREATER_EQUAL depth
		//test to light each pixel exactly once
		render().set_blend_state({ BLEND_ONE, BLEND_ONE });
		render().set_depth_buffer_state({ DM_DISABLE });
		render().set_cullface_state({ CF_BACK });

		//////////////////////////////////////////////////////////////////
		// AMBIENT (+ emissive), full-screen
		//////////////////////////////////////////////////////////////////
		{
			SQUARE_RENDER_SCOPE(render(), "Ambient");
			m_shader_ambient->bind();
			render().bind_uniform_CB(m_cb_camera.get(), m_shader_ambient->base_shader(), "Camera");
			bind_gbuffer(m_shader_ambient.get());
			if (auto uniform_light = m_shader_ambient->uniform("light"))
			{
				uniform_light->set(ambient_color);
			}
			else
			{
				context().logger()->warning("DrawerPassDeferred: 'light' uniform not found in ambient shader");
			}
			m_quad->draw(render());
			m_shader_ambient->unbind();
		}

		//////////////////////////////////////////////////////////////////
		// DIRECTIONAL lights, full-screen
		//////////////////////////////////////////////////////////////////
		if (queues[RQ_DIRECTION_LIGHT].size())
		{
			SQUARE_RENDER_SCOPE(render(), "Direction lights");
			//the shaders: no shadow, shadow (its filter, by the light, in the shader)
			for (bool with_shadow : { false, true })
			{
				auto& shader = with_shadow ? m_shader_direction_shadow : m_shader_direction;
				if (!shader || !shader->base_shader()) continue;
				bool shader_bound = false;
				for (auto weak_light : queues[RQ_DIRECTION_LIGHT])
				if (auto light = weak_light->lock<Render::Light>())
				{
					//jump?
					if (!light->visible()) continue;
					if (light->shadow() != with_shadow) continue;
					//bind only when a light of this kind exists
					if (!shader_bound)
					{
						shader->bind();
						render().bind_uniform_CB(m_cb_camera.get(), shader->base_shader(), "Camera");
						bind_gbuffer(shader.get());
						render().bind_uniform_CB(m_cb_direction_light.get(), shader->base_shader(), "Light");
						if (with_shadow)
						{
							render().bind_uniform_CB(m_cb_direction_shadow_light.get(), shader->base_shader(), "DirectionShadowCamera");
						}
						shader_bound = true;
					}
					//update light buffer
					Render::UniformDirectionLight udirection_light;
					light->set(&udirection_light);
					Render::update_constant_buffer(&render(), m_cb_direction_light.get(), &udirection_light);
					//shadow
					if (with_shadow)
					{
						Render::UniformDirectionShadowLight udirection_shadow_light;
						light->set(&udirection_shadow_light, &camera, false);
						Render::update_constant_buffer(&render(), m_cb_direction_shadow_light.get(), &udirection_shadow_light);
						//the sun of the post effects: the first one (its cascades stay in the buffer)
						if (!m_sun_shadow_map)
						{
							m_sun_shadow_map = light->shadow_buffer().texture();
							m_sun_direction = udirection_light.m_direction;
							m_sun_color = udirection_light.m_diffuse;
						}
						if (auto uniform_shadow_map = shader->uniform("direction_shadow_map"))
						{
							uniform_shadow_map->set(light->shadow_buffer().texture());
						}
					}
					//draw
					m_quad->draw(render());
				}
				if (shader_bound)
				{
					shader->unbind();
				}
			}
		}

		//////////////////////////////////////////////////////////////////
		// POINT lights, sphere volumes (read-only depth >=)
		//////////////////////////////////////////////////////////////////
		if (queues[RQ_POINT_LIGHT].size())
		{
			SQUARE_RENDER_SCOPE(render(), "Point lights");
			//only the volume back faces behind the shaded geometry pass the test
			render().set_depth_buffer_state({ DT_GREATER_EQUAL, DM_ENABLE_ONLY_READ });
			for (bool with_shadow : { false, true })
			{
				auto& shader = with_shadow ? m_shader_point_shadow : m_shader_point;
				if (!shader || !shader->base_shader()) continue;
				bool shader_bound = false;
				for (auto weak_light : queues[RQ_POINT_LIGHT])
				if (auto light = weak_light->lock<Render::PointLight>())
				{
					//jump?
					if (!light->visible()) continue;
					if (light->shadow() != with_shadow) continue;
					//bind only when a light of this kind exists
					if (!shader_bound)
					{
						shader->bind();
						render().bind_uniform_CB(m_cb_camera.get(), shader->base_shader(), "Camera");
						bind_gbuffer(shader.get());
						render().bind_uniform_CB(m_cb_point_light.get(), shader->base_shader(), "Light");
						render().bind_uniform_CB(m_cb_light_volume.get(), shader->base_shader(), "LightVolume");
						if (with_shadow)
						{
							render().bind_uniform_CB(m_cb_point_shadow_light.get(), shader->base_shader(), "PointShadowCamera");
						}
						shader_bound = true;
					}
					//update light buffer
					Render::UniformPointLight upoint_light;
					light->set(&upoint_light);
					Render::update_constant_buffer(&render(), m_cb_point_light.get(), &upoint_light);
					//shadow
					if (with_shadow)
					{
						Render::UniformPointShadowLight upoint_shadow_light;
						//call through the base: PointLight::set(UniformPointLight*) hides the shadow overloads,
						//and a qualified call (light->Render::Light::set) would skip the virtual override
						static_cast<const Render::Light&>(*light).set(&upoint_shadow_light, false);
						Render::update_constant_buffer(&render(), m_cb_point_shadow_light.get(), &upoint_shadow_light);
						if (auto uniform_shadow_map = shader->uniform("point_shadow_map"))
						{
							uniform_shadow_map->set(light->shadow_buffer().texture());
						}
					}
					//sphere volume
					draw_light_volume(render(), m_cb_light_volume, m_sphere, LightVolume::point_light_model(upoint_light));
				}
				if (shader_bound)
				{
					shader->unbind();
				}
			}
			render().set_depth_buffer_state({ DM_DISABLE });
		}

		//////////////////////////////////////////////////////////////////
		// SPOT lights, cone volumes (read-only depth >=)
		//////////////////////////////////////////////////////////////////
		if (queues[RQ_SPOT_LIGHT].size())
		{
			SQUARE_RENDER_SCOPE(render(), "Spot lights");
			//only the volume back faces behind the shaded geometry pass the test
			render().set_depth_buffer_state({ DT_GREATER_EQUAL, DM_ENABLE_ONLY_READ });
			for (bool with_shadow : { false, true })
			{
				auto& shader = with_shadow ? m_shader_spot_shadow : m_shader_spot;
				if (!shader || !shader->base_shader()) continue;
				bool shader_bound = false;
				for (auto weak_light : queues[RQ_SPOT_LIGHT])
				if (auto light = weak_light->lock<Render::SpotLight>())
				{
					//jump?
					if (!light->visible()) continue;
					if (light->shadow() != with_shadow) continue;
					//bind only when a light of this kind exists
					if (!shader_bound)
					{
						shader->bind();
						render().bind_uniform_CB(m_cb_camera.get(), shader->base_shader(), "Camera");
						bind_gbuffer(shader.get());
						render().bind_uniform_CB(m_cb_spot_light.get(), shader->base_shader(), "Light");
						render().bind_uniform_CB(m_cb_light_volume.get(), shader->base_shader(), "LightVolume");
						if (with_shadow)
						{
							render().bind_uniform_CB(m_cb_spot_shadow_light.get(), shader->base_shader(), "SpotShadowCamera");
						}
						shader_bound = true;
					}
					//update light buffer
					Render::UniformSpotLight uspot_light;
					light->set(&uspot_light);
					Render::update_constant_buffer(&render(), m_cb_spot_light.get(), &uspot_light);
					//shadow
					if (with_shadow)
					{
						Render::UniformSpotShadowLight uspot_shadow_light;
						//call through the base: SpotLight::set(UniformSpotLight*) hides the shadow overloads,
						//and a qualified call (light->Render::Light::set) would skip the virtual override
						static_cast<const Render::Light&>(*light).set(&uspot_shadow_light, false);
						Render::update_constant_buffer(&render(), m_cb_spot_shadow_light.get(), &uspot_shadow_light);
						if (auto uniform_shadow_map = shader->uniform("spot_shadow_map"))
						{
							uniform_shadow_map->set(light->shadow_buffer().texture());
						}
					}

					// Draw volume
					draw_light_volume(render(), m_cb_light_volume, m_cone, LightVolume::spot_light_model(uspot_light));
				}
				if (shader_bound)
				{
					shader->unbind();
				}
			}
			render().set_depth_buffer_state({ DM_DISABLE });
		}

		//unbind light accumulation target and restore state
		render().disable_render_target(m_light_target);
		render().set_blend_state({});
		render().set_depth_buffer_state({ DM_ENABLE_AND_WRITE });
		render().set_cullface_state({ CF_BACK });
	}

	void DrawerPassDeferred::draw
	(
	  Drawer&           drawer
	, int               num_of_pass
	, const Vec4&       clear_color
	, const Vec4&       ambient_color
	, const Camera&     camera
	, const Collection& collection
	, const PoolQueues& queues
	)
	{
		//the scene at a share of the size of the camera (then stretched to it by the present)
		const float scale = drawer.resolution_scale();
		const ScaledCamera scaled(camera, scale);
		const Camera& scene = scale < 1.0f ? static_cast<const Camera&>(scaled) : camera;
		//viewport size in pixels
		const IVec4& viewport = scene.viewport().viewport();
		const IVec2 size((int)viewport.z, (int)viewport.w);
		if (size.x <= 0 || size.y <= 0) return;
		//(re)build buffers if needed
		if (!build_buffers(size)) return;
		//post effects of the world
		const auto& post_effects = drawer.post_effects();
		//1) geometry into the G-Buffer
		{
			SQUARE_RENDER_SCOPE(render(), "G-Buffer");
			geometry_pass(clear_color, num_of_pass, scene, queues, &drawer.occlusion());
		}
		//1a) the velocity of the renderables with their own motion blur (an effect needs it)
		m_velocity_drawn = false;
		m_sun_shadow_map = nullptr;
		if (PostEffectChain::any_velocity(post_effects))
		{
			SQUARE_RENDER_SCOPE(render(), "Velocity");
			m_velocity_drawn = velocity_pass(scene, queues);
		}
		//1b) G-Buffer post effects (SSAO...)
		if (PostEffectChain::any(post_effects, PES_GBUFFER))
		{
			SQUARE_RENDER_SCOPE(render(), "G-Buffer effects");
			m_post_effects.draw_gbuffer(post_effects, post_effect_frame(scene));
		}
		//2) accumulate lights into the light buffer
		{
			SQUARE_RENDER_SCOPE(render(), "Lights");
			light_pass(ambient_color, scene, queues);
		}
		//2b) blend the translucent renderables over it (forward shaded)
		{
			SQUARE_RENDER_SCOPE(render(), "Translucent");
			translucent_pass(ambient_color, scene, queues);
		}
		//2c) color post effects, on the light buffer
		Texture* frame = m_light_texture;
		if (PostEffectChain::any(post_effects, PES_COLOR))
		{
			SQUARE_RENDER_SCOPE(render(), "Color effects");
			frame = m_post_effects.draw_color(post_effects, post_effect_frame(scene), m_light_texture);
		}
		//3) present the frame to the screen (or the debug view of a post effect)
		if (Texture* debug = PostEffectChain::debug_texture(post_effects)) frame = debug;
		{
			SQUARE_RENDER_SCOPE(render(), "Present");
			present_pass(camera, frame);
			//4) copy depth for later passes (no-op on backends without blit support; a scene
			//scaled: not the size of the screen, none)
			if (scale >= 1.0f)
			{
				const IVec4 area(0, 0, size.x, size.y);
				render().copy_target_to_target(area, m_gbuffer->target(), area, nullptr, RT_DEPTH);
			}
		}
	}

	void DrawerPassDeferred::translucent_pass(const Vec4& ambient_color, const Camera& camera, const PoolQueues& queues)
	{
		if (!queues[RQ_TRANSLUCENT].size()) return;
		//the light buffer: linear HDR, with the G-Buffer depth attached (hidden by the opaque scene).
		//Blend and depth states come from the passes of the "translucent" technique
		//(depth test without write, alpha blending): see PBRTranslucent.sqfx
		render().enable_render_target(m_light_target);
		render().set_viewport_state({ camera.viewport().viewport() });
		draw_forward
		(
			  render()
			, "translucent"
			, camera
			, ambient_color
			, queues
			, { RQ_TRANSLUCENT }
			, ForwardShadingBuffers
			  {
				  m_cb_camera.get()
				, m_cb_transform.get()
				, m_cb_direction_light.get()
				, m_cb_point_light.get()
				, m_cb_spot_light.get()
				, m_cb_direction_shadow_light.get()
				, m_cb_point_shadow_light.get()
				, m_cb_spot_shadow_light.get()
			  }
		);
		render().disable_render_target(m_light_target);
		//restore state
		render().set_blend_state({});
		render().set_depth_buffer_state({ DM_ENABLE_AND_WRITE });
		render().set_cullface_state({ CF_BACK });
	}

	PostEffectFrame DrawerPassDeferred::post_effect_frame(const Camera& camera)
	{
		PostEffectFrame frame;
		frame.m_render        = &render();
		frame.m_camera        = &camera;
		frame.m_camera_buffer = m_cb_camera.get(); //updated by the geometry pass
		frame.m_size          = m_size;
		frame.m_viewport      = camera.viewport().viewport();
		frame.m_quad          = m_quad.get();
		frame.m_gbuffer       = m_gbuffer.get();
		frame.m_occlusion     = m_occlusion_target;
		frame.m_velocity      = m_velocity_drawn ? m_velocity_texture : nullptr;
		frame.m_linear        = true; //the light buffer is linear HDR
		frame.m_sun_direction = m_sun_direction;
		frame.m_sun_color     = m_sun_color;
		frame.m_sun_shadow_map = m_sun_shadow_map;
		frame.m_sun_shadow_buffer = m_sun_shadow_map ? m_cb_direction_shadow_light.get() : nullptr;
		return frame;
	}

	void DrawerPassDeferred::present_pass(const Camera& camera, Texture* frame)
	{
		//present shader is required
		if (!m_shader_present || !m_shader_present->base_shader())
		{
			context().logger()->warning("DrawerPassDeferred: missing present shader, present pass skipped");
			return;
		}
		//draw on the default (screen) target
		render().set_viewport_state({ camera.viewport().viewport() });
		render().set_depth_buffer_state({ DM_DISABLE });
		render().set_blend_state({});
		render().set_cullface_state({ CF_BACK });
		//draw
		m_shader_present->bind();
		if (auto uniform_light = m_shader_present->uniform("g_light"))
		{
			uniform_light->set(frame);
		}
		//(the screen: the frame stretched to it, a scene scaled)
		if (auto uniform_viewport = m_shader_present->uniform("g_viewport"))
		{
			uniform_viewport->set(Vec4(camera.viewport().viewport()));
		}
		else
		{
			context().logger()->warning("DrawerPassDeferred: 'g_light' uniform not found in present shader");
		}
		m_quad->draw(render());
		m_shader_present->unbind();
		//restore state for the passes that follow (UI/debug)
		render().set_depth_buffer_state({ DM_ENABLE_AND_WRITE });
		render().set_cullface_state({ CF_BACK });
	}
}
}
