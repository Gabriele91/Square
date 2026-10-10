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
//The occluders of a scene (the software occlusion of the cameras: what is behind them is not
//drawn): the meshes of a node "square_occluder" (Blender: a custom property; it and the nodes
//under it) get a Scene::Occluder with their triangles. true: the mesh stays (drawn, solid) and
//hides; "proxy": only the occluder (the mesh gone: a simple shape inside what it stands for, not
//drawn, not solid). An occluder must be few triangles and inside the thing it stands for (what
//it hides must really be hidden).
namespace Occluders
{
    //the nodes "square_occluder" (their actors): proxy or not
    using Nodes = std::unordered_map< const Square::Scene::Actor*, bool >;

    //the occluders of a scene made; how many (and their triangles)
    size_t build(Square::Context& context, const Square::Shared<Square::Scene::Actor>& root, const Nodes& nodes, size_t& triangles);
}
