//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "MaterialManager.h"
#include "SquareExtras.h"
#include <sstream>
#include <algorithm>

namespace
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //The .mat of a glTF material for an effect: the glTF values mapped to the parameters of the
    //effect family (Legacy/LegacyTranslucent, PBR/PBRTranslucent); the "square_<parameter>"
    //custom properties set/override parameters
    class MaterialTemplate
    {
    public:
        enum class Family
        {
            LEGACY, //Legacy, LegacyTranslucent: color, shininess, specular map
            PBR     //PBR, PBRTranslucent: metallic, roughness, emissive
        };

        static Family family(const std::string& effect)
        {
            return effect.compare(0, 6, "Legacy") == 0 ? Family::LEGACY : Family::PBR;
        }

        //effect: "square_effect", else by the glTF alphaMode (BLEND: translucent, drawn after the
        //opaque scene)
        static std::string effect(const Square::Data::GLTF::Material& material)
        {
            using AlphaMode = Square::Data::GLTF::Material::AlphaMode;
            return SquareExtras::string(material.extras, "effect")
                   .value_or(material.alpha_mode == AlphaMode::AM_BLEND ? "PBRTranslucent" : "PBR");
        }

        MaterialTemplate(const Square::Data::GLTF::Material& material, const TextureManager& texture_manager)
        : m_material(material)
        , m_texture_manager(texture_manager)
        {
        }

        //the text of the .mat
        std::string build(const std::string& effect) const
        {
            Parameters parameters = family(effect) == Family::LEGACY ? legacy() : pbr();
            //the faces: double sided (Blender: Backface Culling, Camera off) both, in the view and
            //in the shadow map; square_cull and square_shadow_cull override them
            if (m_material.double_sided)
            {
                parameters.emplace_back("cull", "cullface(off)");
                parameters.emplace_back("shadow_cull", "cullface(off)");
            }
            //square_<parameter>: set or override
            for (const auto& extra : m_material.extras)
            {
                std::string name = SquareExtras::name(extra.first);
                if (name.empty() || name == "effect") continue;
                std::string value = SquareExtras::material_value(extra.second);
                //square_shadow_cast false: none of its pixels in the shadow maps (its mask of the
                //shadow over any alpha: the engine leaves it out of them); true: as it is
                if (name == "shadow_cast")
                {
                    if (SquareExtras::number(extra.second) != 0.0) continue;
                    name = "mask_shadow";
                    value = "float(1.5)";
                }
                if (value.empty()) continue;
                auto it = std::find_if(parameters.begin(), parameters.end(), [&](const Parameter& parameter) { return parameter.first == name; });
                if (it != parameters.end()) it->second = value;
                else parameters.emplace_back(name, value);
            }
            std::ostringstream text;
            text << "effect \"" << effect << "\"\n{\n";
            for (const Parameter& parameter : parameters) text << "\t" << parameter.first << " " << parameter.second << "\n";
            text << "}";
            return text.str();
        }

    private:
        using Parameter = std::pair<std::string, std::string>;
        using Parameters = std::vector<Parameter>;
        using Material = Square::Data::GLTF::Material;

        const Material&       m_material;
        const TextureManager& m_texture_manager;

        //texture("name") of a texture of the material, the default if it has none
        std::string texture(const std::optional<Material::TextureInfo>& info, const std::string& default_name) const
        {
            const std::string name = info.has_value() ? m_texture_manager.at(info.value().index).value_or(default_name) : default_name;
            return "texture(\"" + name + "\")";
        }
        static std::string real(float value)
        {
            std::ostringstream text;
            text << "float(" << value << ")";
            return text.str();
        }
        static std::string vec3(const Square::Vec3& value)
        {
            std::ostringstream text;
            text << "Vec3(" << value.x << "," << value.y << "," << value.z << ")";
            return text.str();
        }
        static std::string vec4(const Square::Vec4& value)
        {
            std::ostringstream text;
            text << "Vec4(" << value.x << "," << value.y << "," << value.z << "," << value.w << ")";
            return text.str();
        }

        //the glTF values
        Material::PbrMetallicRoughness pbr_values() const
        {
            return m_material.pbr_metallic_roughness.value_or(Material::PbrMetallicRoughness());
        }
        //MASK: opaque with an alpha test at alphaCutoff; OPAQUE and BLEND: no test
        float mask() const
        {
            return m_material.alpha_mode == Material::AlphaMode::AM_MASK ? m_material.alpha_cutoff : -1.0f;
        }

        //Legacy/LegacyTranslucent (albedo, normal, specular, occlusion, emissive maps; color,
        //shininess, emissive; the emission is drawn by the forward/translucent passes)
        Parameters legacy() const
        {
            const Material::PbrMetallicRoughness values = pbr_values();
            const auto& specular_glossiness = m_material.extra_specular_glossiness;
            //specular map: the one of KHR_materials_pbrSpecularGlossiness, else none
            const std::optional<Material::TextureInfo> specular_map = specular_glossiness.has_value()
                                                                    ? specular_glossiness->specular_glossiness_texture
                                                                    : std::nullopt;
            //shininess from the roughness (Blinn-Phong exponent of alpha = roughness^2)
            const float alpha = std::max(values.roughness_factor * values.roughness_factor, 0.01f);
            const float shininess = std::clamp(2.0f / (alpha * alpha) - 2.0f, 1.0f, 256.0f);
            return
            {
                { "albedo_map",    texture(values.base_color_texture, "white") },
                { "normal_map",    texture(m_material.normal_texture, "normal_up") },
                { "specular_map",  texture(specular_map, "black") },
                { "occlusion_map", texture(m_material.occlusion_texture, "white") },
                { "emmisive_map",  texture(m_material.emissive_texture, "black") },
                { "color",         vec4(values.base_color_factor) },
                { "shininess",     real(shininess) },
                { "emmisive",      vec3(m_material.emissive_factor) },
                { "mask",          real(mask()) }
            };
        }

        //PBR/PBRTranslucent (albedo, metallic, roughness, emissive, occlusion, normal maps;
        //color, metallic, roughness, emissive); metallic and roughness share the glTF texture
        Parameters pbr() const
        {
            const Material::PbrMetallicRoughness values = pbr_values();
            return
            {
                { "albedo_map",    texture(values.base_color_texture, "white") },
                { "metallic_map",  texture(values.metallic_roughness_texture, "black") },
                { "roughness_map", texture(values.metallic_roughness_texture, "white") },
                { "emmisive_map",  texture(m_material.emissive_texture, "black") },
                { "occlusion_map", texture(m_material.occlusion_texture, "white") },
                { "normal_map",    texture(m_material.normal_texture, "normal_up") },
                { "color",         vec4(values.base_color_factor) },
                { "metallic",      real(values.metallic_factor) },
                { "roughness",     real(values.roughness_factor) },
                { "emmisive",      vec3(m_material.emissive_factor) },
                { "mask",          real(mask()) }
            };
        }
    };
}

