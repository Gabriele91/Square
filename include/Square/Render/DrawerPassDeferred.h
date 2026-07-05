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
//   3) the light buffer is copied to the screen.
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
		//light passes: accumulate all lights into the light buffer
		void light_pass(const Vec4& ambient_color, const Camera& camera, const PoolQueues& queues);
		//present pass: draw the light buffer on the current (screen) target
		void present_pass(const Camera& camera);
		//bind the four G-Buffer textures on a light shader
		void bind_gbuffer(Resource::Shader* shader);
		//debug: save the G-Buffer and light buffer as TGA files (SQUARE_DEFERRED_DUMP=1)
		void dump_buffers();

		//CPU DATA
		Square::Context& m_context;

		//GPU DATA: buffers
		Shared<GBuffer>  m_gbuffer;
		Render::Texture* m_light_texture{ nullptr };
		Render::Target*  m_light_target{ nullptr };
		IVec2            m_size{ 0, 0 };

		//constant buffers
		Shared<Render::ConstBuffer> m_cb_camera;
		Shared<Render::ConstBuffer> m_cb_transform;
		Shared<Render::ConstBuffer> m_cb_direction_light;
		Shared<Render::ConstBuffer> m_cb_point_light;
		Shared<Render::ConstBuffer> m_cb_spot_light;
		Shared<Render::ConstBuffer> m_cb_light_volume;
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

		//debug dump (SQUARE_DEFERRED_DUMP=1)
		bool m_dump_enabled{ false };
		int  m_frame_counter{ 0 };
		//debug view (SQUARE_DEFERRED_VIEW=position|normal|albedo|emissive|light):
		//the present pass shows the selected buffer instead of the final image
		int  m_debug_view{ -1 };
		//debug: draw point/spot lights with a fullscreen quad instead of
		//sphere/cone volumes (SQUARE_DEFERRED_NO_VOLUMES=1)
		bool m_no_volumes{ false };

		//light volume meshes
		Shared<Mesh> m_quad;
		Shared<Mesh> m_sphere;
		Shared<Mesh> m_cone;
	};
}
}
