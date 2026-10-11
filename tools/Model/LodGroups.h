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
//The levels of detail of a scene: the sibling nodes "<name>_lod0", "<name>_lod1"... moved under
//a new node "<name>" with a Scene::LodGroup (as Unity imports them). Their thresholds from the
//custom properties of one of them (Blender):
//- "square_lod_mode": "screen" (the default) or "distance";
//- "square_lod": the thresholds of its levels (screen: shares of the height of the screen,
//  going down; distance: distances, going up); none: the defaults of the mode.
namespace LodGroups
{
    //the extras of the nodes (by their actor)
    using Extras = std::unordered_map< const Square::Scene::Actor*, Square::Data::JsonObject >;

    //the groups of a scene made; how many
    size_t build(Square::Context& context, const Square::Shared<Square::Scene::Actor>& root, const Extras& extras);
}
