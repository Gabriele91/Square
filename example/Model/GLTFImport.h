#pragma once
#include <Square/Square.h>
#include "GLTFReader.h"

namespace Square
{
namespace Data 
{
namespace GLTF 
{
namespace Import
{
    // Enum to represent the possible structures
    enum class StructureType
    {
        Position2D,
        Position2DUV,
        Position3D,
        Position3DUV,
        Position3DNormalUV,
        Position3DNormalTangentBinormalUV,
        Unknown
    };

    // Function to determine the best fitting structure
    StructureType determine_structure(const PrimitiveAttributes& attributes,  const Accessors& accessors)
    {
        // Find POSITION attribute and get its accessor index
        auto position_iter = attributes.find("POSITION");
        if (position_iter == attributes.end())
        {
            return StructureType::Unknown;  // POSITION is required, if not found, return Unknown
        }
        // Determine if it's 2D or 3D based on the layout_type of the POSITION accessor
        const Square::Data::GLTF::Accessor& position_accessor = accessors[position_iter->second];
        bool is_2d = (position_accessor.layout_type == Square::Data::GLTF::LayoutType::VEC2);
        bool is_3d = (position_accessor.layout_type == Square::Data::GLTF::LayoutType::VEC3);

        bool has_normal = attributes.find("NORMAL") != attributes.end();
        bool has_tangent = attributes.find("TANGENT") != attributes.end();
        // Assuming we're checking only TEXCOORD_0
        bool has_texcoord = attributes.find("TEXCOORD_0") != attributes.end();  

        // Determine the best fitting structure based on the attributes and their layout types
        if (is_2d)
        {
            if (has_texcoord)
            {
                return StructureType::Position2DUV;
            }
            return StructureType::Position2D;
        }
        else if (is_3d)
        {
            if (has_normal)
            {
                if (has_tangent)
                {
                    return StructureType::Position3DNormalTangentBinormalUV;
                }
                if (has_texcoord)
                {
                    return StructureType::Position3DNormalUV;
                }
                return StructureType::Position3D;
            }
            if (has_texcoord)
            {
                return StructureType::Position3DUV;
            }
            return StructureType::Position3D;
        }

        return StructureType::Unknown;  // Return Unknown if the layout type doesn't match expected VEC2 or VEC3
    }
    
    // Debugging
    std::string structure_type_to_string(StructureType type)
    {
        switch (type)
        {
        case StructureType::Position2D: return "Position2D";
        case StructureType::Position2DUV: return "Position2DUV";
        case StructureType::Position3D: return "Position3D";
        case StructureType::Position3DUV: return "Position3DUV";
        case StructureType::Position3DNormalUV: return "Position3DNormalUV";
        case StructureType::Position3DNormalTangentBinormalUV: return "Position3DNormalTangentBinormalUV";
        default: return "Unknown";
        }
    }

    // Vector Traits 
    template <typename T> struct VecTraits 
    {
        static constexpr std::size_t length = 0;
    };
    template <> struct VecTraits<Vec2> 
    {
        static constexpr std::size_t length = 2;
    };
    template <> struct VecTraits<Vec3> 
    {
        static constexpr std::size_t length = 3;
    };
    template <> struct VecTraits<Vec4> 
    {
        static constexpr std::size_t length = 4;
    };

    inline Render::Mesh::IndexList get_indices(const unsigned char* data_ptr
                                                , size_t count 
                                                , ComponentType component_type)
    {
        Render::Mesh::IndexList indices(count);

        switch (component_type) 
        {
        case ComponentType::UNSIGNED_BYTE: 
        {
            const uint8_t* byte_ptr = reinterpret_cast<const uint8_t*>(data_ptr);
            for (size_t i = 0; i < count; ++i) 
            {
                indices[i] = static_cast<unsigned int>(byte_ptr[i]);
            }
            break;
        }
        case ComponentType::UNSIGNED_SHORT: 
        {
            const uint16_t* short_ptr = reinterpret_cast<const uint16_t*>(data_ptr);
            for (size_t i = 0; i < count; ++i) 
            {
                indices[i] = static_cast<unsigned int>(short_ptr[i]);
            }
            break;
        }
        case ComponentType::UNSIGNED_INT: 
        {
            const uint32_t* int_ptr = reinterpret_cast<const uint32_t*>(data_ptr);
            for (size_t i = 0; i < count; ++i) 
            {
                indices[i] = int_ptr[i];
            }
            break;
        }
        default:
            throw std::runtime_error("Unsupported component type for indices.");
        }

        return indices;
    }

