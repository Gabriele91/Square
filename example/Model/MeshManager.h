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
    std::string m_output;
    unsigned char m_mode{ M_NONE };

    std::unordered_map<std::string, std::string> m_mesh_name_files;
    std::vector< std::vector<size_t> > m_meshes_materials;
    std::vector<std::string> m_mesh_names;
    std::vector<Square::Geometry::OBoundingBox> m_mesh_obbs;
    std::vector< std::vector<Square::Vec4> > m_mesh_skin_points; //(skinned: each vertex, its strongest joint in w; else none)
    UniqueNames m_names;

public:
    MeshManager(Square::Context& context, const std::string& output, unsigned char mode);

    MeshManager(Square::Context& context, const std::string& output, unsigned char mode, const Square::Data::GLTF::GLTF& gltf);

    const std::unordered_map<std::string, std::string>& resource_map() const;

    std::optional< std::tuple<const std::string*, const Square::Geometry::OBoundingBox*, const std::vector<size_t>* > > at(size_t id) const;

    size_t add_mesh(const Square::Data::GLTF::Mesh& mesh, const Square::Data::GLTF::GLTF& gltf);

    //a mesh with joints (its primitives JOINTS_0, WEIGHTS_0: the skinned layout); its vertices (in
    //the axes of the scene), the index of their strongest joint in w (the reach of a SkinnedMesh)
    bool skinned(size_t id) const;
    const std::vector<Square::Vec4>& skin_points(size_t id) const;
};
