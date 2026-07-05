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
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Square
{
namespace Render
{
	//G-Buffer light volume model matrix
	struct UniformLightVolume
	{
		Mat4 m_model;
	};

	//////////////////////////////////////////////////////////////////////
	// Volume meshes (built once, in unit space; the pass scales/positions them)
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
		Mesh::IndexList indices{ 0, 1, 2, 0, 2, 3 };
		auto mesh = MakeShared<Mesh>(context);
		mesh->build(vertices, indices);
		return mesh;
	}

	static Shared<Mesh> build_sphere(Square::Context& context, unsigned int rings = 12, unsigned int sectors = 24)
	{
		Mesh::Vertex3DList vertices;
		Mesh::IndexList    indices;
		const float R = 1.0f / (float)(rings - 1);
		const float S = 1.0f / (float)(sectors - 1);
		for (unsigned int r = 0; r < rings; ++r)
		for (unsigned int s = 0; s < sectors; ++s)
		{
			const float y = std::sin(-Constants::pi<float>() * 0.5f + Constants::pi<float>() * r * R);
			const float x = std::cos(2.0f * Constants::pi<float>() * s * S) * std::sin(Constants::pi<float>() * r * R);
			const float z = std::sin(2.0f * Constants::pi<float>() * s * S) * std::sin(Constants::pi<float>() * r * R);
			vertices.push_back({ Vec3(x, y, z) });
		}
		for (unsigned int r = 0; r + 1 < rings; ++r)
		for (unsigned int s = 0; s + 1 < sectors; ++s)
		{
			unsigned int a = r * sectors + s;
			unsigned int b = r * sectors + (s + 1);
			unsigned int c = (r + 1) * sectors + (s + 1);
			unsigned int d = (r + 1) * sectors + s;
			indices.insert(indices.end(), { a, b, c, a, c, d });
		}
		auto mesh = MakeShared<Mesh>(context);
		mesh->build(vertices, indices);
		return mesh;
	}

	//Cone: apex at origin, base circle of radius 1 at z=+1 (opens toward +Z).
	static Shared<Mesh> build_cone(Square::Context& context, unsigned int sectors = 24)
	{
		Mesh::Vertex3DList vertices;
		Mesh::IndexList    indices;
		//apex
		vertices.push_back({ Vec3(0.0f, 0.0f, 0.0f) }); // 0
		//base center
		vertices.push_back({ Vec3(0.0f, 0.0f, 1.0f) }); // 1
		//base ring
		const unsigned int base_start = 2;
		for (unsigned int s = 0; s < sectors; ++s)
		{
			const float a = 2.0f * Constants::pi<float>() * (float)s / (float)sectors;
			vertices.push_back({ Vec3(std::cos(a), std::sin(a), 1.0f) });
		}
		for (unsigned int s = 0; s < sectors; ++s)
		{
			unsigned int i0 = base_start + s;
			unsigned int i1 = base_start + (s + 1) % sectors;
			//side (apex, ring, ring+1)
			indices.insert(indices.end(), { 0u, i0, i1 });
			//base cap (center, ring+1, ring)
			indices.insert(indices.end(), { 1u, i1, i0 });
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
		//debug dump enabled?
		if (const char* dump = std::getenv("SQUARE_DEFERRED_DUMP"))
			m_dump_enabled = dump[0] != '0';
		//debug view of a single buffer?
		if (const char* view = std::getenv("SQUARE_DEFERRED_VIEW"))
		{
			     if (!std::strcmp(view, "position")) m_debug_view = GB_POSITION;
			else if (!std::strcmp(view, "normal"))   m_debug_view = GB_NORMAL;
			else if (!std::strcmp(view, "albedo"))   m_debug_view = GB_ALBEDO;
			else if (!std::strcmp(view, "emissive")) m_debug_view = GB_EMISSIVE;
			else if (!std::strcmp(view, "light"))    m_debug_view = GB_COUNT; //light buffer
		}
		//debug: no light volumes?
		if (const char* novol = std::getenv("SQUARE_DEFERRED_NO_VOLUMES"))
			m_no_volumes = novol[0] != '0';
		//volume meshes
		m_quad   = build_fullscreen_quad(context);
		m_sphere = build_sphere(context);
		m_cone   = build_cone(context);
	}

	DrawerPassDeferred::~DrawerPassDeferred()
	{
		if (auto r = context().render())
		{
			if (m_light_target)  r->delete_render_target(m_light_target);
			if (m_light_texture) r->delete_texture(m_light_texture);
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
		//G-Buffer: position/normal/albedo/emissive (RGBA32F) + depth
		std::vector<GBuffer::BufferFormat> formats
		{
			GBuffer::BufferFormat(),                                              // GB_POSITION
			GBuffer::BufferFormat(),                                              // GB_NORMAL
			GBuffer::BufferFormat(),                                              // GB_ALBEDO
			GBuffer::BufferFormat(),                                              // GB_EMISSIVE
			GBuffer::BufferFormat(TF_DEPTH_COMPONENT32, TT_DEPTH, TTF_FLOAT, RT_DEPTH) // GB_DEPTH
		};
		m_gbuffer = MakeShared<GBuffer>(context(), size, formats, m_dump_enabled);
		if (!m_gbuffer->target()) return false;
		//light accumulation texture (HDR, linear) sharing the G-Buffer depth
		if (m_light_target)  render().delete_render_target(m_light_target);
		if (m_light_texture) render().delete_texture(m_light_texture);
		m_light_texture = render().create_texture(
			{ TF_RGBA16F, (unsigned int)size.x, (unsigned int)size.y, nullptr, TT_RGBA, TTF_FLOAT, false },
			{ TMIN_NEAREST, TMAG_NEAREST, TEDGE_CLAMP, TEDGE_CLAMP, TEDGE_CLAMP, false, 0, 10, 1, m_dump_enabled }
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
		static const char* names[]{ "g_position", "g_normal", "g_albedo", "g_emissive" };
		for (size_t i = 0; i != 4; ++i)
		{
			if (auto u = shader->uniform(names[i])) u->set(m_gbuffer->texture(i));
			else context().logger()->warning(std::string("DrawerPassDeferred: uniform not found: ") + names[i]);
		}
	}

	void DrawerPassDeferred::geometry_pass(const Vec4& clear_color, int num_of_pass, const Camera& camera, const PoolQueues& queues)
	{
		//bind G-Buffer
		render().enable_render_target(m_gbuffer->target());
		render().set_viewport_state({ camera.viewport().viewport() });
		//clear on the first camera pass.
		//alpha 0: position.w is the shading model id, 0 = background (see
		//SurfaceDeferredPBR); the light passes skip those pixels.
		if (num_of_pass == 0)
		{
			render().set_clear_color_state({ Vec4(clear_color.x, clear_color.y, clear_color.z, 0.0f) });
			render().clear();
		}
		//camera buffer
		Render::UniformBufferCamera ucamera;
		camera.set(&ucamera);
		render().update_steam_CB(m_cb_camera.get(), (const unsigned char*)&ucamera, sizeof(ucamera));
		//transform buffer
		Render::UniformBufferTransform utransform;
		//inputs (only camera+transform are needed by the geometry technique)
		EffectPassInputs inputs{ m_cb_camera.get(), m_cb_transform.get(), Vec4(1.0f) };
		//for each opaque renderable
		for (auto randerable : RenderableQuery(queues, { RQ_OPAQUE }))
		if (randerable)
		{
			if (!randerable->can_draw()) continue;
			//transform
			if (auto transform = randerable->transform().lock())
			{
				transform->set(&utransform);
				render().update_steam_CB(m_cb_transform.get(), (const unsigned char*)&utransform, sizeof(utransform));
			}
			//for each material
			for (size_t material_id = 0; material_id != randerable->materials_count(); ++material_id)
			{
				auto material = randerable->material(material_id).lock();
				if (!material) continue;
				auto effect = material->effect();
				auto technique = effect->technique("deferred");
				if (!technique) continue;
				//draw each pass of the deferred technique
				for (auto& pass : *technique)
				for (size_t draw_id = 0; draw_id < pass.m_draw_count; ++draw_id)
					randerable->draw(render(), material_id, inputs, pass, draw_id);
			}
		}
		//unbind G-Buffer
		render().disable_render_target(m_gbuffer->target());
	}

	void DrawerPassDeferred::light_pass(const Vec4& ambient_color, const Camera& camera, const PoolQueues& queues)
	{
		//all the light shaders are required (they may be null if their
		//resources failed to load: skip the pass instead of crashing)
		if (!m_shader_ambient   || !m_shader_ambient->base_shader()
		||  !m_shader_direction || !m_shader_direction->base_shader()
		||  !m_shader_point     || !m_shader_point->base_shader()
		||  !m_shader_spot      || !m_shader_spot->base_shader())
		{
			context().logger()->warning("DrawerPassDeferred: missing deferred light shaders, light pass skipped");
			return;
		}
		//bind light accumulation target
		render().enable_render_target(m_light_target);
		render().set_viewport_state({ camera.viewport().viewport() });
		//clear the accumulator
		render().set_clear_color_state({ Vec4(0.0f, 0.0f, 0.0f, 1.0f) });
		render().clear(Render::CLEAR_COLOR);
		//additive blending, no depth for full-screen passes.
		//NOTE: no face culling anywhere in the light pass: winding conventions
		//differ between backends (Metal front = CW, GL front = CCW) and the
		//volume passes rely on the GREATER_EQUAL depth test (back faces only)
		//to avoid double lighting, not on culling.
		render().set_blend_state({ BLEND_ONE, BLEND_ONE });
		render().set_depth_buffer_state({ DM_DISABLE });
		render().set_cullface_state({ CF_DISABLE });

		//////////////////////////////////////////////////////////////////
		// AMBIENT (+ emissive), full-screen
		//////////////////////////////////////////////////////////////////
		m_shader_ambient->bind();
		render().bind_uniform_CB(m_cb_camera.get(), m_shader_ambient->base_shader(), "Camera");
		bind_gbuffer(m_shader_ambient.get());
		if (auto u = m_shader_ambient->uniform("light")) u->set(ambient_color);
		else context().logger()->warning("DrawerPassDeferred: 'light' uniform not found in ambient shader");
		m_quad->draw(render());
		m_shader_ambient->unbind();

		//////////////////////////////////////////////////////////////////
		// DIRECTIONAL lights, full-screen (pass 0: no shadow, pass 1: shadow)
		//////////////////////////////////////////////////////////////////
		if (queues[RQ_DIRECTION_LIGHT].size())
		for (int with_shadow = 0; with_shadow != 2; ++with_shadow)
		{
			auto& shader = with_shadow ? m_shader_direction_shadow : m_shader_direction;
			if (!shader || !shader->base_shader()) continue;
			bool bound = false;
			for (auto weak_light : queues[RQ_DIRECTION_LIGHT])
			if (auto light = weak_light->lock<Render::Light>())
			{
				if (!light->visible()) continue;
				if (light->shadow() != bool(with_shadow)) continue;
				//lazy bind: only when a light of this kind exists
				if (!bound)
				{
					shader->bind();
					render().bind_uniform_CB(m_cb_camera.get(), shader->base_shader(), "Camera");
					bind_gbuffer(shader.get());
					render().bind_uniform_CB(m_cb_direction_light.get(), shader->base_shader(), "Light");
					if (with_shadow)
						render().bind_uniform_CB(m_cb_direction_shadow_light.get(), shader->base_shader(), "DirectionShadowCamera");
					bound = true;
				}
				Render::UniformDirectionLight udir;
				light->set(&udir);
				render().update_steam_CB(m_cb_direction_light.get(), (const unsigned char*)&udir, sizeof(udir));
				if (with_shadow)
				{
					Render::UniformDirectionShadowLight udirshadow;
					light->set(&udirshadow, &camera, false);
					render().update_steam_CB(m_cb_direction_shadow_light.get(), (const unsigned char*)&udirshadow, sizeof(udirshadow));
					if (auto u = shader->uniform("direction_shadow_map")) u->set(light->shadow_buffer().texture());
				}
				m_quad->draw(render());
			}
			if (bound) shader->unbind();
		}

		//////////////////////////////////////////////////////////////////
		// POINT lights, sphere volumes (front cull, read-only depth >=)
		//////////////////////////////////////////////////////////////////
		if (queues[RQ_POINT_LIGHT].size())
		{
			//debug (SQUARE_DEFERRED_NO_VOLUMES): fullscreen quad instead of the
			//sphere volume; the model matrix turns the NDC quad into clip space.
			const Mat4 fullscreen_model = m_no_volumes ? inverse(camera.projection() * camera.view()) : Mat4(1.0f);
			//read-only reversed depth test: only the volume back faces behind
			//the shaded geometry pass the test (no culling, no double lighting)
			if (!m_no_volumes)
				render().set_depth_buffer_state({ DT_GREATER_EQUAL, DM_ENABLE_ONLY_READ });
			//pass 0: lights without shadow, pass 1: lights with shadow
			for (int with_shadow = 0; with_shadow != 2; ++with_shadow)
			{
				auto& shader = with_shadow ? m_shader_point_shadow : m_shader_point;
				if (!shader || !shader->base_shader()) continue;
				bool bound = false;
				for (auto weak_light : queues[RQ_POINT_LIGHT])
				if (auto light = weak_light->lock<Render::PointLight>())
				{
					if (!light->visible()) continue;
					if (light->shadow() != bool(with_shadow)) continue;
					//lazy bind: only when a light of this kind exists
					if (!bound)
					{
						shader->bind();
						render().bind_uniform_CB(m_cb_camera.get(), shader->base_shader(), "Camera");
						bind_gbuffer(shader.get());
						render().bind_uniform_CB(m_cb_point_light.get(), shader->base_shader(), "Light");
						render().bind_uniform_CB(m_cb_light_volume.get(), shader->base_shader(), "LightVolume");
						if (with_shadow)
							render().bind_uniform_CB(m_cb_point_shadow_light.get(), shader->base_shader(), "PointShadowCamera");
						bound = true;
					}
					Render::UniformPointLight upoint;
					light->set(&upoint);
					render().update_steam_CB(m_cb_point_light.get(), (const unsigned char*)&upoint, sizeof(upoint));
					if (with_shadow)
					{
						Render::UniformPointShadowLight upointshadow;
						//qualified: PointLight::set(UniformPointLight*) hides the base shadow overloads
						light->Render::Light::set(&upointshadow, false);
						render().update_steam_CB(m_cb_point_shadow_light.get(), (const unsigned char*)&upointshadow, sizeof(upointshadow));
						if (auto u = shader->uniform("point_shadow_map")) u->set(light->shadow_buffer().texture());
					}
					//sphere volume model: translate to light, scale to radius (+10% margin)
					const float s = light->radius() * 1.1f;
					UniformLightVolume vol;
					vol.m_model = m_no_volumes
					            ? fullscreen_model
					            : glm::translate(Mat4(1.0f), upoint.m_position) * glm::scale(Mat4(1.0f), Vec3(s, s, s));
					render().update_steam_CB(m_cb_light_volume.get(), (const unsigned char*)&vol, sizeof(vol));
					if (m_no_volumes) m_quad->draw(render());
					else              m_sphere->draw(render());
				}
				if (bound) shader->unbind();
			}
			render().set_depth_buffer_state({ DM_DISABLE });
		}

		//////////////////////////////////////////////////////////////////
		// SPOT lights, cone volumes (front cull, read-only depth >=)
		//////////////////////////////////////////////////////////////////
		if (queues[RQ_SPOT_LIGHT].size())
		{
			//debug (SQUARE_DEFERRED_NO_VOLUMES): fullscreen quad instead of the
			//cone volume; the model matrix turns the NDC quad into clip space.
			const Mat4 fullscreen_model = m_no_volumes ? inverse(camera.projection() * camera.view()) : Mat4(1.0f);
			//read-only reversed depth test: only the volume back faces behind
			//the shaded geometry pass the test (no culling, no double lighting)
			if (!m_no_volumes)
				render().set_depth_buffer_state({ DT_GREATER_EQUAL, DM_ENABLE_ONLY_READ });
			//pass 0: lights without shadow, pass 1: lights with shadow
			for (int with_shadow = 0; with_shadow != 2; ++with_shadow)
			{
				auto& shader = with_shadow ? m_shader_spot_shadow : m_shader_spot;
				if (!shader || !shader->base_shader()) continue;
				bool bound = false;
				for (auto weak_light : queues[RQ_SPOT_LIGHT])
				if (auto light = weak_light->lock<Render::SpotLight>())
				{
					if (!light->visible()) continue;
					if (light->shadow() != bool(with_shadow)) continue;
					//lazy bind: only when a light of this kind exists
					if (!bound)
					{
						shader->bind();
						render().bind_uniform_CB(m_cb_camera.get(), shader->base_shader(), "Camera");
						bind_gbuffer(shader.get());
						render().bind_uniform_CB(m_cb_spot_light.get(), shader->base_shader(), "Light");
						render().bind_uniform_CB(m_cb_light_volume.get(), shader->base_shader(), "LightVolume");
						if (with_shadow)
							render().bind_uniform_CB(m_cb_spot_shadow_light.get(), shader->base_shader(), "SpotShadowCamera");
						bound = true;
					}
					Render::UniformSpotLight uspot;
					light->set(&uspot);
					render().update_steam_CB(m_cb_spot_light.get(), (const unsigned char*)&uspot, sizeof(uspot));
					if (with_shadow)
					{
						Render::UniformSpotShadowLight uspotshadow;
						//qualified: SpotLight::set(UniformSpotLight*) hides the base shadow overloads
						light->Render::Light::set(&uspotshadow, false);
						render().update_steam_CB(m_cb_spot_shadow_light.get(), (const unsigned char*)&uspotshadow, sizeof(uspotshadow));
						if (auto u = shader->uniform("spot_shadow_map")) u->set(light->shadow_buffer().texture());
					}
					//cone volume model: orient +Z to the light direction, scale by cone size
					const Vec3  dir       = normalize(uspot.m_direction);
					const float height    = uspot.m_radius * 1.1f;
					const float cos_outer = clamp(uspot.m_outer_cut_off, -1.0f, 1.0f);
					const float base_r    = height * std::tan(std::acos(cos_outer)) * 1.1f + 1e-3f;
					//rotation from +Z to dir
					const Vec3 z_axis(0.0f, 0.0f, 1.0f);
					const float d = clamp(dot(z_axis, dir), -1.0f, 1.0f);
					Mat4 rotation(1.0f);
					if (d < 0.9999f)
					{
						if (d < -0.9999f)
							rotation = to_mat4(angle_axis(Constants::pi<float>(), Vec3(1.0f, 0.0f, 0.0f)));
						else
							rotation = to_mat4(angle_axis(std::acos(d), normalize(cross(z_axis, dir))));
					}
					UniformLightVolume vol;
					vol.m_model = m_no_volumes
					            ? fullscreen_model
					            : glm::translate(Mat4(1.0f), Vec3(uspot.m_position))
					            * rotation
					            * glm::scale(Mat4(1.0f), Vec3(base_r, base_r, height));
					render().update_steam_CB(m_cb_light_volume.get(), (const unsigned char*)&vol, sizeof(vol));
					if (m_no_volumes) m_quad->draw(render());
					else              m_cone->draw(render());
				}
				if (bound) shader->unbind();
			}
			render().set_depth_buffer_state({ DM_DISABLE });
		}

		//unbind light accumulation target + restore state
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
		const Vec4& vp = camera.viewport().viewport();
		const IVec2 size((int)vp.z, (int)vp.w);
		if (size.x <= 0 || size.y <= 0) return;
		//(re)build buffers if needed
		if (!build_buffers(size)) return;
		//1) geometry into the G-Buffer
		geometry_pass(clear_color, num_of_pass, camera, queues);
		//2) accumulate lights into the light buffer
		light_pass(ambient_color, camera, queues);
		//3) present the light buffer to the screen (a shader pass: not every
		//backend can blit between different formats, e.g. Metal)
		present_pass(camera);
		//4) copy depth for later passes (no-op on backends without blit support)
		const IVec4 area(0, 0, size.x, size.y);
		render().copy_target_to_target(area, m_gbuffer->target(), area, nullptr, RT_DEPTH);
		//debug: dump the buffers once, a few frames in (SQUARE_DEFERRED_DUMP=1)
		if (m_dump_enabled && ++m_frame_counter == 3) dump_buffers();
	}

	void DrawerPassDeferred::present_pass(const Camera& camera)
	{
		//shader may be null if its resource failed to load
		if (!m_shader_present || !m_shader_present->base_shader())
		{
			context().logger()->warning("DrawerPassDeferred: missing present shader, present pass skipped");
			return;
		}
		//draw on the default (screen) target
		render().set_viewport_state({ camera.viewport().viewport() });
		render().set_depth_buffer_state({ DM_DISABLE });
		render().set_blend_state({});
		//no culling: the quad winding is backend-dependent (Metal front = CW)
		render().set_cullface_state({ CF_DISABLE });
		//debug view: show one of the internal buffers instead of the final image
		Render::Texture* source = m_light_texture;
		if (0 <= m_debug_view && m_debug_view < GB_COUNT) source = m_gbuffer->texture(m_debug_view);
		m_shader_present->bind();
		if (auto u = m_shader_present->uniform("g_light")) u->set(source);
		else context().logger()->warning("DrawerPassDeferred: 'g_light' uniform not found in present shader");
		m_quad->draw(render());
		m_shader_present->unbind();
		//restore state for the passes that follow (UI/debug)
		render().set_depth_buffer_state({ DM_ENABLE_AND_WRITE });
		render().set_cullface_state({ CF_BACK });
	}

	//////////////////////////////////////////////////////////////////////
	// Debug dump (SQUARE_DEFERRED_DUMP=1): save each buffer as a TGA in /tmp
	//////////////////////////////////////////////////////////////////////
	static void dump_write_tga(const char* path, int w, int h, const std::vector<unsigned char>& bgra)
	{
		FILE* f = std::fopen(path, "wb");
		if (!f) return;
		unsigned char header[18] = { 0 };
		header[2]  = 2; //uncompressed true-color
		header[12] = w & 0xFF; header[13] = (w >> 8) & 0xFF;
		header[14] = h & 0xFF; header[15] = (h >> 8) & 0xFF;
		header[16] = 32;   //bpp
		header[17] = 0x20; //origin: top-left
		std::fwrite(header, sizeof(header), 1, f);
		std::fwrite(bgra.data(), bgra.size(), 1, f);
		std::fclose(f);
	}

	static float dump_half_to_float(unsigned short h)
	{
		const unsigned int sign = (h >> 15) & 0x1;
		const unsigned int exp  = (h >> 10) & 0x1F;
		const unsigned int man  = h & 0x3FF;
		float value;
		if (exp == 0)       value = man * (1.0f / 16777216.0f); //subnormal: man * 2^-24
		else if (exp == 31) value = man ? 0.0f : (sign ? -65504.0f : 65504.0f);
		else                value = std::ldexp(float(man | 0x400), int(exp) - 25);
		return sign ? -value : value;
	}

	void DrawerPassDeferred::dump_buffers()
	{
		//convert a float/half RGBA buffer to a clamped BGRA8 image and save it
		auto save = [&](const char* name, Render::Texture* texture, bool is_half)
		{
			if (!texture) return;
			//NOTE: the (face, level) overload is used to avoid ambiguity with
			//get_texture(Texture*, int level); face 0 is the 2D surface itself
			auto raw = render().get_texture(texture, 0, 0);
			const size_t pixel_size = is_half ? 8 : 16;
			const size_t count = raw.size() / pixel_size;
			if (!count) { context().logger()->warning(std::string("Deferred dump: no data for ") + name); return; }
			std::vector<unsigned char> bgra(count * 4);
			for (size_t i = 0; i < count; ++i)
			{
				float rgba[4];
				if (is_half)
				{
					auto* h = reinterpret_cast<const unsigned short*>(raw.data()) + i * 4;
					for (int c = 0; c < 4; ++c) rgba[c] = dump_half_to_float(h[c]);
				}
				else
				{
					auto* fp = reinterpret_cast<const float*>(raw.data()) + i * 4;
					for (int c = 0; c < 4; ++c) rgba[c] = fp[c];
				}
				auto to8 = [](float v) -> unsigned char
				{ return (unsigned char)(std::fmin(std::fmax(v, 0.0f), 1.0f) * 255.0f + 0.5f); };
				bgra[i * 4 + 0] = to8(rgba[2]);
				bgra[i * 4 + 1] = to8(rgba[1]);
				bgra[i * 4 + 2] = to8(rgba[0]);
				bgra[i * 4 + 3] = 255;
			}
			std::string path = std::string("/tmp/square_deferred_") + name + ".tga";
			dump_write_tga(path.c_str(), m_size.x, m_size.y, bgra);
			context().logger()->info("Deferred dump: " + path);
		};
		save("position", m_gbuffer->texture(GB_POSITION), false);
		save("normal",   m_gbuffer->texture(GB_NORMAL),   false);
		save("albedo",   m_gbuffer->texture(GB_ALBEDO),   false);
		save("emissive", m_gbuffer->texture(GB_EMISSIVE), false);
		save("light",    m_light_texture,                 true);
	}
}
}
