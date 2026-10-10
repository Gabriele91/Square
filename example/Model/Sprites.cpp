//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Sprites.h"
#include <algorithm>
#include <array>
#include <vector>

namespace Sprites
{
    namespace AuxSprites
    {
        using namespace Square;
        using namespace Square::Scene;

        //the size of a sprite of a plane: its two largest sides (its box by the scale of its node)
        Vec2 size_of(const Geometry::OBoundingBox& box, const Vec3& scale)
        {
            std::array<float, 3> sides
            {
                box.get_extension().x * 2.0f * std::abs(scale.x),
                box.get_extension().y * 2.0f * std::abs(scale.y),
                box.get_extension().z * 2.0f * std::abs(scale.z)
            };
            std::sort(sides.begin(), sides.end());
            return Vec2(sides[2], std::max(sides[1], 0.001f));
        }
    }

    size_t build(Square::Context& context, const Square::Shared<Square::Scene::Actor>& root, const Nodes& nodes)
    {
        using namespace AuxSprites;
        //the flagged nodes with a mesh (the node of the mesh: it or one under it)
        std::vector< Shared<Actor> > found;
        root->visit([&](Shared<Actor> node) -> bool
        {
            if (nodes.count(node.get()))
            {
                node->visit([&](Shared<Actor> part) -> bool
                {
                    if (part->contains<StaticMesh>() && part->component<StaticMesh>()->m_mesh)
                    {
                        found.push_back(part);
                    }
                    return true;
                });
            }
            return true;
        });
        size_t made = 0;
        for (const Shared<Actor>& part : found)
        {
            auto mesh = part->component<StaticMesh>();
            auto sprite = part->component<Sprite>();
            sprite->size(size_of(mesh->local_bounding_box(), part->scale()));
            if (!mesh->m_materials.empty())
            {
                sprite->material(mesh->m_materials[0]);
            }
            else
            {
                context.logger()->warning("Sprite " + part->name() + ": no material");
            }
            //its quad at the origin of the node: the scale of the node not on it twice
            part->scale(Vec3(1.0f));
            part->remove(mesh);
            made += 1;
        }
        return made;
    }
}