    template<typename T, typename R>
    void get_vertexes(std::vector<T>& output_vertices
                        , const unsigned char* data_ptr
                        , size_t count
                        , LayoutType layout_type
                        , ComponentType component_type
                        , R T::*field
                        )
    {
        // Components
        size_t source_components = s_layout_type_number_of_components.at(layout_type);
        size_t target_components = VecTraits<R>::length;
        size_t stride = 0;

        // Adjust stride based on component type
        switch (component_type)
        {
        case ComponentType::FLOAT:
            stride = source_components * sizeof(float);
            break;
        case ComponentType::UNSIGNED_SHORT:
            stride = source_components * sizeof(unsigned short);
            break;
        case ComponentType::UNSIGNED_BYTE:
            stride = source_components * sizeof(unsigned char);
            break;
            // Add other cases as needed
        default:
            throw std::runtime_error("Unsupported component type");
        }

        for (size_t i = 0; i < count && i < output_vertices.size(); ++i)
        {
            const unsigned char* vertex_data = data_ptr + i * stride;
            switch (component_type)
            {
            case ComponentType::FLOAT:
                for (std::size_t j = 0; j < target_components && j < source_components; ++j)
                {
                    float value = *reinterpret_cast<const float*>(vertex_data + j * sizeof(float));
                    (output_vertices[i].*field)[j] = value;
                }
                break;
            case ComponentType::UNSIGNED_SHORT:
                for (std::size_t j = 0; j < target_components && j < source_components; ++j)
                {
                    unsigned char value = *(vertex_data + j);
                    (output_vertices[i].*field)[j] = static_cast<float>(value) / 255.0f;
                }
                break;
            case ComponentType::UNSIGNED_BYTE:
                for (std::size_t j = 0; j < target_components && j < source_components; ++j)
                {
                    unsigned short value = *reinterpret_cast<const unsigned short*>(vertex_data + j * sizeof(unsigned short));
                    (output_vertices[i].*field)[j] = static_cast<float>(value) / 65535.0f;
                }
                break;
            default:
                throw std::runtime_error("Unsupported component type");
            }
        }
    }

    template <typename T, typename R, typename F>
    void process_attribute( std::vector<T>& vertexes
                            , const GLTF& gltf
                            , size_t accessor_index
                            , F field_pointer
                            , void(*get_vertexes_func)(std::vector<T>&
                                                    , const unsigned char*
                                                    , size_t
                                                    , LayoutType
                                                    , ComponentType
                                                    , R T::*)
    ) {
        const auto& accessor = gltf.accessors[accessor_index];
        const auto& buffer_view = gltf.views[accessor.buffer_view];
        const auto& buffer_data = gltf.buffers[buffer_view.buffer];
        const unsigned char* data_ptr = buffer_data.data() + buffer_view.offset + accessor.byte_offset;

        if(vertexes.size() < accessor.count) 
            vertexes.resize(accessor.count);
        get_vertexes_func(vertexes, data_ptr, accessor.count, accessor.layout_type, accessor.component_type, field_pointer);
    }

