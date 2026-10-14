//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Square/Square.h>
#include <optional>
#include <tuple>
#include "GLTFReader.h"
#include "UniqueNames.h"

//the transformations of the geometry
enum Modes : unsigned char
{
    M_NONE = 0,
    M_SWAP_ZY = 0b00000001,
    M_TO_LHS = 0b00000010,
};

//////////////////////////////////////////////////////////////////////////////////////////
//The meshes of a glTF: a .sm3dgz per mesh (its primitives are its sub meshes)
class MeshManager
{
    Square::Context& m_context;
    Square::String m_output;
    unsigned char m_mode{ M_NONE };

    Square::UnorderedMap<Square::String, Square::String> m_mesh_name_files;
    Square::Vector< Square::Vector<size_t> > m_meshes_materials;
    Square::Vector<Square::String> m_mesh_names;
    Square::Vector<Square::Geometry::OBoundingBox> m_mesh_obbs;
    Square::Vector< Square::Vector<Square::Vec4> > m_mesh_skin_points; //(skinned: each vertex, its strongest joint in w; else none)
    UniqueNames m_names;

public:
    MeshManager(Square::Context& context, const Square::String& output, unsigned char mode);

    MeshManager(Square::Context& context, const Square::String& output, unsigned char mode, const Square::Data::GLTF::GLTF& gltf);

    const Square::UnorderedMap<Square::String, Square::String>& resource_map() const;

    std::optional< std::tuple<const Square::String*, const Square::Geometry::OBoundingBox*, const Square::Vector<size_t>* > > at(size_t id) const;

    size_t add_mesh(const Square::Data::GLTF::Mesh& mesh, const Square::Data::GLTF::GLTF& gltf);

    //a mesh with joints (its primitives JOINTS_0, WEIGHTS_0: the skinned layout); its vertices (in
    //the axes of the scene), the index of their strongest joint in w (the reach of a SkinnedMesh)
    bool skinned(size_t id) const;
    const Square::Vector<Square::Vec4>& skin_points(size_t id) const;
};
