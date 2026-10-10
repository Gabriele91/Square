//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Square/Square.h>
#include <vector>
#include "GLTFReader.h"

//////////////////////////////////////////////////////////////////////////////////////////
//The animations of a scene (glTF animations: Blender, its actions) become the clips of a
//Scene::Animator at its root: a channel of a clip the position, the rotation or the scale of a node
//(its actor: by its path from the root), its keys turned as the converter turns the scene (the
//axes, the hand). The actors animated move: not static. The animator plays all of them in loop
//when it starts (autoplay). The weights of the morph targets are not taken.
namespace Animations
{
    //the actor of each node of the glTF (by its index: nullptr, not in the scene)
    using NodeActors = std::vector< Square::Shared<Square::Scene::Actor> >;

    //a transform of glTF (a node, a key) in the axes of the scene (mode: Modes of MeshManager.h)
    void turn(Square::Vec3& translation, Square::Quat& rotation, Square::Vec3& scale, unsigned char mode);

    //the path of an actor from another one above it ("" itself; false: not under it)
    bool path(const Square::Scene::Actor& root, const Square::Scene::Actor& actor, std::string& out);

    //the animator of a scene made (none: no animation); how many clips
    size_t build
    (
          Square::Context& context
        , const Square::Shared<Square::Scene::Actor>& root
        , const Square::Data::GLTF::GLTF& gltf
        , const NodeActors& actors
        , unsigned char mode
    );
}
