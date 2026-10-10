//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Occluders.h"
#include <vector>

namespace Occluders
{
    namespace AuxOccluders
    {
        using namespace Square;
        using namespace Square::Scene;

        //the triangles of an occluder at most before a warning (it is drawn on the CPU each frame)
        constexpr size_t s_many_triangles = 4096;

        //the flagged nodes under a root, with their flag (proxy or not)
        std::vector< std::pair< Shared<Actor>, bool > > flagged(const Shared<Actor>& root, const Nodes& nodes)
        {
            std::vector< std::pair< Shared<Actor>, bool > > found;
            root->visit([&](Shared<Actor> node) -> bool
            {
                auto it = nodes.find(node.get());
                if (it != nodes.end())
                {
                    found.push_back({ node, it->second });
                }
                return true;
            });
            return found;
        }

        //the meshes of a node and of the ones under it: an occluder each (proxy: the mesh gone);
        //how many made, their triangles
        size_t occlude(Context& context, const Shared<Actor>& node, bool proxy, size_t& triangles)
        {
            size_t made = 0;
            node->visit([&](Shared<Actor> part) -> bool
            {
                const bool has_mesh = part->contains<StaticMesh>() && part->component<StaticMesh>()->m_mesh;
                if (has_mesh)
                {
                    //its triangles in the space of its node (the one of its mesh)
                    std::vector<Vec3> points;
                    if (part->component<StaticMesh>()->m_mesh->local_triangles(points))
                    {
                        part->component<Occluder>()->triangles(points);
                        triangles += points.size() / 3;
                        made += 1;
                        if (points.size() / 3 > s_many_triangles)
                        {
                            context.logger()->warning("Occluder " + part->name() + ": " + std::to_string(points.size() / 3)
                                                    + " triangles (drawn on the CPU each frame: make it simpler)");
                        }
                    }
                    else
                    {
                        context.logger()->warning("Occluder " + part->name() + ": its mesh cannot be read");
                    }
                    if (proxy)
                    {
                        part->remove(part->component<StaticMesh>());
                    }
                }
                return true;
            });
            return made;
        }
    }

    size_t build(Square::Context& context, const Square::Shared<Square::Scene::Actor>& root, const Nodes& nodes, size_t& triangles)
    {
        using namespace AuxOccluders;
        size_t made = 0;
        triangles = 0;
        for (const auto& [node, proxy] : flagged(root, nodes))
        {
            made += occlude(context, node, proxy, triangles);
        }
        return made;
    }
}
