//
//  SkinnedMesh.h
//  Square
//
//  A mesh moved by joints (glTF skins: Blender, an armature): its vertices have 4 joints and their
//  weights (Layout::Position3DNormalTangetBinomialUVSkin), its joints are actors (their paths from
//  its actor: "../rig/hips/spine"; an Animator moves them), each one with the inverse of its bind
//  matrix. Every frame (after the updates) the matrix of each joint in the world (its world matrix
//  by its inverse bind), at most joints_max; its box the box of its joints grown by its reach (how
//  far its vertices are from their joints). Drawn with the skinned variant of the techniques of its
//  materials (Render::EV_SKINNED: Skin.hlsl, the joints in the constant buffer "Skin"); the
//  transform of its actor is not used (the joints place it). Saved with its scene: the converter
//  makes it from a node of glTF with a skin.
//
#pragma once
#include <string>
#include <vector>
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Scene/Component.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Resource/Mesh.h"
#include "Square/Resource/Material.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Renderable.h"

namespace Square
{
namespace Render
{
	class ConstBuffer;
}
}

namespace Square
{
namespace Scene
{
	class SQUARE_API SkinnedMesh : public Square::Scene::Component
	                             , public Square::Render::Renderable
	{
	public:
		SQUARE_OBJECT(SkinnedMesh)

		//the joints of a skin at most (Skin.hlsl: SKIN_JOINTS_MAX)
		static constexpr size_t joints_max = 128;

		SkinnedMesh(Square::Context& context);
		virtual ~SkinnedMesh();

		//its mesh (a skinned layout), its materials (a sub mesh each)
		void mesh(const Square::Shared<Square::Resource::Mesh>& mesh) { m_mesh = mesh; }
		const Square::Shared<Square::Resource::Mesh>& mesh() const { return m_mesh; }
		void materials(const std::vector< Square::Shared<Square::Resource::Material> >& materials) { m_materials = materials; }
		const std::vector< Square::Shared<Square::Resource::Material> >& materials() const { return m_materials; }

		//its joints: their paths from its actor, the inverse of their bind matrices (as many)
		void joints(const std::vector<std::string>& paths, const std::vector<Square::Mat4>& inverse_binds);
		const std::vector<std::string>& joint_paths() const { return m_joint_paths; }
		const std::vector<Square::Mat4>& inverse_binds() const { return m_inverse_binds; }
		//how far its vertices are from their joints (world units: its box)
		void reach(float reach) { m_reach = reach; }
		float reach() const { return m_reach; }
		//the matrices of its joints in the world (the last frame)
		const std::vector<Square::Mat4>& joint_matrices() const { return m_joint_matrices; }

		//its joints in the world, its box (after the updates: the animators moved them)
		virtual void on_late_update(double delta_time) override;

		//Renderable
		virtual size_t materials_count() const override;
		virtual Square::Weak<Square::Render::Material> material(size_t i = 0) const override;
		virtual void draw
		(
			  Square::Render::Context& render
			, size_t material_id
			, Square::Render::EffectPassInputs& input
			, Square::Render::EffectPass& pass
			, int draw_id = 0
		) override;
		virtual bool skinned() const override { return true; }
		virtual bool visible() const override;
		virtual bool support_culling() const override;
		virtual const Square::Geometry::OBoundingBox& bounding_box() override;
		virtual Square::Weak<Square::Render::Transform> transform() const override;

		//events
		virtual void on_attach(Square::Scene::Actor& entity) override;
		virtual void on_deattch() override;

		//regs
		static void object_registration(Square::Context& ctx);

		//serialize
		virtual void serialize(Square::Data::Archive& archive)  override;
		virtual void serialize_json(Square::Data::JsonValue& archive) override;
		virtual void deserialize(Square::Data::Archive& archive) override;
		virtual void deserialize_json(Square::Data::JsonValue& archive) override;

	private:

		//the actors of its joints (by their paths)
		void find_joints();
		//its joints in the world and its box, now
		void pose();

		Square::Shared< Square::Resource::Mesh >                     m_mesh;
		std::vector< Square::Shared< Square::Resource::Material > >  m_materials;
		std::vector< std::string >                                   m_joint_paths;
		std::vector< Square::Mat4 >                                  m_inverse_binds;
		std::vector< Square::Weak<Actor> >                           m_joints;
		std::vector< Square::Mat4 >                                  m_joint_matrices; //joints_max of them
		Square::Shared< Square::Render::ConstBuffer >                m_buffer;
		Square::Geometry::OBoundingBox                               m_obb_global;
		Square::Weak< Square::Render::Transform >                    m_transform;
		float                                                        m_reach{ 1.0f };
		bool                                                         m_joints_found{ false };
		bool                                                         m_uploaded{ false }; //its pose in m_buffer
	};
}
}
