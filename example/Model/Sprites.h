//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Square/Square.h>
#include <unordered_set>

//////////////////////////////////////////////////////////////////////////////////////////
//The sprites of a scene: a node "square_sprite" (Blender: a custom property true on a plane)
//becomes a Scene::Sprite (a quad turned toward the camera, or around the axis of its material):
//its size the two largest sides of its mesh (by the scale of the node), its material the one of
//the mesh (a sprite effect: its "square_effect" Sprite or SpriteAdditive); the mesh is gone.
namespace Sprites
{
    //the nodes "square_sprite" (their actors)
    using Nodes = std::unordered_set< const Square::Scene::Actor* >;

    //the sprites of a scene made; how many
    size_t build(Square::Context& context, const Square::Shared<Square::Scene::Actor>& root, const Nodes& nodes);
}
