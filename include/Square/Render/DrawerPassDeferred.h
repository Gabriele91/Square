//
//  DrawerPassDeferred.h
//  Square
//
//  Deferred rendering pass.
//  Mirrors the structure of DrawerPassForward but splits rendering in two:
//   1) geometry pass  : opaque renderables are drawn with their "deferred"
//                        technique, filling a G-Buffer (position/normal/albedo/
//                        emissive + depth);
//   2) light passes   : the lights in the scene queues are accumulated additively
//                        into a light buffer, reusing the shared PBR light math;
//   3) the post effects (see PostEffect.h): the G-Buffer ones after 1), the color ones on
//      the light buffer after 2);
//   4) the light buffer (or the result of the color post effects) is copied to the screen.
//
//  Both DrawerPassForward and DrawerPassDeferred are RPT_RENDER passes: register
//  one or the other on the Drawer, not both.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Render/Drawer.h"
#include "Square/Render/GBuffer.h"
#include "Square/Render/Mesh.h"
#include "Square/Render/PostEffect.h"
#include <unordered_map>

namespace Square
{
namespace Resource
{
	class Shader;
}
}

namespace Square
{
namespace Render
{
	class SQUARE_API DrawerPassDeferred : public DrawerPass
	{
	public:
		//G-Buffer target indices (kept in sync with the shaders' GBUFFER_* macros)
		enum GBufferTarget : size_t
		{
			GB_POSITION = 0,
			GB_NORMAL   = 1,
			GB_ALBEDO   = 2,
			GB_EMISSIVE = 3,
			GB_DEPTH    = 4,
			GB_COUNT    = 5
		};

		//pass
		DrawerPassDeferred(Square::Context& context);
		virtual ~DrawerPassDeferred();

		//draw
		virtual void draw
		(
		  Drawer&           drawer
		, int               num_of_pass
		, const Vec4&       clear_color
		, const Vec4&       ambient_color
		, const Camera&     camera
		, const Collection& collection
		, const PoolQueues& queues
		)
		override;

	protected:
		//context
		Square::Context& context();
		const Square::Context& context() const;
		//render
		Render::Context& render();
		const Render::Context& render() const;

		//(re)build the G-Buffer and the light accumulation target for a given size
		bool build_buffers(const IVec2& size);
		//geometry pass: fill the G-Buffer with the "deferred" technique
		void geometry_pass(const Vec4& clear_color, int num_of_pass, const Camera& camera, const PoolQueues& queues);
		//velocity pass: the renderables with their own motion blur (Renderable::motion_blur) on the
		//G-Buffer depth, their motion on the screen since the last frame; false: none drawn
		bool velocity_pass(const Camera& camera, const PoolQueues& queues);
		//light passes: accumulate all lights into the light buffer
		void light_pass(const Vec4& ambient_color, const Camera& camera, const PoolQueues& queues);
		//translucent pass: the translucent renderables, forward shaded ("translucent" technique)
		//into the light buffer, over the lit opaque scene and tested against its depth
		void translucent_pass(const Vec4& ambient_color, const Camera& camera, const PoolQueues& queues);
		//present pass: draw the frame (light buffer, or the result of the post effects) on the
		//current (screen) target
		void present_pass(const Camera& camera, Texture* frame);
		//a frame for the post effects of a camera
		PostEffectFrame post_effect_frame(const Camera& camera);
		//bind the four G-Buffer textures on a light shader
		void bind_gbuffer(Resource::Shader* shader);

		//CPU DATA
		Square::Context& m_context;

		//GPU DATA: buffers
		Shared<GBuffer>  m_gbuffer;
		Render::Texture* m_light_texture{ nullptr };
		Render::Target*  m_light_target{ nullptr };
		//the G-Buffer occlusion (GT3) alone, for the G-Buffer post effects
		Render::Target*  m_occlusion_target{ nullptr };
		//the velocity (RG16F: uv on the screen) with the G-Buffer depth; drawn this frame
		Render::Texture* m_velocity_texture{ nullptr };
		Render::Target*  m_velocity_target{ nullptr };
		bool             m_velocity_drawn{ false };
		//the sun of the frame (the first direction light with a shadow): for the post effects
		Vec3             m_sun_direction{ 0.0f, -1.0f, 0.0f };
		Vec3             m_sun_color{ 0.0f };
		Render::Texture* m_sun_shadow_map{ nullptr };
		//the last frame: the model of each renderable drawn in the velocity, the camera
		std::unordered_map<const Renderable*, Mat4> m_previous_models;
		std::unordered_map<const Renderable*, Mat4> m_current_models;
		Mat4             m_previous_view{ 1.0f };
		Mat4             m_previous_projection{ 1.0f };
		Vec3             m_previous_eye{ 0.0f };
		bool             m_previous_camera{ false };
		IVec2            m_size{ 0, 0 };
		//post effects: the color chain
		PostEffectChain  m_post_effects;

		//constant buffers
		Shared<Render::ConstBuffer> m_cb_camera;
		Shared<Render::ConstBuffer> m_cb_transform;
		Shared<Render::ConstBuffer> m_cb_direction_light;
		Shared<Render::ConstBuffer> m_cb_point_light;
		Shared<Render::ConstBuffer> m_cb_spot_light;
		Shared<Render::ConstBuffer> m_cb_light_volume;
		Shared<Render::ConstBuffer> m_cb_velocity;
		//shadow constant buffers
		Shared<Render::ConstBuffer> m_cb_direction_shadow_light;
		Shared<Render::ConstBuffer> m_cb_point_shadow_light;
		Shared<Render::ConstBuffer> m_cb_spot_shadow_light;

		//light shaders
		Shared<Resource::Shader> m_shader_ambient;
		Shared<Resource::Shader> m_shader_direction;
		Shared<Resource::Shader> m_shader_point;
		Shared<Resource::Shader> m_shader_spot;
		//light shaders with shadow mapping
		Shared<Resource::Shader> m_shader_direction_shadow;
		Shared<Resource::Shader> m_shader_point_shadow;
		Shared<Resource::Shader> m_shader_spot_shadow;
		Shared<Resource::Shader> m_shader_present;
		Shared<Resource::Shader> m_shader_velocity;

		//light volume meshes
		Shared<Mesh> m_quad;
		Shared<Mesh> m_sphere;
		Shared<Mesh> m_cone;
	};
}
}
