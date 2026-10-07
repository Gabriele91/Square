//
//  PostEffect.cpp
//  Square
//
//  See PostEffect.h for the high level description.
//
#include "Square/Core/Context.h"
#include "Square/System/RenderSystem.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Mesh.h"
#include "Square/Render/PostEffect.h"
#include "Square/Render/Profiler.h"
#include "Square/Resource/Shader.h"

namespace Square
{
namespace Render
{
	//a color texture (linear, clamp, one level: it is drawn and read at level 0) and a target on it
	static bool build_color_target(Render::Context& render, const IVec2& size, TextureFormat format, Texture*& texture, Target*& target, bool linear = true)
	{
		texture = render.create_texture(
			{ format, (unsigned int)size.x, (unsigned int)size.y, nullptr, TT_RGBA, TTF_FLOAT, false },
			{ linear ? TMIN_LINEAR : TMIN_NEAREST, linear ? TMAG_LINEAR : TMAG_NEAREST, TEDGE_CLAMP, TEDGE_CLAMP, TEDGE_CLAMP, false, 0, 1 }
		);
		if (!texture) return false;
		target = render.create_render_target({ Render::TargetField{ texture, RT_COLOR } });
		return target != nullptr;
	}

	static void release_color_target(Render::Context& render, Texture*& texture, Target*& target)
	{
		if (target)  render.delete_render_target(target);
		if (texture) render.delete_texture(texture);
		target = nullptr;
		texture = nullptr;
	}

	//////////////////////////////////////////////////////////////////////
	// PostEffect
	//////////////////////////////////////////////////////////////////////
	PostEffect::PostEffect(Square::Context& context, PostEffectStage stage)
	: BaseObject()
	, SharedObject_t(context.allocator())
	, m_context(context)
	, m_stage(stage)
	{
	}

	PostEffect::~PostEffect()
	{
	}

	Square::Context& PostEffect::context() { return m_context; }
	const Square::Context& PostEffect::context() const { return m_context; }
	Render::Context& PostEffect::render() { return *System::get<RenderSystem>(context())->render(); }

	Shared<Resource::Shader> PostEffect::load_shader(const std::string& name)
	{
		auto shader = context().resource<Resource::Shader>(name);
		if (!shader || !shader->base_shader())
		{
			context().logger()->warning("PostEffect: missing shader '" + name + "'");
			return nullptr;
		}
		return shader;
	}

	void PostEffect::draw_fullscreen
	(
		  PostEffectFrame& frame
		, Target* target
		, Resource::Shader* shader
		, const BlendState& blend
		, const std::function<void(Resource::Shader&)>& uniforms
	)
	{
		if (!shader || !target || !frame.m_quad) return;
		auto& render = *frame.m_render;
		SQUARE_RENDER_SCOPE(render, shader->profile_name().c_str());
		render.enable_render_target(target);
		render.set_viewport_state({ Vec4(0.0f, 0.0f, float(frame.m_size.x), float(frame.m_size.y)) });
		render.set_depth_buffer_state({ DM_DISABLE });
		render.set_blend_state(blend);
		render.set_cullface_state({ CF_BACK });
		shader->bind();
		if (frame.m_camera_buffer) render.bind_uniform_CB(frame.m_camera_buffer, shader->base_shader(), "Camera");
		if (uniforms) uniforms(*shader);
		frame.m_quad->draw(render);
		shader->unbind();
		render.disable_render_target(target);
		render.set_blend_state({});
		render.set_depth_buffer_state({ DM_ENABLE_AND_WRITE });
	}

	bool PostEffect::create_color_target(const IVec2& size, TextureFormat format, Texture*& texture, Target*& target, bool linear)
	{
		delete_color_target(texture, target);
		return build_color_target(render(), size, format, texture, target, linear);
	}

	void PostEffect::delete_color_target(Texture*& texture, Target*& target)
	{
		if (auto render_system = System::get<RenderSystem>(context()))
		if (auto render_driver = render_system->render())
		{
			release_color_target(*render_driver, texture, target);
		}
	}

	//////////////////////////////////////////////////////////////////////
	// PostEffectChain
	//////////////////////////////////////////////////////////////////////
	PostEffectChain::PostEffectChain(Square::Context& context)
	: m_context(context)
	{
	}

	PostEffectChain::~PostEffectChain()
	{
		release();
	}

	Render::Context& PostEffectChain::render() { return *System::get<RenderSystem>(m_context)->render(); }

	bool PostEffectChain::any(const std::vector< Shared<PostEffect> >& effects, PostEffectStage stage)
	{
		for (auto& effect : effects)
		{
			if (effect && effect->enabled() && effect->stage() == stage) return true;
		}
		return false;
	}

	bool PostEffectChain::any_velocity(const std::vector< Shared<PostEffect> >& effects)
	{
		for (auto& effect : effects)
		{
			if (effect && effect->enabled() && effect->needs_velocity()) return true;
		}
		return false;
	}

	Texture* PostEffectChain::debug_texture(const std::vector< Shared<PostEffect> >& effects)
	{
		for (auto& effect : effects)
		{
			if (!effect || !effect->enabled()) continue;
			if (Texture* texture = effect->debug_texture()) return texture;
		}
		return nullptr;
	}

	bool PostEffectChain::build(const IVec2& size)
	{
		if (m_targets[0] && m_targets[1] && m_size == size) return true;
		release();
		m_size = size;
		for (int i = 0; i != 2; ++i)
		{
			if (!build_color_target(render(), size, TF_RGBA16F, m_textures[i], m_targets[i])) return false;
		}
		return true;
	}

	void PostEffectChain::release()
	{
		if (auto render_system = System::get<RenderSystem>(m_context))
		if (auto render_driver = render_system->render())
		{
			for (int i = 0; i != 2; ++i) release_color_target(*render_driver, m_textures[i], m_targets[i]);
		}
		m_size = IVec2(0, 0);
	}

	void PostEffectChain::draw_gbuffer(const std::vector< Shared<PostEffect> >& effects, PostEffectFrame frame)
	{
		if (!frame.m_gbuffer) return;
		for (auto& effect : effects)
		{
			if (!effect || !effect->enabled() || effect->stage() != PES_GBUFFER) continue;
			SQUARE_RENDER_SCOPE(render(), effect->object_name().c_str());
			effect->draw(frame);
		}
	}

	Texture* PostEffectChain::draw_color(const std::vector< Shared<PostEffect> >& effects, PostEffectFrame frame, Texture* source)
	{
		if (!any(effects, PES_COLOR) || !build(frame.m_size)) return source;
		//ping-pong: each effect reads the result of the previous one
		Texture* current = source;
		int next = 0;
		for (auto& effect : effects)
		{
			if (!effect || !effect->enabled() || effect->stage() != PES_COLOR) continue;
			frame.m_source = current;
			frame.m_destination = m_targets[next];
			SQUARE_RENDER_SCOPE(render(), effect->object_name().c_str());
			effect->draw(frame);
			current = m_textures[next];
			next = 1 - next;
		}
		return current;
	}
}
}
