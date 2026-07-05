//
//  DrawerPassDeferred.cpp
//  Square
//
//  See DrawerPassDeferred.h for the high level description.
//
#include "Square/Core/Context.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Material.h"
#include "Square/Render/Effect.h"
#include "Square/Render/Camera.h"
#include "Square/Render/Viewport.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Light.h"
#include "Square/Render/ShadowBuffer.h"
#include "Square/Render/DrawerPassDeferred.h"
#include "Square/Resource/Shader.h"
#include <cmath>

namespace Square
{
namespace Render
{
	//light volume model matrix
	struct UniformLightVolume
	{
		Mat4 m_model;
	};

	//////////////////////////////////////////////////////////////////////
	// Volume meshes (unit space, scaled/positioned by the light pass)
	//////////////////////////////////////////////////////////////////////
	static Shared<Mesh> build_fullscreen_quad(Square::Context& context)
	{
		Mesh::Vertex3DList vertices
		{
			{ Vec3(-1.0f, -1.0f, 0.0f) },
			{ Vec3( 1.0f, -1.0f, 0.0f) },
			{ Vec3( 1.0f,  1.0f, 0.0f) },
			{ Vec3(-1.0f,  1.0f, 0.0f) },
		};
		Mesh::IndexList indices{ 0, 2, 1, 0, 3, 2 };
		auto mesh = MakeShared<Mesh>(context);
		mesh->build(vertices, indices);
		return mesh;
	}

	static Shared<Mesh> build_sphere(Square::Context& context, unsigned int rings = 12, unsigned int sectors = 24)
	{
		Mesh::Vertex3DList vertices;
		Mesh::IndexList    indices;
		const float ring_step   = 1.0f / (float)(rings - 1);
		const float sector_step = 1.0f / (float)(sectors - 1);
		for (unsigned int ring = 0; ring < rings; ++ring)
		for (unsigned int sector = 0; sector < sectors; ++sector)
		{
			const float y = std::sin(-Constants::pi<float>() * 0.5f + Constants::pi<float>() * ring * ring_step);
			const float x = std::cos(2.0f * Constants::pi<float>() * sector * sector_step) * std::sin(Constants::pi<float>() * ring * ring_step);
			const float z = std::sin(2.0f * Constants::pi<float>() * sector * sector_step) * std::sin(Constants::pi<float>() * ring * ring_step);
			vertices.push_back({ Vec3(x, y, z) });
		}
		for (unsigned int ring = 0; ring + 1 < rings; ++ring)
		for (unsigned int sector = 0; sector + 1 < sectors; ++sector)
		{
			unsigned int index_bottom_left  = ring * sectors + sector;
			unsigned int index_bottom_right = ring * sectors + (sector + 1);
			unsigned int index_top_right    = (ring + 1) * sectors + (sector + 1);
			unsigned int index_top_left     = (ring + 1) * sectors + sector;
			indices.insert(indices.end(), { index_bottom_left, index_bottom_right, index_top_right,
			                                index_bottom_left, index_top_right,    index_top_left });
		}
		auto mesh = MakeShared<Mesh>(context);
		mesh->build(vertices, indices);
		return mesh;
	}

	//cone: apex at origin, base circle of radius 1 at z=+1
	static Shared<Mesh> build_cone(Square::Context& context, unsigned int sectors = 24)
	{
		Mesh::Vertex3DList vertices;
		Mesh::IndexList    indices;
		//apex (0) and base center (1)
		vertices.push_back({ Vec3(0.0f, 0.0f, 0.0f) });
		vertices.push_back({ Vec3(0.0f, 0.0f, 1.0f) });
		//base ring
		const unsigned int base_start = 2;
		for (unsigned int sector = 0; sector < sectors; ++sector)
		{
			const float angle = 2.0f * Constants::pi<float>() * (float)sector / (float)sectors;
			vertices.push_back({ Vec3(std::cos(angle), std::sin(angle), 1.0f) });
		}
		//sides and base cap
		for (unsigned int sector = 0; sector < sectors; ++sector)
		{
			unsigned int index_current = base_start + sector;
			unsigned int index_next    = base_start + (sector + 1) % sectors;
			indices.insert(indices.end(), { 0u, index_current, index_next });
			indices.insert(indices.end(), { 1u, index_next, index_current });
		}
		auto mesh = MakeShared<Mesh>(context);
		mesh->build(vertices, indices);
		return mesh;
	}