    // Get geometry vectors
    Render::Mesh::Vertex2DList get_Position2D(const GLTF& gltf, const Primitive& primitive)
    {
        using Vertex = Render::Layout::Position2D;
        using Vec3Callback = void(*)(std::vector<Vertex>&, const unsigned char*, size_t, LayoutType, ComponentType, Vec3 Vertex::*);
        using Vec2Callback = void(*)(std::vector<Vertex>&, const unsigned char*, size_t, LayoutType, ComponentType, Vec2 Vertex::*);
        std::vector<Vertex> vertexes;

        // Map attribute names to corresponding vertex fields and types
        std::unordered_map<std::string, std::pair<Vec2Callback, Vec2 Vertex::*>> attribute_map_vec2
        {
            {"POSITION", {get_vertexes<Vertex, Square::Vec2>, &Vertex::m_position}},
        };

        // Process attributes
        for (const auto& [attribute_name, func_and_field] : attribute_map_vec2)
        {
            auto iter = primitive.attributes.find(attribute_name);
            if (iter != primitive.attributes.end())
            {
                process_attribute
                (
                    vertexes,
                    gltf,
                    iter->second,
                    attribute_map_vec2[attribute_name].second,
                    attribute_map_vec2[attribute_name].first
                );
            }
        }
        // Ok
        return vertexes;
    }
    Render::Mesh::Vertex2DUVList get_Position2DUV(const GLTF& gltf, const Primitive& primitive)
    {
        using Vertex = Render::Layout::Position2DUV;
        using Vec3Callback = void(*)(std::vector<Vertex>&, const unsigned char*, size_t, LayoutType, ComponentType, Vec3 Vertex::*);
        using Vec2Callback = void(*)(std::vector<Vertex>&, const unsigned char*, size_t, LayoutType, ComponentType, Vec2 Vertex::*);
        std::vector<Vertex> vertexes;

        // Map attribute names to corresponding vertex fields and types
        std::unordered_map<std::string, std::pair<Vec2Callback, Vec2 Vertex::*>> attribute_map_vec2
        {
            {"POSITION", {get_vertexes<Vertex, Square::Vec2>, &Vertex::m_position}},
            {"TEXCOORD_0", {get_vertexes<Vertex, Square::Vec2>, &Vertex::m_uvmap}}
        };

        // Process attributes
        for (const auto& [attribute_name, func_and_field] : attribute_map_vec2)
        {
            auto iter = primitive.attributes.find(attribute_name);
            if (iter != primitive.attributes.end())
            {
                process_attribute
                (
                    vertexes,
                    gltf,
                    iter->second,
                    attribute_map_vec2[attribute_name].second,
                    attribute_map_vec2[attribute_name].first
                );
            }
        }
        // Ok
        return vertexes;
    }
    Render::Mesh::Vertex3DList get_Position3D(const GLTF& gltf, const Primitive& primitive)
    {
        using Vertex = Render::Layout::Position3D;
        using Vec3Callback = void(*)(std::vector<Vertex>&, const unsigned char*, size_t, LayoutType, ComponentType, Vec3 Vertex::*);
        using Vec2Callback = void(*)(std::vector<Vertex>&, const unsigned char*, size_t, LayoutType, ComponentType, Vec2 Vertex::*);
        std::vector<Vertex> vertexes;

        // Map attribute names to corresponding vertex fields and types
        std::unordered_map<std::string, std::pair<Vec3Callback, Vec3 Vertex::*>> attribute_map_vec3
        {
            {"POSITION", {get_vertexes<Vertex, Square::Vec3>, &Vertex::m_position}},
        };

        // Process attributes
        for (const auto& [attribute_name, func_and_field] : attribute_map_vec3)
        {
            auto iter = primitive.attributes.find(attribute_name);
            if (iter != primitive.attributes.end())
            {
                process_attribute
                (
                    vertexes,
                    gltf,
                    iter->second,
                    attribute_map_vec3[attribute_name].second,
                    attribute_map_vec3[attribute_name].first
                );
            }
        }
        // Ok
        return vertexes;
    }
    Render::Mesh::Vertex3DUVList get_Position3DUV(const GLTF& gltf, const Primitive& primitive)
    {
        using Vertex = Render::Layout::Position3DUV;
        using Vec3Callback = void(*)(std::vector<Vertex>&, const unsigned char*, size_t, LayoutType, ComponentType, Vec3 Vertex::*);
        using Vec2Callback = void(*)(std::vector<Vertex>&, const unsigned char*, size_t, LayoutType, ComponentType, Vec2 Vertex::*);
        std::vector<Vertex> vertexes;

        // Map attribute names to corresponding vertex fields and types
        std::unordered_map<std::string, std::pair<Vec3Callback, Vec3 Vertex::*>> attribute_map_vec3
        {
            {"POSITION", {get_vertexes<Vertex, Square::Vec3>, &Vertex::m_position}},
        };

        std::unordered_map<std::string, std::pair<Vec2Callback, Vec2 Vertex::*>> attribute_map_vec2
        {
            {"TEXCOORD_0", {get_vertexes<Vertex, Square::Vec2>, &Vertex::m_uvmap}}
        };

        // Process attributes
        for (const auto& [attribute_name, func_and_field] : attribute_map_vec3)
        {
            auto iter = primitive.attributes.find(attribute_name);
            if (iter != primitive.attributes.end())
            {
                process_attribute
                (
                    vertexes,
                    gltf,
                    iter->second,
                    attribute_map_vec3[attribute_name].second,
                    attribute_map_vec3[attribute_name].first
                );
            }
        }

        for (const auto& [attribute_name, func_and_field] : attribute_map_vec2)
        {
            auto iter = primitive.attributes.find(attribute_name);
            if (iter != primitive.attributes.end())
            {
                process_attribute
                (
                    vertexes,
                    gltf,
                    iter->second,
                    attribute_map_vec2[attribute_name].second,
                    attribute_map_vec2[attribute_name].first
                );
            }
        }
        // Ok
        return vertexes;
    }
    Render::Mesh::Vertex3DNUVList get_Position3DNormalUV(const GLTF& gltf, const Primitive& primitive)
    {
        using Vertex = Render::Layout::Position3DNormalUV;
        using Vec3Callback = void(*)(std::vector<Vertex>&, const unsigned char*, size_t, LayoutType, ComponentType, Vec3 Vertex::*);
        using Vec2Callback = void(*)(std::vector<Vertex>&, const unsigned char*, size_t, LayoutType, ComponentType, Vec2 Vertex::*);
        std::vector<Vertex> vertexes;

        // Map attribute names to corresponding vertex fields and types
        std::unordered_map<std::string, std::pair<Vec3Callback, Vec3 Vertex::*>> attribute_map_vec3
        {
            {"POSITION", {get_vertexes<Vertex, Square::Vec3>, &Vertex::m_position}},
            {"NORMAL",   {get_vertexes<Vertex, Square::Vec3>, &Vertex::m_normal}},
        };

        std::unordered_map<std::string, std::pair<Vec2Callback, Vec2 Vertex::*>> attribute_map_vec2
        {
            {"TEXCOORD_0", {get_vertexes<Vertex, Square::Vec2>, &Vertex::m_uvmap}}
        };

        // Process attributes
        for (const auto& [attribute_name, func_and_field] : attribute_map_vec3)
        {
            auto iter = primitive.attributes.find(attribute_name);
            if (iter != primitive.attributes.end())
            {
                process_attribute
                (
                    vertexes,
                    gltf,
                    iter->second,
                    attribute_map_vec3[attribute_name].second,
                    attribute_map_vec3[attribute_name].first
                );
            }
        }

        for (const auto& [attribute_name, func_and_field] : attribute_map_vec2)
        {
            auto iter = primitive.attributes.find(attribute_name);
            if (iter != primitive.attributes.end())
            {
                process_attribute
                (
                    vertexes,
                    gltf,
                    iter->second,
                    attribute_map_vec2[attribute_name].second,
                    attribute_map_vec2[attribute_name].first
                );
            }
        }
        // Ok
        return vertexes;
    }
    Render::Mesh::Vertex3DNTBUVList get_Position3DNormalTangetBinomialUV(const GLTF& gltf, const Primitive& primitive, const Render::Mesh::IndexList& indices = {})
    {
        using Vertex = Render::Layout::Position3DNormalTangetBinomialUV;
        using Vec3Callback = void(*)(std::vector<Vertex>&, const unsigned char*, size_t, LayoutType, ComponentType, Vec3 Vertex::*);
        using Vec2Callback = void(*)(std::vector<Vertex>&, const unsigned char*, size_t, LayoutType, ComponentType, Vec2 Vertex::*);
        std::vector<Vertex> vertexes;

        // Map attribute names to corresponding vertex fields and types
        std::unordered_map<std::string, std::tuple<Vec3Callback, Vec3 Vertex::*, Render::Layout::LayoutFields>> attribute_map_vec3
        {
                {"POSITION", {get_vertexes<Vertex, Square::Vec3>, &Vertex::m_position, Render::Layout::LayoutFields::LF_POSITION_3D} }
            , {"NORMAL",   {get_vertexes<Vertex, Square::Vec3>, &Vertex::m_normal, Render::Layout::LayoutFields::LF_NORMAL}}
            , {"TANGENT",  {get_vertexes<Vertex, Square::Vec3>, &Vertex::m_tangent, Render::Layout::LayoutFields::LF_TANGENT}}
            , {"BINOMIAL", {get_vertexes<Vertex, Square::Vec3>, &Vertex::m_binomial, Render::Layout::LayoutFields::LF_BINOMIAL}}
        };

        std::unordered_map<std::string, std::pair<Vec2Callback, Vec2 Vertex::*>> attribute_map_vec2
        {
            {"TEXCOORD_0", {get_vertexes<Vertex, Square::Vec2>, &Vertex::m_uvmap}}
        };
        // Model layout
        unsigned char layout = 0;
        // Process attributes
        for (const auto& [attribute_name, func_and_field] : attribute_map_vec3)
        {
            auto iter = primitive.attributes.find(attribute_name);
            if (iter != primitive.attributes.end())
            {
                layout |= std::get<2>(attribute_map_vec3[attribute_name]);
                process_attribute
                (
                    vertexes,
                    gltf,
                    iter->second,
                    std::get<1>(attribute_map_vec3[attribute_name]),
                    std::get<0>(attribute_map_vec3[attribute_name])
                );
            }
        }

        for (const auto& [attribute_name, func_and_field] : attribute_map_vec2)
        {
            auto iter = primitive.attributes.find(attribute_name);
            if (iter != primitive.attributes.end())
            {
                process_attribute
                (
                    vertexes,
                    gltf,
                    iter->second,
                    attribute_map_vec2[attribute_name].second,
                    attribute_map_vec2[attribute_name].first
                );
            }
        }

        // Compute TNB
        if (!(layout & Render::Layout::LayoutFields::LF_NORMAL))
        {
            if(indices.size())
                Square::tangent_model_slow(indices, vertexes, true);
            else
                Square::tangent_model_slow(vertexes, true);
        }
        else if (!(layout & Render::Layout::LayoutFields::LF_TANGENT))
        {
            if (indices.size())
                Square::tangent_model_fast(indices, vertexes);
            else
                Square::tangent_model_fast(vertexes);
        }
        else if (!(layout & Render::Layout::LayoutFields::LF_BINOMIAL))
        {
            if (indices.size())
            {
                for (auto index : indices)
                {
                    vertexes[index].m_binomial = normalize(cross(vertexes[index].m_normal, vertexes[index].m_tangent));
                }
            }
            else
            {
                for (auto& vertex : vertexes)
                {
                    vertex.m_binomial = normalize(cross(vertex.m_normal, vertex.m_tangent));
                }
            }
        }
        // Ok
        return vertexes;
    }
    Render::Mesh::IndexList get_Index(const GLTF& gltf, const Primitive& primitive)
    {
        const auto& accessor = gltf.accessors[primitive.indices];
        const auto& buffer_view = gltf.views[accessor.buffer_view];
        const auto& buffer_data = gltf.buffers[buffer_view.buffer];
        const unsigned char* data_ptr = buffer_data.data() + buffer_view.offset + accessor.byte_offset;
        return get_indices(data_ptr, accessor.count, accessor.component_type);
    }
    Render::DrawType get_DrawType(const Primitive& primitive)
    {
        switch (primitive.mode)
        {
        case PrimitiveMode::POINTS: return Render::DrawType::DRAW_POINTS;
        case PrimitiveMode::LINES: return Render::DrawType::DRAW_LINES;
        case PrimitiveMode::LINE_LOOP: return Render::DrawType::DRAW_LINE_LOOP;
        case PrimitiveMode::TRIANGLES: return Render::DrawType::DRAW_TRIANGLES;
        case PrimitiveMode::TRIANGLE_STRIP: return Render::DrawType::DRAW_TRIANGLE_STRIP;
        default: return static_cast<Render::DrawType>(Render::DrawType::DRAW_INVALID);
        }
    }

