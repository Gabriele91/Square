//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "InstanceGroups.h"
#include <map>
#include <vector>

namespace InstanceGroups
{
    namespace AuxInstanceGroups
    {
        using namespace Square;
        using namespace Square::Scene;

        //the copies of a mesh under a node: its static mesh (its materials, its box), where each
        //copy is in the space of the node, their nodes
        struct Group
        {
            Shared<StaticMesh>    m_first;
            std::vector<Mat4>     m_models;
            ActorList             m_nodes;
            bool                  m_static{ false };
        };

        //the node of a child with its mesh: the child, or a node of the exporter under it
        Shared<Actor> holder_of(const Shared<Actor>& child)
        {
            Shared<Actor> holder;
            child->visit([&holder](Shared<Actor> part) -> bool
            {
                bool more = true;
                if (part->contains<StaticMesh>() && part->component<StaticMesh>()->m_mesh)
                {
                    holder = part;
                    more = false;
                }
                return more;
            });
            return holder;
        }

        //the children of a node by their mesh
        std::map<const void*, Group> groups_of(const Shared<Actor>& node)
        {
            std::map<const void*, Group> groups;
            const Mat4 to_node = inverse(node->global_model_matrix());
            //(a copy: the children change)
            const ActorList children = node->childs();
            for (const auto& child : children)
            {
                if (auto holder = holder_of(child))
                {
                    auto mesh = holder->component<StaticMesh>();
                    Group& group = groups[mesh->m_mesh.get()];
                    if (!group.m_first)
                    {
                        group.m_first = mesh;
                        group.m_static = holder->is_static();
                    }
                    group.m_models.push_back(to_node * holder->global_model_matrix());
                    group.m_nodes.push_back(child);
                }
            }
            return groups;
        }

        //a group: its node of instances under the node, its copies gone
        void instance(Context& context, const Shared<Actor>& node, Group& group, size_t index)
        {
            auto instances = MakeShared<Actor>(context, "instances_" + std::to_string(index));
            instances->set_static(group.m_static);
            auto instanced = instances->component<InstancedMesh>();
            instanced->mesh(group.m_first->m_mesh, group.m_first->m_materials);
            instanced->mesh_box(group.m_first->local_bounding_box());
            instanced->instances(group.m_models);
            for (const auto& old : group.m_nodes)
            {
                node->remove(old);
            }
            node->add(instances);
        }
    }

    size_t build(Square::Context& context, const Square::Shared<Square::Scene::Actor>& root, const Nodes& nodes, size_t& instances)
    {
        using namespace Square;
        using namespace Square::Scene;
        //the nodes asked (found first: the tree changes)
        ActorList found;
        root->visit([&found, &nodes](Shared<Actor> node) -> bool
        {
            if (nodes.count(node.get()))
            {
                found.push_back(node);
            }
            return true;
        });
        size_t meshes = 0;
        instances = 0;
        for (const auto& node : found)
        {
            auto groups = AuxInstanceGroups::groups_of(node);
            for (auto& entry : groups)
            {
                AuxInstanceGroups::Group& group = entry.second;
                AuxInstanceGroups::instance(context, node, group, meshes);
                instances += group.m_models.size();
                ++meshes;
            }
        }
        return meshes;
    }
}