	//////////////////////////////////////////////////////////////////////
	// DrawerPassDeferred
	//////////////////////////////////////////////////////////////////////
	DrawerPassDeferred::DrawerPassDeferred(Square::Context& context)
	: DrawerPass(context.allocator(), RPT_RENDER)
	, m_context(context)
	{
		//constant buffers
		m_cb_camera          = Render::stream_constant_buffer<Render::UniformBufferCamera>(&render());
		m_cb_transform       = Render::stream_constant_buffer<Render::UniformBufferTransform>(&render());
		m_cb_direction_light = Render::stream_constant_buffer<Render::UniformDirectionLight>(&render());
		m_cb_point_light     = Render::stream_constant_buffer<Render::UniformPointLight>(&render());
		m_cb_spot_light      = Render::stream_constant_buffer<Render::UniformSpotLight>(&render());
		m_cb_light_volume    = Render::stream_constant_buffer<UniformLightVolume>(&render());
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
		//volume meshes
		m_quad   = build_fullscreen_quad(context);
		m_sphere = build_sphere(context);
		m_cone   = build_cone(context);
	}

	DrawerPassDeferred::~DrawerPassDeferred()
	{
		if (auto render_driver = context().render())
		{
			if (m_light_target)  render_driver->delete_render_target(m_light_target);
			if (m_light_texture) render_driver->delete_texture(m_light_texture);
		}
	}

	//context
	Square::Context& DrawerPassDeferred::context() { return m_context; }
	const Square::Context& DrawerPassDeferred::context() const { return m_context; }
	//render
	Render::Context& DrawerPassDeferred::render() { return *context().render(); }
	const Render::Context& DrawerPassDeferred::render() const { return *context().render(); }

