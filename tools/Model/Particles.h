//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Square/Square.h>
#include <unordered_map>

//////////////////////////////////////////////////////////////////////////////////////////
//The emitters of particles of a scene: a node "square_particles" (Blender: a custom property true
//on a plane) becomes a Scene::ParticleEmitter: its box the box of its mesh (by the scale of the
//node), its material the one of the mesh (a sprite effect: its "square_effect" Sprite or
//SpriteAdditive), its settings its custom properties "square_<attribute>" (square_rate,
//square_life, square_speed, square_direction, square_gravity, square_size, square_color_start...:
//ParticleEmitter.h); the mesh is gone. The vectors (direction: of the node, gravity: of the world)
//are in the axes of Blender (z up), turned as the converter turns the scene.
namespace Particles
{
    //the nodes "square_particles" (their actors) and their custom properties
    using Nodes = std::unordered_map< const Square::Scene::Actor*, Square::Data::JsonObject >;

    //the emitters of a scene made; how many. mode: the turn of the axes (Modes of MeshManager.h)
    size_t build(Square::Context& context, const Square::Shared<Square::Scene::Actor>& root, const Nodes& nodes, unsigned char mode);
}
