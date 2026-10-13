//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Particles.h"
#include "MeshManager.h"
#include "SquareExtras.h"
#include <algorithm>
#include <vector>

namespace Particles
{
    namespace AuxParticles
    {
        using namespace Square;
        using namespace Square::Scene;

        //a vector of Blender (z up) in the axes of the scene (as the converter turns them)
        Vec3 turned(Vec3 vector, unsigned char mode)
        {
            if (mode & M_SWAP_ZY)
            {
                std::swap(vector.y, vector.z);
            }
            if (mode & M_TO_LHS)
            {
                vector.z = -vector.z;
            }
            return vector;
        }

        //a vector of the custom properties ("square_<key>"), if there is
        bool vector_of(const Data::JsonObject& extras, const std::string& key, Vec3& vector)
        {
            auto it = extras.find(SquareExtras::PREFIX + key);
            const bool found = it != extras.end();
            if (found)
            {
                vector = Vec3(float(SquareExtras::component(it->second, 0)), float(SquareExtras::component(it->second, 1)), float(SquareExtras::component(it->second, 2)));
            }
            return found;
        }
    }

    size_t build(Square::Context& context, const Square::Shared<Square::Scene::Actor>& root, const Nodes& nodes, unsigned char mode)
    {
        using namespace AuxParticles;
        //the flagged nodes with a mesh (the node of the mesh: it or one under it), their properties
        std::vector< std::pair< Shared<Actor>, const Data::JsonObject* > > found;
        root->visit([&](Shared<Actor> node) -> bool
        {
            auto flagged = nodes.find(node.get());
            if (flagged != nodes.end())
            {
                const Data::JsonObject* extras = &flagged->second;
                node->visit([&](Shared<Actor> part) -> bool
                {
                    if (part->contains<StaticMesh>() && part->component<StaticMesh>()->m_mesh)
                    {
                        found.push_back({ part, extras });
                    }
                    return true;
                });
            }
            return true;
        });
        size_t made = 0;
        for (const auto& [part, extras] : found)
        {
            auto mesh = part->component<StaticMesh>();
            auto emitter = part->component<ParticleEmitter>();
            //its settings (the custom properties), its box (the mesh, by the scale of the node)
            SquareExtras::apply(*emitter, *extras);
            ParticleEmitter::Settings settings = emitter->settings();
            const Vec3 scale = part->scale();
            const Vec3 half = mesh->local_bounding_box().get_extension();
            settings.m_box = Vec3(half.x * std::abs(scale.x), half.y * std::abs(scale.y), half.z * std::abs(scale.z));
            Vec3 vector;
            if (vector_of(*extras, "direction", vector)) settings.m_direction = turned(vector, mode);
            if (vector_of(*extras, "gravity", vector)) settings.m_gravity = turned(vector, mode);
            emitter->settings(settings);
            if (!mesh->m_materials.empty())
            {
                emitter->material(mesh->m_materials[0]);
            }
            else
            {
                context.logger()->warning("Particles " + part->name() + ": no material");
            }
            //the box is in its settings: the scale of the node not on it twice
            part->scale(Vec3(1.0f));
            part->remove(mesh);
            made += 1;
        }
        return made;
    }
}
