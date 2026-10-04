//
//  PostEffect.h
//  Square
//
//  Post effects: full-screen passes on the frame, after the scene is drawn.
//  A post effect declares its stage:
//   - PES_GBUFFER: deferred only, after the geometry pass and before the lights: it reads
//     and changes the G-Buffer (e.g. SSAO multiplies the occlusion, so only the ambient
//     light is darkened). Skipped in forward (no G-Buffer).
//   - PES_COLOR: forward and deferred, after the lighting (translucent included): it reads
//     the color of the frame (source, linear HDR) and writes the next one (destination).
//     The effects of this stage are a chain, in the order they were added.
//  To add a post effect: derive PostEffect, give the stage to the constructor, implement
//  draw(frame); draw_fullscreen() draws a shader over a target, create_color_target()
//  makes the textures it needs. Its GPU objects are made in draw (lazily) and released in
//  on_release() (the render system is closing, or the drawer is rebuilt). Then add it to the
//  world: RenderInstance::add_post_effect.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Object.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Driver/Render.h"
#include <functional>
#include <string>
#include <vector>

namespace Square
{
	class Context;
namespace Resource
{
	class Shader;
}
namespace Render
{
	class Camera;
	class GBuffer;
	class Mesh;

	//when a post effect is drawn
	enum PostEffectStage : unsigned char
	{
		PES_GBUFFER = 0, //deferred: after the geometry pass, on the G-Buffer
		PES_COLOR   = 1  //forward and deferred: after the lighting, source -> destination
	};

	//size of the passes of a post effect, from the frame (its result is full size)
	enum PostEffectResolution : int
	{
		PER_FULL    = 0, //the frame size
		PER_HALF    = 1, //half the frame size (a quarter of the pixels)
		PER_QUARTER = 2  //a quarter of the frame size (1/16 of the pixels)
	};
	//pixels of the frame a texel of a pass covers (on each side): 1, 2, 4
	inline int post_effect_scale(PostEffectResolution resolution)
	{
		return 1 << int(resolution);
	}
	//size of a pass at a resolution (at least 1x1)
	inline IVec2 post_effect_size(const IVec2& size, PostEffectResolution resolution)
	{
		return glm::max(size / post_effect_scale(resolution), IVec2(1, 1));
	}

	//what a post effect gets to draw a frame (a camera)
	struct PostEffectFrame
	{
		Render::Context*  m_render{ nullptr };
		const Camera*     m_camera{ nullptr };
		ConstBuffer*      m_camera_buffer{ nullptr }; //"Camera" cbuffer, already updated for the camera
		IVec2             m_size{ 0, 0 };             //pixels
		Vec4              m_viewport{ 0.0f };         //viewport of the camera (pixels)
		Mesh*             m_quad{ nullptr };          //full-screen quad (NDC)
		//deferred
		const GBuffer*    m_gbuffer{ nullptr };       //nullptr in forward
		Target*           m_occlusion{ nullptr };     //PES_GBUFFER: the G-Buffer occlusion (GT3); a (ZERO, SRC_COLOR) blend with rgb 1 multiplies it by alpha
		//PES_COLOR
		Texture*          m_source{ nullptr };        //color of the frame
		Target*           m_destination{ nullptr };   //the effect writes here
		bool              m_linear{ true };           //source in linear space (false: already sRGB encoded)
	};

	class SQUARE_API PostEffect : public BaseObject
	                            , public SharedObject<PostEffect>
	{
	public:
		SQUARE_OBJECT(PostEffect)

		PostEffect(Square::Context& context, PostEffectStage stage);
		virtual ~PostEffect();

		PostEffectStage stage() const { return m_stage; }

		//an effect off is not drawn
		void enabled(bool enabled) { m_enabled = enabled; }
		bool enabled() const { return m_enabled; }

		//draw the effect on a frame (a PES_COLOR effect must write the destination)
		virtual void draw(PostEffectFrame& frame) = 0;

		//release the GPU objects (called by the RenderInstance with its drawer)
		void release() { on_release(); }

		//debug view: a texture shown on the screen instead of the frame (the first effect that
		//has one), e.g. the occlusion of the SSAO; nullptr: none
		virtual Texture* debug_texture() const { return nullptr; }

	protected:
		//release the shaders and textures of the effect (they are made again in draw)
		virtual void on_release() {}

		Square::Context& context();
		const Square::Context& context() const;
		Render::Context& render();

		//a shader of the resources (nullptr and a warning if it is missing)
		Shared<Resource::Shader> load_shader(const std::string& name);
		//draw a shader on the whole target (full-screen quad, no depth, the blend given); binds the
		//"Camera" cbuffer; set the uniforms in the callback (the shader is bound)
		void draw_fullscreen
		(
			  PostEffectFrame& frame
			, Target* target
			, Resource::Shader* shader
			, const BlendState& blend
			, const std::function<void(Resource::Shader&)>& uniforms
		);
		//a color texture and its target of the frame size (linear or nearest, clamp); rebuilt when
		//the size changes (texture/target are released first)
		bool create_color_target(const IVec2& size, TextureFormat format, Texture*& texture, Target*& target, bool linear = true);
		void delete_color_target(Texture*& texture, Target*& target);

	private:
		Square::Context& m_context;
		PostEffectStage  m_stage;
		bool             m_enabled{ true };
	};

	//the PES_COLOR chain of a pass: ping-pong between two color textures of the frame size
	class SQUARE_API PostEffectChain
	{
	public:
		PostEffectChain(Square::Context& context);
		~PostEffectChain();

		//G-Buffer effects (deferred): after the geometry pass
		void draw_gbuffer(const std::vector< Shared<PostEffect> >& effects, PostEffectFrame frame);
		//color effects: from source; returns the texture with the result (source if no effect)
		Texture* draw_color(const std::vector< Shared<PostEffect> >& effects, PostEffectFrame frame, Texture* source);
		//an effect of the stage is on
		static bool any(const std::vector< Shared<PostEffect> >& effects, PostEffectStage stage);
		//the debug texture of the first effect on that has one (nullptr: none)
		static Texture* debug_texture(const std::vector< Shared<PostEffect> >& effects);

	private:
		bool build(const IVec2& size);
		void release();
		Render::Context& render();

		Square::Context& m_context;
		IVec2            m_size{ 0, 0 };
		Texture*         m_textures[2]{ nullptr, nullptr };
		Target*          m_targets[2]{ nullptr, nullptr };
	};

}
}
