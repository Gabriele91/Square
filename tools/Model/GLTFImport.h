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
    inline StructureType determine_structure(const PrimitiveAttributes& attributes,  const Accessors& accessors)
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
    inline std::string structure_type_to_string(StructureType type)
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
    inline Render::Mesh::Vertex2DList get_Position2D(const GLTF& gltf, const Primitive& primitive)
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
    inline Render::Mesh::Vertex2DUVList get_Position2DUV(const GLTF& gltf, const Primitive& primitive)
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
    inline Render::Mesh::Vertex3DList get_Position3D(const GLTF& gltf, const Primitive& primitive)
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
    inline Render::Mesh::Vertex3DUVList get_Position3DUV(const GLTF& gltf, const Primitive& primitive)
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
    inline Render::Mesh::Vertex3DNUVList get_Position3DNormalUV(const GLTF& gltf, const Primitive& primitive)
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
    // Tangent space of glTF: T along u, B toward the top of the image (the v of glTF grows toward
    // the bottom, the normal maps are OpenGL: green up), N. Per vertex, the sum of its triangles.
    inline void compute_tangents(const Render::Mesh::IndexList& indices, Render::Mesh::Vertex3DNTBUVList& vertexes)
    {
        for (auto& vertex : vertexes)
        {
            vertex.m_tangent = Vec3(0.0f);
            vertex.m_binomial = Vec3(0.0f);
        }
        auto triangle = [&](size_t i0, size_t i1, size_t i2)
        {
            auto& v0 = vertexes[i0]; auto& v1 = vertexes[i1]; auto& v2 = vertexes[i2];
            const Vec3 edge1 = v1.m_position - v0.m_position;
            const Vec3 edge2 = v2.m_position - v0.m_position;
            const Vec2 delta_uv1 = v1.m_uvmap - v0.m_uvmap;
            const Vec2 delta_uv2 = v2.m_uvmap - v0.m_uvmap;
            const float div = delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x;
            if (div == 0.0f) return;
            const Vec3 tangent = (edge1 * delta_uv2.y - edge2 * delta_uv1.y) / div;   // dP/du
            const Vec3 bitangent = (edge1 * delta_uv2.x - edge2 * delta_uv1.x) / div; // -dP/dv: up
            for (auto* vertex : { &v0, &v1, &v2 })
            {
                vertex->m_tangent += tangent;
                vertex->m_binomial += bitangent;
            }
        };
        if (indices.size())
            for (size_t i = 0; i + 2 < indices.size(); i += 3) triangle(indices[i], indices[i + 1], indices[i + 2]);
        else
            for (size_t i = 0; i + 2 < vertexes.size(); i += 3) triangle(i, i + 1, i + 2);
        // Orthogonal to the normal (Gram-Schmidt); B keeps its side (mirrored uv: B = -N x T)
        for (auto& vertex : vertexes)
        {
            const Vec3& n = vertex.m_normal;
            Vec3 t = vertex.m_tangent - n * dot(n, vertex.m_tangent);
            if (length(t) < 1e-8f) t = std::abs(n.x) < 0.9f ? cross(n, Constants::axis_x) : cross(n, Constants::axis_y);
            t = normalize(t);
            const float handedness = dot(cross(n, t), vertex.m_binomial) < 0.0f ? -1.0f : 1.0f;
            vertex.m_tangent = t;
            vertex.m_binomial = cross(n, t) * handedness;
        }
    }

    inline Render::Mesh::Vertex3DNTBUVList get_Position3DNormalTangetBinomialUV(const GLTF& gltf, const Primitive& primitive, const Render::Mesh::IndexList& indices = {})
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
            compute_tangents(indices, vertexes);
        }
        else if (!(layout & Render::Layout::LayoutFields::LF_BINOMIAL))
        {
            // B = (N x T) * w: the w of the glTF TANGENT (VEC4) is the side of B (mirrored uv: -1)
            struct TangentW { Vec4 m_tangent; };
            std::vector<TangentW> tangents;
            process_attribute(tangents, gltf, primitive.attributes.at("TANGENT"), &TangentW::m_tangent, get_vertexes<TangentW, Square::Vec4>);
            for (size_t i = 0; i < vertexes.size(); ++i)
            {
                const float w = i < tangents.size() && tangents[i].m_tangent.w < 0.0f ? -1.0f : 1.0f;
                vertexes[i].m_binomial = normalize(cross(vertexes[i].m_normal, vertexes[i].m_tangent)) * w;
            }
        }
        // Ok
        return vertexes;
    }
    // The values of an accessor as floats, num_components a element (its view's stride
    // followed); the integers as they are, or in [0, 1] / [-1, 1] when normalized (the weights
    // of the joints in bytes or shorts, the keys of a rotation in bytes or shorts)
    inline std::vector<float> read_floats(const GLTF& gltf, size_t accessor_id, bool normalized)
    {
        std::vector<float> values;
        if (accessor_id < gltf.accessors.size())
        {
            const Accessor& accessor = gltf.accessors[accessor_id];
            const View& view = gltf.views[accessor.buffer_view];
            const auto& buffer = gltf.buffers[view.buffer];
            size_t component_size = 4;
            switch (accessor.component_type)
            {
            case ComponentType::SIGNED_BYTE:
            case ComponentType::UNSIGNED_BYTE:  component_size = 1; break;
            case ComponentType::SIGNED_SHORT:
            case ComponentType::UNSIGNED_SHORT: component_size = 2; break;
            default:                            component_size = 4; break;
            }
            const size_t components = accessor.num_components;
            const size_t stride = view.stride ? view.stride : components * component_size;
            const unsigned char* data = buffer.data() + view.offset + accessor.byte_offset;
            values.resize(accessor.count * components);
            for (size_t i = 0; i < accessor.count; ++i)
            {
                const unsigned char* element = data + i * stride;
                for (size_t c = 0; c < components; ++c)
                {
                    const unsigned char* at = element + c * component_size;
                    float value = 0.0f;
                    switch (accessor.component_type)
                    {
                    case ComponentType::SIGNED_BYTE:
                    {
                        const int8_t raw = *reinterpret_cast<const int8_t*>(at);
                        value = normalized ? std::max(float(raw) / 127.0f, -1.0f) : float(raw);
                    }
                    break;
                    case ComponentType::UNSIGNED_BYTE:
                    {
                        const uint8_t raw = *at;
                        value = normalized ? float(raw) / 255.0f : float(raw);
                    }
                    break;
                    case ComponentType::SIGNED_SHORT:
                    {
                        int16_t raw = 0;
                        std::memcpy(&raw, at, sizeof(raw));
                        value = normalized ? std::max(float(raw) / 32767.0f, -1.0f) : float(raw);
                    }
                    break;
                    case ComponentType::UNSIGNED_SHORT:
                    {
                        uint16_t raw = 0;
                        std::memcpy(&raw, at, sizeof(raw));
                        value = normalized ? float(raw) / 65535.0f : float(raw);
                    }
                    break;
                    case ComponentType::UNSIGNED_INT:
                    {
                        uint32_t raw = 0;
                        std::memcpy(&raw, at, sizeof(raw));
                        value = float(raw);
                    }
                    break;
                    default:
                    {
                        std::memcpy(&value, at, sizeof(value));
                    }
                    break;
                    }
                    values[i * components + c] = value;
                }
            }
        }
        return values;
    }

    inline Render::Mesh::IndexList get_Index(const GLTF& gltf, const Primitive& primitive)
    {
        const auto& accessor = gltf.accessors[primitive.indices];
        const auto& buffer_view = gltf.views[accessor.buffer_view];
        const auto& buffer_data = gltf.buffers[buffer_view.buffer];
        const unsigned char* data_ptr = buffer_data.data() + buffer_view.offset + accessor.byte_offset;
        return get_indices(data_ptr, accessor.count, accessor.component_type);
    }
    inline Render::DrawType get_DrawType(const Primitive& primitive)
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