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
    Square::UnorderedMap<Square::String, Square::String> m_materials_resrouces;
    Square::Vector< Square::String > m_materials;
    Square::Context& m_context;
    Square::String m_output;
    UniqueNames m_names;

public:
    MaterialManager(Square::Context& context, const Square::String& output);

    MaterialManager(Square::Context& context, const Square::String& output, const TextureManager& texture_manager, const Square::Data::GLTF::GLTF& gltf);

    const Square::UnorderedMap<Square::String, Square::String>& resource_map() const;

    std::optional<Square::String> at(size_t id);

    size_t add_material(const Square::Data::GLTF::Material& material, const TextureManager& texture_manager);
};