	bool DrawerPassDeferred::build_buffers(const IVec2& size)
	{
		//already the right size?
		if (m_gbuffer && m_size == size) return true;
		//save size
		m_size = size;
		//G-Buffer: position/normal/albedo/emissive + depth
		std::vector<GBuffer::BufferFormat> formats
		{
			GBuffer::BufferFormat(),                                                   // GB_POSITION
			GBuffer::BufferFormat(),                                                   // GB_NORMAL
			GBuffer::BufferFormat(),                                                   // GB_ALBEDO
			GBuffer::BufferFormat(),                                                   // GB_EMISSIVE
			GBuffer::BufferFormat(TF_DEPTH_COMPONENT32, TT_DEPTH, TTF_FLOAT, RT_DEPTH) // GB_DEPTH
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
		return m_light_target != nullptr;
	}

	void DrawerPassDeferred::bind_gbuffer(Resource::Shader* shader)
	{
		static const char* s_uniform_names[]{ "g_position", "g_normal", "g_albedo", "g_emissive" };
		for (size_t texture_id = 0; texture_id != 4; ++texture_id)
		{
			if (auto uniform_texture = shader->uniform(s_uniform_names[texture_id]))
			{
				uniform_texture->set(m_gbuffer->texture(texture_id));
			}
			else
			{
				context().logger()->warning(std::string("DrawerPassDeferred: uniform not found: ") + s_uniform_names[texture_id]);
			}
		}
	}

	void DrawerPassDeferred::geometry_pass(const Vec4& clear_color, int num_of_pass, const Camera& camera, const PoolQueues& queues)
	{
		//bind G-Buffer
		render().enable_render_target(m_gbuffer->target());
		render().set_viewport_state({ camera.viewport().viewport() });
		//clear on the first camera pass, alpha 0 = background shading model id
		if (num_of_pass == 0)
		{
			render().set_clear_color_state({ Vec4(clear_color.x, clear_color.y, clear_color.z, 0.0f) });
			render().clear();
		}
		//buffers
		Render::UniformBufferCamera ucamera;
		Render::UniformBufferTransform utransform;
		//parameters (only camera + transform are needed by the geometry technique)
		EffectPassInputs inputs{ m_cb_camera.get(), m_cb_transform.get(), Vec4(1.0f) };
		//update camera
		camera.set(&ucamera);
		render().update_steam_CB(m_cb_camera.get(), (const unsigned char*)&ucamera, sizeof(ucamera));
		//for each elements of the opaque queue
		for (auto randerable : RenderableQuery(queues, { RQ_OPAQUE }))
		if (randerable)
		{
			//jump?
			if (!randerable->can_draw()) continue;
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
				auto material = randerable->material(material_id).lock();
				if (!material) continue;
				//effect
				auto effect = material->effect();
				auto technique = effect->technique("deferred");
				if (!technique) continue;
				//draw for each pass
				for (auto& pass : *technique)
				for (size_t draw_id = 0; draw_id < pass.m_draw_count; ++draw_id)
				{
					randerable->draw(render(), material_id, inputs, pass, draw_id);
				}
			}
		}
		//unbind G-Buffer
		render().disable_render_target(m_gbuffer->target());
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

		//////////////////////////////////////////////////////////////////
		// DIRECTIONAL lights, full-screen
		//////////////////////////////////////////////////////////////////
		if (queues[RQ_DIRECTION_LIGHT].size())
		{
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
						//qualified call: PointLight::set(UniformPointLight*) hides the base shadow overloads
						light->Render::Light::set(&upoint_shadow_light, false);
						Render::update_constant_buffer(&render(), m_cb_point_shadow_light.get(), &upoint_shadow_light);
						if (auto uniform_shadow_map = shader->uniform("point_shadow_map"))
						{
							uniform_shadow_map->set(light->shadow_buffer().texture());
						}
					}
					//sphere volume: translate to the light, scale to radius (+10% margin)
					const float sphere_scale = light->radius() * 1.1f;
					UniformLightVolume ulight_volume;
					ulight_volume.m_model = glm::translate(Mat4(1.0f), upoint_light.m_position)
					                      * glm::scale(Mat4(1.0f), Vec3(sphere_scale, sphere_scale, sphere_scale));
					Render::update_constant_buffer(&render(), m_cb_light_volume.get(), &ulight_volume);
					//draw
					m_sphere->draw(render());
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
						//qualified call: SpotLight::set(UniformSpotLight*) hides the base shadow overloads
						light->Render::Light::set(&uspot_shadow_light, false);
						Render::update_constant_buffer(&render(), m_cb_spot_shadow_light.get(), &uspot_shadow_light);
						if (auto uniform_shadow_map = shader->uniform("spot_shadow_map"))
						{
							uniform_shadow_map->set(light->shadow_buffer().texture());
						}
					}
					//cone volume: orient +z to the light direction, scale by cone size
					const Vec3  light_direction = normalize(uspot_light.m_direction);
					const float cone_height     = uspot_light.m_radius * 1.1f;
					const float cos_outer       = clamp(uspot_light.m_outer_cut_off, -1.0f, 1.0f);
					const float base_radius     = cone_height * std::tan(std::acos(cos_outer)) * 1.1f + 1e-3f;
					//rotation from +z to the light direction
					const Vec3  z_axis(0.0f, 0.0f, 1.0f);
					const float cos_angle = clamp(dot(z_axis, light_direction), -1.0f, 1.0f);
					Mat4 rotation(1.0f);
					if (cos_angle < -0.9999f)
					{
						rotation = to_mat4(angle_axis(Constants::pi<float>(), Vec3(1.0f, 0.0f, 0.0f)));
					}
					else if (cos_angle < 0.9999f)
					{
						rotation = to_mat4(angle_axis(std::acos(cos_angle), normalize(cross(z_axis, light_direction))));
					}
					UniformLightVolume ulight_volume;
					ulight_volume.m_model = glm::translate(Mat4(1.0f), Vec3(uspot_light.m_position))
					                      * rotation
					                      * glm::scale(Mat4(1.0f), Vec3(base_radius, base_radius, cone_height));
					Render::update_constant_buffer(&render(), m_cb_light_volume.get(), &ulight_volume);
					//draw
					m_cone->draw(render());
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
		//viewport size in pixels
		const Vec4& viewport = camera.viewport().viewport();
		const IVec2 size((int)viewport.z, (int)viewport.w);
		if (size.x <= 0 || size.y <= 0) return;
		//(re)build buffers if needed
		if (!build_buffers(size)) return;
		//1) geometry into the G-Buffer
		geometry_pass(clear_color, num_of_pass, camera, queues);
		//2) accumulate lights into the light buffer
		light_pass(ambient_color, camera, queues);
		//3) present the light buffer to the screen
		present_pass(camera);
		//4) copy depth for later passes (no-op on backends without blit support)
		const IVec4 area(0, 0, size.x, size.y);
		render().copy_target_to_target(area, m_gbuffer->target(), area, nullptr, RT_DEPTH);
	}

	void DrawerPassDeferred::present_pass(const Camera& camera)
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
			uniform_light->set(m_light_texture);
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
