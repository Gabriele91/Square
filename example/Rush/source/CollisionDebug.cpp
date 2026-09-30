//
//  CollisionDebug.cpp
//  Rush
//
#include <CollisionDebug.h>
#include <Collision.h>
#include <algorithm>

using namespace Square;

CollisionDebug::CollisionDebug(Context& context, CollisionWorld& world)
: PostEffect(context, Render::PES_COLOR)
, m_world(&world)
{
}

bool CollisionDebug::build()
{
	if (!m_copy)      m_copy = load_shader("PostCopy");
	if (!m_effect)    m_effect = context().resource<Resource::Effect>("Debug");
	if (!m_transform) m_transform = Render::stream_constant_buffer<Render::UniformBufferTransform>(&render());
	if (!m_sphere)    m_sphere = Render::BasicMesh::build_sphere(context(), 10, 16);
	return m_copy && m_effect && m_transform && m_sphere;
}

void CollisionDebug::on_release()
{
	m_transform.reset();
	m_sphere.reset();
	m_meshes.clear();
}

Render::Mesh* CollisionDebug::mesh_of(const Shared<MeshCollider>& collider)
{
	//gone colliders
	m_meshes.erase(std::remove_if(m_meshes.begin(), m_meshes.end(), [](const MeshShape& shape) { return shape.m_collider.expired(); }), m_meshes.end());
	for (const MeshShape& shape : m_meshes)
	{
		if (shape.m_collider.lock() == collider) return shape.m_mesh.get();
	}
	//its triangles in world space, three vertices each
	std::vector<Vec3> triangles;
	collider->mesh().triangles(triangles);
	MeshShape shape;
	shape.m_collider = collider;
	if (!triangles.empty())
	{
		Render::Mesh::Vertex3DList vertices(triangles.size());
		for (size_t i = 0; i < triangles.size(); ++i) vertices[i].m_position = triangles[i];
		shape.m_mesh = MakeShared<Render::Mesh>(context());
		if (!shape.m_mesh->build(vertices)) shape.m_mesh.reset();
	}
	m_meshes.push_back(shape);
	return m_meshes.back().m_mesh.get();
}

void CollisionDebug::draw_shape(Render::PostEffectFrame& frame, Render::Mesh& mesh, const Mat4& model, const Vec4& color)
{
	auto* wireframe  = m_effect->technique("wireframe");
	auto* line_color = m_effect->parameter("line_color");
	if (!wireframe || !line_color) return;
	Render::UniformBufferTransform transform;
	transform.m_model     = model;
	transform.m_inv_model = inverse(model);
	transform.m_rotation  = Mat4(1.0f);
	transform.m_position  = Vec3(model[3]);
	transform.m_scale     = Vec3(1.0f);
	frame.m_render->update_steam_CB(m_transform.get(), (const unsigned char*)&transform, sizeof(transform));
	line_color->set(color);
	const Render::EffectPassInputs inputs{ frame.m_camera_buffer, m_transform.get(), Vec4(1.0f) };
	for (auto& pass : *wireframe)
	{
		pass.bind(*frame.m_render, inputs, m_effect->parameters());
		mesh.draw(*frame.m_render);
		pass.unbind();
	}
}

void CollisionDebug::draw(Render::PostEffectFrame& frame)
{
	if (!build() || !frame.m_destination) return;
	//the frame as it is
	draw_fullscreen(frame, frame.m_destination, m_copy.get(), Render::BlendState(), [&](Resource::Shader& shader)
	{
		if (auto source = shader.uniform("g_source")) source->set(frame.m_source);
	});
	if (!m_world) return;
	//the shapes over it
	auto& render = *frame.m_render;
	render.enable_render_target(frame.m_destination);
	render.set_viewport_state({ Vec4(0.0f, 0.0f, float(frame.m_size.x), float(frame.m_size.y)) });
	if (m_settings.meshes)
	{
		for (const Weak<MeshCollider>& weak_mesh : m_world->meshes())
		{
			if (auto collider = weak_mesh.lock())
			if (Render::Mesh* mesh = mesh_of(collider))
			{
				draw_shape(frame, *mesh, Mat4(1.0f), m_settings.mesh_color);
			}
		}
	}
	for (const Weak<SphereCollider>& weak_sphere : m_world->spheres())
	{
		auto sphere = weak_sphere.lock();
		if (!sphere) continue;
		if (m_settings.spheres)
		{
			const Mat4 model = glm::scale(glm::translate(Mat4(1.0f), sphere->center()), Vec3(sphere->radius(), sphere->radius_y(), sphere->radius()));
			draw_shape(frame, *m_sphere, model, sphere->collisions().empty() ? m_settings.sphere_color : m_settings.hit_color);
		}
		if (m_settings.contacts)
		{
			for (const CollisionReport& report : sphere->collisions())
			{
				const Mat4 model = glm::scale(glm::translate(Mat4(1.0f), report.m_point), Vec3(m_settings.contact_radius));
				draw_shape(frame, *m_sphere, model, m_settings.contact_color);
			}
		}
	}
	render.disable_render_target(frame.m_destination);
	render.set_blend_state({});
	render.set_depth_buffer_state({ Render::DM_ENABLE_AND_WRITE });
}
