//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Square/Square.h>
#include <optional>
#include "GLTFReader.h"
#include "TextureManager.h"
#include "UniqueNames.h"

//////////////////////////////////////////////////////////////////////////////////////////
//The materials of a glTF: a .mat per material, for its effect (see MaterialTemplate)
class MaterialManager
{
    std::unordered_map<std::string, std::string> m_materials_resrouces;
    std::vector< std::string > m_materials;
    Square::Context& m_context;
    std::string m_output;
    UniqueNames m_names;

public:
    MaterialManager(Square::Context& context, const std::string& output);

    MaterialManager(Square::Context& context, const std::string& output, const TextureManager& texture_manager, const Square::Data::GLTF::GLTF& gltf);

    const std::unordered_map<std::string, std::string>& resource_map() const;

    std::optional<std::string> at(size_t id);

    size_t add_material(const Square::Data::GLTF::Material& material, const TextureManager& texture_manager);
};
