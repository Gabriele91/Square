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
//The instances of a scene: the children of a node "square_instances" (Blender: a custom property
//true, e.g. a chunk of props) that share a mesh become one node "instances_<n>" with a
//Scene::InstancedMesh (GPU instancing: a draw for all of them), their matrices in the space of
//that node; the children are gone (their meshes: on them, or on a node of the exporter under
//them).
namespace InstanceGroups
{
    //the nodes "square_instances" (their actors)
    using Nodes = std::unordered_set< const Square::Scene::Actor* >;

    //the instanced meshes of a scene made; how many (and the instances in them)
    size_t build(Square::Context& context, const Square::Shared<Square::Scene::Actor>& root, const Nodes& nodes, size_t& instances);
}