    template<typename T>
    void visit_node(const GLTF& gltf, const Node& parent, const Node& node, T& in_value, const std::function<T(const Node* const parent, const Node& node, T&)>& callback)
    {
        // call on this node the callback
        T value = std::move(callback(&parent, node, in_value));
        // call for each child
        for (auto& node_id : node.children)
        {
            visit_node<T>(gltf, node, gltf.nodes[node_id], value, callback);
        }
    }
    template<typename T>
    void visit_node(const GLTF& gltf, const Node& node, T& in_value, const std::function<T(const Node* const parent, const Node& node, T&)>& callback)
    {
        // call on this node the callback
        T value = std::move(callback(nullptr, node, in_value));
        // call for each child
        for (auto& node_id : node.children)
        {
            visit_node<T>(gltf, node, gltf.nodes[node_id], value, callback);
        }
    }
    
    template<typename T>
    void visit_scene(const GLTF& gltf, const Scene& scene, T& value, const std::function<T(const Node* const parent, const Node& node, T&)>& callback)
    {
        // Visit each scene node
        for (auto& node_id : scene.nodes)
        {
            visit_node<T>(gltf, gltf.nodes[node_id], value, callback);
        }
    }
    template<typename T>
    void visit_default_scene(const GLTF& gltf, T& value ,const std::function<T(const Node* const parent, const Node& node, T&)>& callback)
    {
        if (gltf.scenes.empty())
            return;
        // Get default scene
        visit_scene<T>(gltf, gltf.scenes[gltf.default_scene], value, callback);
    }

} // namespace Import
} // namespace GLTF
} // namespace Data
} // namespace Square