MaterialManager::MaterialManager(Square::Context& context, const std::string& output)
: m_context(context)
, m_output(output)
{}

MaterialManager::MaterialManager(Square::Context& context, const std::string& output, const TextureManager& texture_manager, const Square::Data::GLTF::GLTF& gltf)
: m_context(context)
, m_output(output)
{
    for (auto& material : gltf.materials)
    {
        add_material(material, texture_manager);
    }
}

const std::unordered_map<std::string, std::string>& MaterialManager::resource_map() const
{
    return m_materials_resrouces;
}

std::optional<std::string> MaterialManager::at(size_t id)
{
    return id < m_materials.size()
           ? std::optional<std::string> { m_materials[id] }
           : std::optional<std::string>{};
}

size_t MaterialManager::add_material(const Square::Data::GLTF::Material& material, const TextureManager& texture_manager)
{
    // the parameters of its effect, from the glTF values and the square_* properties
    const std::string material_data = MaterialTemplate(material, texture_manager).build(MaterialTemplate::effect(material));
    const std::string material_name = m_names.make(material.name, "material" + std::to_string(m_materials.size()));
    const std::string material_path = Square::Filesystem::join(m_output, material_name + ".mat");
    Square::Filesystem::text_file_write_all(material_path, material_data);
    m_materials.push_back(material_name);
    m_materials_resrouces[material_name] = material_path;
    return m_materials.size();
}
