//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Skins.h"
#include "GLTFImport.h"
#include "MeshManager.h"
#include <algorithm>
#include <cstring>

namespace Skins
{
    namespace AuxSkins
    {
        using namespace Square;
        using namespace Square::Scene;
        using namespace Square::Data;

        //the turn of the axes as a matrix (a point of glTF in the scene: swap y z, then flip z)
        Mat4 axes(unsigned char mode)
        {
            Mat4 turn(1.0f);
            if (mode & M_SWAP_ZY)
            {
                turn = Mat4(1, 0, 0, 0,
                            0, 0, 1, 0,
                            0, 1, 0, 0,
                            0, 0, 0, 1) * turn;
            }
            if (mode & M_TO_LHS)
            {
                turn = Mat4(1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, -1, 0,
                            0, 0, 0, 1) * turn;
            }
            return turn;
        }

        //the path of an actor from another one ("../" up to the first actor above both)
        bool relative_path(const Actor& from, const Actor& to, std::string& out)
        {
            std::string up;
            Shared<Actor> parent;
            const Actor* at = &from;
            bool found = false;
            while (at && !found)
            {
                std::string down;
                found = Animations::path(*at, to, down);
                if (found)
                {
                    out = up + down;
                    if (out.size() && out.back() == '/')
                    {
                        out.pop_back();
                    }
                }
                else
                {
                    up += "../";
                    parent = at->parent().lock();
                    at = parent.get();
                }
            }
            return found;
        }

        //the inverse bind matrices of a skin, in the axes of the scene (none: identities)
        std::vector<Mat4> inverse_binds(const GLTF::GLTF& gltf, const GLTF::Skin& skin, unsigned char mode)
        {
            std::vector<Mat4> out(skin.joints.size(), Mat4(1.0f));
            if (skin.inverse_bind_matrices.has_value())
            {
                const std::vector<float> values = GLTF::Import::read_floats(gltf, *skin.inverse_bind_matrices, false);
                for (size_t i = 0; i < out.size() && (i + 1) * 16 <= values.size(); ++i)
                {
                    //(column major, as glm)
                    std::memcpy(&out[i][0][0], &values[i * 16], sizeof(float) * 16);
                }
            }
            const Mat4 turn = axes(mode);
            const Mat4 back = inverse(turn);
            for (Mat4& matrix : out)
            {
                matrix = turn * matrix * back;
            }
            return out;
        }

        //how far the vertices are from their strongest joint (where it is when bound)
        float reach(const std::vector<Vec4>& points, const std::vector<Mat4>& inverse_binds)
        {
            std::vector<Vec3> bound(inverse_binds.size());
            for (size_t i = 0; i < inverse_binds.size(); ++i)
            {
                bound[i] = Vec3(inverse(inverse_binds[i])[3]);
            }
            float out = 0.0f;
            for (const Vec4& point : points)
            {
                const size_t joint = size_t(point.w + 0.5f);
                if (joint < bound.size())
                {
                    out = std::max(out, length(Vec3(point) - bound[joint]));
                }
            }
            return out;
        }
    }

    size_t build
    (
          Square::Context& context
        , const Square::Data::GLTF::GLTF& gltf
        , const Animations::NodeActors& actors
        , const MeshManager& meshes
        , unsigned char mode
    )
    {
        using namespace AuxSkins;
        size_t made = 0;
        for (size_t node_id = 0; node_id < gltf.nodes.size() && node_id < actors.size(); ++node_id)
        {
            const GLTF::Node& node = gltf.nodes[node_id];
            const Shared<Actor>& actor = actors[node_id];
            const bool mesh_node = node.content.has_value() && node.content->m_type == GLTF::Node::NT_MESH;
            const bool with_skin = node.skin.has_value() && *node.skin < gltf.skins.size();
            if (actor && mesh_node && with_skin && actor->contains<StaticMesh>() && meshes.skinned(node.content->m_id))
            {
                const GLTF::Skin& skin = gltf.skins[*node.skin];
                auto static_mesh = actor->component<StaticMesh>();
                auto skinned = actor->component<SkinnedMesh>();
                skinned->mesh(static_mesh->m_mesh);
                skinned->materials(static_mesh->m_materials);
                actor->remove(static_mesh);
                //its joints: their paths from it
                std::vector<std::string> paths;
                for (size_t joint_id : skin.joints)
                {
                    std::string path;
                    const Shared<Actor> joint = joint_id < actors.size() ? actors[joint_id] : nullptr;
                    if (!joint || !relative_path(*actor, *joint, path))
                    {
                        context.logger()->warning("Skin " + skin.name + ": a joint not in the scene");
                    }
                    if (joint)
                    {
                        joint->set_static(false);
                    }
                    paths.push_back(path);
                }
                const std::vector<Mat4> binds = inverse_binds(gltf, skin, mode);
                skinned->joints(paths, binds);
                skinned->reach(reach(meshes.skin_points(node.content->m_id), binds));
                if (paths.size() > SkinnedMesh::joints_max)
                {
                    context.logger()->warning("Skin " + skin.name + ": " + std::to_string(paths.size()) + " joints, at most " + std::to_string(SkinnedMesh::joints_max));
                }
                actor->set_static(false);
                made += 1;
            }
        }
        return made;
    }
}
