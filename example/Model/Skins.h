//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Square/Square.h>
#include "GLTFReader.h"
#include "Animations.h"

class MeshManager;

//////////////////////////////////////////////////////////////////////////////////////////
//The skins of a scene (glTF skins: Blender, a mesh with an armature): the node of a mesh with a
//skin becomes a Scene::SkinnedMesh (in place of its StaticMesh): its joints the actors of the
//joints of the skin (their paths from it: "../rig/hips"), the inverse of their bind matrices
//turned as the converter turns the scene, its reach (how far its vertices are from their
//strongest joint). The mesh and its joints move: not static. Before the instances and the levels
//of detail (a skinned mesh is never one of them).
namespace Skins
{
    //the skinned meshes of a scene made; how many. mode: the turn of the axes (Modes of MeshManager.h)
    size_t build
    (
          Square::Context& context
        , const Square::Data::GLTF::GLTF& gltf
        , const Animations::NodeActors& actors
        , const MeshManager& meshes
        , unsigned char mode
    );
}
