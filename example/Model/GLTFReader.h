#pragma once
#include <Square/Square.h>
#include "GLBReader.h"

namespace Square
{
namespace Data 
{
namespace GLTF 
{
    static constexpr const char extension[5]{ '.','g','l','T','F' };

    // Enum for GLTF component types
    enum class ComponentType : unsigned int
    {
        SIGNED_BYTE = 5120,
        UNSIGNED_BYTE = 5121,
        SIGNED_SHORT = 5122,
        UNSIGNED_SHORT = 5123,
        UNSIGNED_INT = 5125,
        FLOAT = 5126,
    };

    // Enum for GLTF layout types
    enum class LayoutType : unsigned char
    {
        SCALAR = 1,
        VEC2 = 2,
        VEC3 = 3,
        VEC4 = 4,
        MAT2 = 5,
        MAT3 = 6,
        MAT4 = 7
    };

    // Enum for GLTF primitive modes
    enum class PrimitiveMode : unsigned char
    {
        POINTS = 0,
        LINES = 1,
        LINE_LOOP = 2,
        LINE_STRIP = 3,
        TRIANGLES = 4,
        TRIANGLE_STRIP = 5,
        TRIANGLE_FAN = 6
    };

    // Enum for the type of transformation
    enum TransformType : unsigned char
    {
        NONE = 0,             // No transformation
        TRANSLATION = 0b0001, // Translation (Vec3)
        ROTATION    = 0b0010, // Rotation (Quat)
        SCALE       = 0b0100, // Scale (Vec3)
        TRANSLATION_ROTATION       = TRANSLATION | ROTATION,
        TRANSLATION_SCALE          = TRANSLATION | SCALE,
        ROTATION_SCALE             = ROTATION    | SCALE,
        TRANSLATION_ROTATION_SCALE = TRANSLATION | ROTATION | SCALE,
        MATRIX      = 0b1000  // Transformation matrix (Mat4)
    };

    // Static map for the number of components based on type
    const static std::unordered_map<std::string, LayoutType> s_name_to_layout_type
    {
        {"SCALAR", LayoutType::SCALAR},
        {"VEC2",   LayoutType::VEC2},
        {"VEC3",   LayoutType::VEC3},
        {"VEC4",   LayoutType::VEC4},
        {"MAT2",   LayoutType::MAT2},
        {"MAT3",   LayoutType::MAT3},
        {"MAT4",   LayoutType::MAT4}
    };

    // Static map for the number of components based on LayoutType enum
    const static std::unordered_map<LayoutType, unsigned char> s_layout_type_number_of_components
    {
        {LayoutType::SCALAR, 1},
        {LayoutType::VEC2, 2},
        {LayoutType::VEC3, 3},
        {LayoutType::VEC4, 4},
        {LayoutType::MAT2, 4},
        {LayoutType::MAT3, 9},
        {LayoutType::MAT4, 16}
    };

    // Structure for GLTF buffer views
    struct View
    {
        size_t buffer;
        size_t offset;
        size_t length;
        size_t target;

        View(size_t buffer, size_t offset, size_t length, size_t target)
        : buffer(buffer)
        , offset(offset)
        , length(length)
        , target(target)
        {
        }
    };

    // Structure for GLTF accessors
    struct Accessor
    {
        size_t buffer_view;                // Index of the buffer view
        size_t byte_offset;                // Byte offset within the buffer view
        ComponentType component_type;      // Type of the components (e.g., FLOAT)
        size_t count;                      // Number of elements
        JsonArray max;                     // Max values for each component
        JsonArray min;                     // Min values for each component
        LayoutType layout_type;            // Layout type (e.g., VEC3)
        unsigned char num_components;      // Number of components based on type

        // Default move
        Accessor(Accessor&&) = default;

        // Constructor
        Accessor(size_t buffer_view,
                 size_t byte_offset,
                 ComponentType component_type,
                 size_t count,
                 Square::Data::JsonArray max,
                 Square::Data::JsonArray min,
                 LayoutType layout_type,
                 unsigned char num_components)
        : buffer_view(buffer_view)
        , byte_offset(byte_offset)
        , component_type(component_type)
        , count(count)
        , max(max)
        , min(min)
        , layout_type(layout_type)
        , num_components(num_components)
        {
        }
    };

    // Type for GLTF primitive attributes
    using PrimitiveAttributes = std::unordered_map<std::string, size_t>;

    // Structure for GLTF primitive
    struct Primitive
    {
        PrimitiveAttributes attributes;  // Attributes like POSITION, NORMAL, etc.
        size_t indices;                  // Index of the accessor containing the indices
        size_t material;                 // Index of the material
        PrimitiveMode mode;              // Primitive mode (e.g., TRIANGLES)

        // Default mov
        Primitive(Primitive&&) = default;

        Primitive(PrimitiveAttributes&& attributes,
                  size_t indices,
                  size_t material,
                  PrimitiveMode mode)
        : attributes(std::forward<PrimitiveAttributes>(attributes))
        , indices(indices)
        , material(material)
        , mode(mode)
        {
        }
    };

    // Alias for primitive vector
    using Primitives = std::vector<Primitive>;

    // Structure for GLTF mesh
    struct Mesh
    {
        std::string name;       // Name of the mesh
        Primitives  primitives; // List of primitives
        
        // Default move
        Mesh(Mesh&&) = default;

        Mesh(const std::string& name,
             Primitives&& primitives)
        : name(name)
        , primitives(std::forward<Primitives>(primitives))
        {
        }
    };

    // Struct for GLTF Prospective camera
    struct CameraProspective
    {
        double m_aspect_ratio{ 0.0 };
        double m_yfov{ 0.0 };
        double m_zfar{ 0.0 };
        double m_znear{ 0.0 };
    };

    // Struct for GLTF Orthographic camera
    struct CameraOrthographic
    {
        double m_xmag{ 0.0 };
        double m_ymag{ 0.0 };
        double m_zfar{ 0.0 };
        double m_znear{ 0.0 };
    };

    // Struct for GLTF Undefined camera
    struct CameraUndefined
    {
    };

    // Struct for GLTF camera
    using Camera = std::variant< CameraProspective, CameraOrthographic, CameraUndefined >;

    // Light
    struct Light
    {
        enum LightType
        {
            LT_DIRECTIONAL,
            LT_POINT,
            LT_SPOTLIGHT
        };

        struct SpotFields
        {
            double m_inner_cone_angle{ 0.0 };
            double m_outer_cone_angle{ Square::Constants::pi<double>() / 4.0 };
        };

        std::string m_name;
        Vec3        m_color{ 1.0, 1.0, 1.0 };
        double      m_intensity{ 1.0 };
        LightType   m_type;
        std::optional<double    > m_range;
        std::optional<SpotFields> m_spotfields;

        Light(const std::string& name,
              const Vec3&        color,
              double             intensity,
              LightType          type,
              std::optional<double    > range,
              std::optional<SpotFields> spotfields)
        : m_name(name)
        , m_color(color)
        , m_intensity(intensity)
        , m_range(range)
        , m_type(type)
        , m_spotfields(spotfields)
        {
        }
    };

    // Enum for GLTF texture filter types
    enum class TextureFilter : unsigned int
    {
        NEAREST = 9728,
        LINEAR = 9729,
        NEAREST_MIPMAP_NEAREST = 9984,
        LINEAR_MIPMAP_NEAREST = 9985,
        NEAREST_MIPMAP_LINEAR = 9986,
        LINEAR_MIPMAP_LINEAR = 9987
    };

    // Enum for GLTF texture wrap modes
    enum class TextureWrapMode : unsigned int
    {
        CLAMP_TO_EDGE = 33071,
        MIRRORED_REPEAT = 33648,
        REPEAT = 10497
    };

    // Structure for GLTF Image
    struct ImageBuffer
    {
        size_t buffer_view{ 0 };
        std::string mime_type;
        std::string name;

        ImageBuffer() = default;

        explicit ImageBuffer(size_t buffer_view, const std::string& mime_type, const std::string& name)
        : buffer_view(buffer_view)
        , mime_type(mime_type)
        , name(name)
        {
        }

        // Copy constructor
        ImageBuffer(const ImageBuffer& other)
        : buffer_view(other.buffer_view)
        , mime_type(other.mime_type)
        , name(other.name)
        {
        }

        // Move constructor
        ImageBuffer(ImageBuffer&& other) noexcept
        : buffer_view(other.buffer_view)
        , mime_type(std::move(other.mime_type))
        , name(std::move(other.name))
        {
            other.buffer_view = 0;
        }
    };
    struct ImagePath
    {
        std::string uri;

        ImagePath() = default;

        explicit ImagePath(const std::string& uri)
        : uri(uri)
        {
        }

        // Copy constructor
        ImagePath(const ImagePath& other) : uri(other.uri)
        {
        }

        // Move constructor
        ImagePath(ImagePath&& other) noexcept : uri(std::move(other.uri))
        {
        }
    };
    using Image = std::variant< ImageBuffer, ImagePath >;


    // Structure for GLTF sampler
    struct Sampler
    {
        TextureFilter mag_filter;
        TextureFilter min_filter;
        TextureWrapMode wrap_t;
        TextureWrapMode wrap_s;
        TextureWrapMode wrap_r;

        Sampler( TextureFilter mag_filter,
                 TextureFilter min_filter,
                 TextureWrapMode wrap_t,
                 TextureWrapMode wrap_s,
                 TextureWrapMode wrap_r = TextureWrapMode::REPEAT)
       : mag_filter(mag_filter)
       , min_filter(min_filter)
       , wrap_t(wrap_t)
       , wrap_s(wrap_s)
       , wrap_r(wrap_r)
        {}
    };

    // Structure for GLTF textures
    struct Texture
    {
        size_t sampler;     // Sampler index
        size_t source;      // Image index

        Texture(size_t sampler, size_t source)
            : sampler(sampler), source(source) {}
    };

    // Structure for Material
    struct Material 
    {
        // Type of alpha mode
        enum class AlphaMode
        {
            AM_OPAQUE,
            AM_BLEND,
            AM_MASK,
            AM_UNKNOWN
        };

        // Structure for Texture Information
        struct TextureInfo 
        {
            int index;
            float scale = 1.0f;
        };

        // Structure for PBR Metallic Roughness
        struct PbrMetallicRoughness 
        {
            Vec4 baseColorFactor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            std::optional<TextureInfo> base_color_texture;
            float metallic_factor = 1.0f;
            float roughness_factor = 1.0f;
            std::optional<TextureInfo> metallic_roughness_texture;
        };
        
        // Sub-structure for Specular-Glossiness (Extension)
        struct SpecularGlossiness
        {
            Vec4 diffuse_factor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            Vec3 specular_factor = Vec3(1.0f, 1.0f, 1.0f);
            float glossiness_factor = 1.0f;           
            std::optional<TextureInfo> diffuse_texture;
            std::optional<TextureInfo> specular_glossiness_texture;
        };

        std::string name;
        std::optional<PbrMetallicRoughness> pbr_metallic_roughness;
        std::optional<TextureInfo> normal_texture;
        std::optional<TextureInfo> occlusion_texture;
        std::optional<TextureInfo> emissive_texture;
        Vec3 emissive_factor;
        AlphaMode alpha_mode;
        float alpha_cutoff;
        bool double_sided;
        std::optional<SpecularGlossiness> extra_specular_glossiness;

        Material(
            const std::string& name = std::string(),
            const std::optional<PbrMetallicRoughness>& pbr_metallic_roughness = std::nullopt,
            const std::optional<TextureInfo>& normal_texture = std::nullopt,
            const std::optional<TextureInfo>& occlusion_texture = std::nullopt,
            const std::optional<TextureInfo>& emissive_texture = std::nullopt,
            const Vec3& emissive_factor = Vec3(1.0f, 1.0f, 1.0f),
            const AlphaMode alpha_mode = AlphaMode::AM_OPAQUE,
            float alpha_cutoff = 0.5f,
            bool double_sided = false,
            const std::optional<SpecularGlossiness>& extra_specular_glossiness = std::nullopt
        )
            : name(name)
            , pbr_metallic_roughness(pbr_metallic_roughness)
            , normal_texture(normal_texture)
            , occlusion_texture(occlusion_texture)
            , emissive_texture(emissive_texture)
            , emissive_factor(emissive_factor)
            , alpha_mode(alpha_mode)
            , alpha_cutoff(alpha_cutoff)
            , double_sided(double_sided)
            , extra_specular_glossiness(extra_specular_glossiness)
        {}
    };

    // Variant type for the possible transformations
    using NodeTransform = std::variant<std::monostate, Vec3, Quat, Mat4>;
    using NodeList = std::vector<size_t>;

    // Structure for GLTF node
    struct Node
    {
        enum NodeContentType
        {
            NT_MESH,
            NT_CAMERA,
            NT_LIGHT
        };

        struct NodeContent
        {
            NodeContentType m_type;
            size_t m_id;
        };

        std::string name;                   // Name of the node
        NodeList children;                  // Indices of child nodes
        std::optional<NodeContent> content; // Index of the mesh this node references (if any)
        Vec3 translation = { 0,0,0 };
        Quat rotation = { 0,0,0,1 };
        Vec3 scale = { 1,1,1 };
        Mat4 matrix = Mat4(1);
        TransformType transform_type;    // Type of transformation (enum)

        // Constructor
        Node(std::string name
            , std::vector<size_t>&& children
            , std::optional<NodeContent> content
            , Vec3&& translation
            , Quat&& rotation
            , Vec3&& scale
            , Mat4&& matrix
            , TransformType transform_type)
            : name(std::move(name))
            , children(std::forward<std::vector<size_t>>(children))
            , content(content)
            , translation(std::forward<Vec3>(translation))
            , rotation(std::forward<Quat>(rotation))
            , scale(std::forward<Vec3>(scale))
            , matrix(std::forward<Mat4>(matrix))
            , transform_type(transform_type)
        {}
    };

    // Structure for GLTF scene
    struct Scene
    {
        std::string name;   // Name of the scene
        NodeList nodes;     // Root node indices of the scene

        Scene(std::string name, std::vector<size_t>&& nodes)
            : name(std::move(name))
            , nodes(std::forward<std::vector<size_t>>(nodes))
        {}
    };

    // Aliases for vectors of Views, Buffers, Nodes and Scenes
    using Views = std::vector<View>;
    using Buffers = std::vector<std::vector<unsigned char>>;
    using Accessors = std::vector<Accessor>;
    using Meshes = std::vector<Mesh>;
    using Cameras = std::vector<Camera>;
    using Lights = std::vector<Light>;
    using Images = std::vector<Image>;
    using Samplers = std::vector<Sampler>;
    using Textures = std::vector<Texture>;
    using Materials = std::vector<Material>;
    using Nodes = std::vector<Node>;
    using Scenes = std::vector<Scene>;

    // GLTF Loader
    struct GLTF
    {
        Buffers buffers;
        Views views;
        Accessors accessors;
        Meshes meshes;
        Cameras cameras;
        Lights lights;
        Images images;
        Samplers samplers;
        Textures textures;
        Materials materials;
        Nodes nodes;            // List of all nodes in the GLTF model
        Scenes scenes;          // List of all scenes in the GLTF model
        size_t default_scene;   // Index of the default scene

        GLTF() = default;
        GLTF(GLTF&&) = default;
        GLTF(Buffers&& buffers, Views&& views, Accessors&& accessors, Meshes&& meshes, Cameras&& cameras, Lights&& lights, Images&& images, Samplers&& samplers, Textures&& textures, Materials&& materials, Nodes&& nodes, Scenes&& scenes, size_t default_scene)
            : buffers(std::forward<Buffers>(buffers))
            , views(std::forward<Views>(views))
            , accessors(std::forward<Accessors>(accessors))
            , meshes(std::forward<Meshes>(meshes))
            , cameras(std::forward<Cameras>(cameras))
            , lights(std::forward<Lights>(lights))
            , images(std::forward<Images>(images))
            , samplers(std::forward<Samplers>(samplers))
            , textures(std::forward<Textures>(textures))
            , materials(std::forward<Materials>(materials))
            , nodes(std::forward<Nodes>(nodes))
            , scenes(std::forward<Scenes>(scenes))
            , default_scene(default_scene)
        {}
    };

    // GLTF loader return
    using GLTFReturn = std::variant<std::monostate, GLTF, std::string>;

    // Function to load buffers from file
    static Buffers load_buffers(const std::string& filepath, const JsonValue& buffers)
    {
        Buffers out_buffers;
        using namespace Square;
        using namespace Square::Filesystem;

        // Get the base directory
        std::string directory_fullpath = get_directory(filepath);

        // Get array
        auto& jbuffers = buffers.array({});
        out_buffers.reserve(jbuffers.size());

        // Get the path for each binary
        for (auto& i_jbuffer : jbuffers)
        {
            // Get buffer
            auto& jbuffer = i_jbuffer.object({});
            auto it_uri = jbuffer.find("uri");
            if (it_uri != jbuffer.end())
            {
                auto buffer_path = join(directory_fullpath, it_uri->second.string({}));
                // Load file
                if (exists(buffer_path))
                {
                    out_buffers.emplace_back(binary_file_read_all(buffer_path));
                }
            }
        }
        return out_buffers;
    }

    // Function to retrieve views from JSON data
    static Views decode_views(const JsonValue& views)
    {
        Views out_views;
        auto& jviews = views.array({});
        out_views.reserve(jviews.size());

        for (auto& i_view : jviews)
        {
            auto& jview = i_view.object({});
            out_views.emplace_back
            (
                jview.find("buffer") != jview.end() ? jview.at("buffer").number(0) : 0,
                jview.find("byteOffset") != jview.end() ? jview.at("byteOffset").number(0) : 0,
                jview.find("byteLength") != jview.end() ? jview.at("byteLength").number(0) : 0,
                jview.find("target") != jview.end() ? jview.at("target").number(0) : 0
            );
        }
        return out_views;
    }

    // Function to decode GLTF accessors from JSON data
    static Accessors decode_accessors(const JsonValue& accessors)
    {
        Accessors out_accessors;
        auto& jaccessors = accessors.array({});
        out_accessors.reserve(jaccessors.size());

        for (auto& i_jaccessor : jaccessors)
        {
            JsonObject jaccessor = i_jaccessor.object({});

            size_t buffer_view = jaccessor["bufferView"].number(0);
            size_t byte_offset = jaccessor["byteOffset"].number(0);
            ComponentType component_type = static_cast<ComponentType>(jaccessor["componentType"].number(0));
            size_t count = jaccessor["count"].number(0);

            Square::Data::JsonArray max = jaccessor["max"].array({});
            Square::Data::JsonArray min = jaccessor["min"].array({});

            // Determine the correct LayoutType based on the string
            LayoutType layout_type = s_name_to_layout_type.at(jaccessor["type"].string("SCALAR"));

            // Get the number of components using the LayoutType map
            unsigned char num_components = s_layout_type_number_of_components.at(layout_type);

            // Add to the list of accessors
            out_accessors.emplace_back(
                buffer_view,
                byte_offset,
                component_type,
                count,
                max,
                min,
                layout_type,
                num_components
            );
        }

        return out_accessors;
    }

    //  Function to decode GLTF images JSON data
    static Images decode_images(const JsonValue& images)
    {
        Images out_images;
        auto& jimages = images.array({});
        out_images.reserve(jimages.size());

        for (auto& jimage : jimages)
        {
            auto image = jimage.object({});
            if (jimage.object().find("uri") != jimage.object().end())
            {
                std::string uri = image["uri"].string("");
                out_images.emplace_back(ImagePath{ uri });
            }
            else if (image.find("bufferView") != image.end())
            {
                ImageBuffer image_buffer
                {
                    static_cast<size_t>(image["bufferView"].number(0)),
                    image["mimeType"].string(""),
                    image["name"].string("")
                };
                out_images.emplace_back(image_buffer);
            }
        }

        return out_images;
    }

    //  Function to decode GLTF samplers JSON data
    static Samplers decode_samplers(const JsonValue& samplers)
    {
        Samplers out_samplers;
        auto& jsamplers = samplers.array({});
        out_samplers.reserve(jsamplers.size());

        for (auto& jsampler : jsamplers)
        {
            auto sampler = jsampler.object({});
            TextureFilter mag_filter = (TextureFilter) sampler["magFilter"].number((float)TextureFilter::NEAREST);
            TextureFilter min_filter = (TextureFilter) sampler["minFilter"].number((float)TextureFilter::NEAREST);
            TextureWrapMode wrap_t = (TextureWrapMode) sampler["wrapT"].number((float)TextureWrapMode::REPEAT);
            TextureWrapMode wrap_s = (TextureWrapMode) sampler["wrapS"].number((float)TextureWrapMode::REPEAT);
            TextureWrapMode wrap_r = (TextureWrapMode) sampler["wrapR"].number((float)TextureWrapMode::REPEAT);
            out_samplers.emplace_back
            (
                mag_filter,
                min_filter,
                wrap_t,
                wrap_s,
                wrap_r
            );
        }

        return out_samplers;
    }

    // Function to decode GLTF textures JSON data
    static Textures decode_textures(const JsonValue& textures)
    {
        Textures out_textures;
        auto& jtextures = textures.array({});
        out_textures.reserve(jtextures.size());

        for (auto& jtexture : jtextures)
        {
            auto texture = jtexture.object({});
            size_t sampler_index = texture["sampler"].number(0);
            size_t image_index = texture["source"].number(0);

            out_textures.emplace_back(sampler_index, image_index);
        }

        return out_textures;
    }

    // Function to decode GLTF meshes from JSON data
    static Meshes decode_meshes(const JsonValue& meshes)
    {
        Meshes out_meshes;
        auto& jmeshes = meshes.array({});
        out_meshes.reserve(jmeshes.size()); // Reserve space in the vector for performance

        for (auto& jmesh : jmeshes)
        {
            auto& mesh = jmesh.object({});
            std::string name = mesh.at("name").string({});

            Primitives primitives;
            auto& jprimitives = mesh.at("primitives").array({});
            primitives.reserve(jprimitives.size()); // Reserve space in the vector for performance

            for (auto& jprimitive : jprimitives)
            {
                auto& primitive = jprimitive.object({});

                // Decode attributes
                PrimitiveAttributes attributes;
                auto& jattributes = primitive.at("attributes").object({});
                for (const auto& attribute : jattributes)
                {
                    attributes.emplace(attribute.first, attribute.second.number(0));
                }

                // Decode other primitive properties
                size_t indices = primitive.at("indices").number(0);
                size_t material = primitive.find("material") != primitive.end() 
                                ? primitive.at("material").number(0) 
                                : 0;
                PrimitiveMode mode = PrimitiveMode::TRIANGLES;
                if (auto mode_it = primitive.find("mode");  mode_it != primitive.end())
                {
                    mode = static_cast<PrimitiveMode>(primitive.at("mode").number(int(PrimitiveMode::TRIANGLES)));
                }
                // Create and add the Primitive object to the primitives vector
                primitives.emplace_back(std::move(attributes), indices, material, mode);
            }

            // Create and add the Mesh object to the out_meshes vector
            out_meshes.emplace_back(name, std::move(primitives));
        }

        return out_meshes;
    }

    // Function to decode GLTF cameras from JSON data
    static Cameras decode_cameras(const JsonValue& cameras)
    {
        Cameras out_cameras;
        auto& jcameras = cameras.array({});
        out_cameras.reserve(jcameras.size()); // Reserve space in the vector for performance

        for (auto& jcamera : jcameras)
        {
            auto& camera = jcamera.object({});
            std::string type = camera.at("type").string({});

            if (type == "perspective")
            {
                auto& prospective = camera.at("perspective").object({});
                out_cameras.emplace_back(CameraProspective{
                        prospective.at("aspectRatio").number(0),
                        prospective.at("yfov").number(0),
                        prospective.at("zfar").number(0),
                        prospective.at("znear").number(0)
                });
            } 
            else if (type == "orthographic")
            {
                auto& orthographic = camera.at("perspective").object({});
                out_cameras.emplace_back(CameraOrthographic{
                        orthographic.at("xmag").number(0),
                        orthographic.at("ymag").number(0),
                        orthographic.at("zfar").number(0),
                        orthographic.at("znear").number(0)
                });
            } 
            else
            {
                // Dump camera
                out_cameras.emplace_back(CameraUndefined{});
            }
        }
        return out_cameras;
    }
    
    // Function to decode GLTF lights from JSON data
    static Lights decode_lights(const JsonValue& lights)
    {
        Lights out_lights;
        auto& jlights = lights.array({});
        out_lights.reserve(jlights.size()); // Reserve space in the vector for performance

        for (auto& jlight : jlights)
        {
            const auto& light = jlight.object(JsonObject{
                {"name",""},
                {"intensity",1.0},
                {"type",""}
            });
            std::string name = light.at("name").string({});
            double intensity = light.at("intensity").number(1.0);            
            std::string jstring_type = light.at("type").string({});
            Light::LightType type = jstring_type == "spot" 
                                  ? Light::LightType::LT_SPOTLIGHT
                                  : jstring_type == "point"
                                  ? Light::LightType::LT_POINT
                                  : Light::LightType::LT_DIRECTIONAL;

            Vec3 color{ 1.0,1.0,1.0 };
            if (light.find("color") != light.end())
            {
                auto& jcolor_values = light.at("color").array({ 1.0,1.0,1.0 });
                color = Vec3(jcolor_values[0].number(1.0f), jcolor_values[1].number(1.0f), jcolor_values[2].number(1.0f));
            }

            std::optional<double> range;
            if (type != Light::LightType::LT_DIRECTIONAL)
            if (light.find("range") != light.end())
            {
                range = light.at("range").number(std::numeric_limits<double>::max());
            }

            std::optional<Light::SpotFields> spotfields;
            if (type == Light::LightType::LT_SPOTLIGHT)
            if (light.find("spot") != light.end())
            {
                auto& spot = light.at("spot").object({});
                spotfields = Light::SpotFields
                {
                    spot.at("innerConeAngle").number(0),
                    spot.at("outerConeAngle").number(Square::Constants::pi<double>() / 4.0),
                };
            }

            out_lights.push_back(Light {
                name,
                color,
                intensity,
                type,
                range,
                spotfields
            });
        }
        return out_lights;
    }

    namespace Aux
    {
        // Function to decode Alpha mode from string
        static Material::AlphaMode decode_alpha_mode(const std::string& alpha_mode)
        {
            if (Square::case_insensitive_equal(alpha_mode, "OPQUE"))
                return Material::AlphaMode::AM_OPAQUE;
            else if (Square::case_insensitive_equal(alpha_mode, "BLEND"))
                return Material::AlphaMode::AM_BLEND;
            else if (Square::case_insensitive_equal(alpha_mode, "MASK"))
                return Material::AlphaMode::AM_MASK;
            return Material::AlphaMode::AM_UNKNOWN;
        }

        // Function to decode TextureInfo from JSON data
        static std::optional<Material::TextureInfo> decode_texture_info(const JsonObject& texture)
        {
            if (texture.empty()) return std::nullopt;

            int index = texture.at("index").number(0);
            float scale = texture.find("scale") != texture.end() ?
                texture.at("scale").number(1.0f) : 1.0f;
            return Material::TextureInfo{ index, scale };
        }

        // Function to decode PBR Metallic-Roughness from JSON data
        static Material::PbrMetallicRoughness decode_pbr_metallic_roughness(const JsonObject& pbr)
        {
            Vec4 base_color_factor{ 0.0f };
            if (pbr.find("baseColorFactor") != pbr.end())
            {
                auto& base_color = pbr.at("baseColorFactor").array({ 1.0,1.0,1.0,1.0 });
                base_color_factor = Vec4(
                    base_color[0].number(1.0f),
                    base_color[1].number(1.0f),
                    base_color[2].number(1.0f),
                    base_color[3].number(1.0f)
                );
            }

            float metallic_factor = pbr.find("metallicFactor") != pbr.end() ?
                pbr.at("metallicFactor").number(1.0f) : 1.0f;
            float roughness_factor = pbr.find("roughnessFactor") != pbr.end() ?
                pbr.at("roughnessFactor").number(1.0f) : 1.0f;

            std::optional<Material::TextureInfo> base_color_texture = std::nullopt;
            if (pbr.find("baseColorTexture") != pbr.end())
            {
                base_color_texture = decode_texture_info(pbr.at("baseColorTexture").object({}));
            }

            std::optional<Material::TextureInfo> metallic_roughness_texture = std::nullopt;
            if (pbr.find("metallicRoughnessTexture") != pbr.end())
            {
                metallic_roughness_texture = decode_texture_info(pbr.at("metallicRoughnessTexture").object({}));
            }

            return Material::PbrMetallicRoughness{
                base_color_factor,
                base_color_texture,
                metallic_factor,
                roughness_factor,
                metallic_roughness_texture
            };
        }

        // Function to decode Specular-Glossiness from JSON data
        static std::optional<Material::SpecularGlossiness> decode_specular_glossiness(const JsonObject& specular_glossiness)
        {
            if (specular_glossiness.empty()) return std::nullopt;

            Vec4 diffuse_factor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            if (specular_glossiness.find("diffuseFactor") != specular_glossiness.end())
            {
                auto& diffuse = specular_glossiness.at("diffuseFactor").array({ 1.0,1.0,1.0,1.0 });
                diffuse_factor = Vec4(
                    diffuse[0].number(1.0f),
                    diffuse[1].number(1.0f),
                    diffuse[2].number(1.0f),
                    diffuse[3].number(1.0f)
                );
            }

            Vec3 specular_factor = Vec3(1.0f, 1.0f, 1.0f);
            if (specular_glossiness.find("specularFactor") != specular_glossiness.end())
            {
                auto& specular = specular_glossiness.at("specularFactor").array({ 1.0,1.0,1.0 });
                specular_factor = Vec3(
                    specular[0].number(1.0f),
                    specular[1].number(1.0f),
                    specular[2].number(1.0f)
                );
            }

            float glossiness_factor = specular_glossiness.find("glossinessFactor") != specular_glossiness.end() ?
                specular_glossiness.at("glossinessFactor").number(1.0f) : 1.0f;

            std::optional<Material::TextureInfo> diffuse_texture = std::nullopt;
            if (specular_glossiness.find("diffuseTexture") != specular_glossiness.end())
            {
                diffuse_texture = Aux::decode_texture_info(specular_glossiness.at("diffuseTexture").object({}));
            }

            std::optional<Material::TextureInfo> specular_glossiness_texture = std::nullopt;
            if (specular_glossiness.find("specularGlossinessTexture") != specular_glossiness.end())
            {
                specular_glossiness_texture = decode_texture_info(specular_glossiness.at("specularGlossinessTexture").object({}));
            }

            return Material::SpecularGlossiness{
                diffuse_factor,
                specular_factor,
                glossiness_factor,
                diffuse_texture,
                specular_glossiness_texture
            };
        }
    }

    // Function to decode GLTF materials from JSON data
    static Materials decode_materials(const JsonValue& materials)
    {
        Materials out_materials;
        auto& jmaterials = materials.array({});
        out_materials.reserve(jmaterials.size());

        for (auto& jmaterial : jmaterials)
        {
            auto& mat = jmaterial.object({});
            std::string name = mat.find("name") != mat.end() ? mat.at("name").string({}) : "";

            // Decode each sub-structure using the helper functions
            std::optional<Material::PbrMetallicRoughness> pbr_metallic_roughness = std::nullopt;
            if (mat.find("pbrMetallicRoughness") != mat.end())
            {
                pbr_metallic_roughness = Aux::decode_pbr_metallic_roughness(mat.at("pbrMetallicRoughness").object({}));
            }

            std::optional<Material::TextureInfo> normal_texture = std::nullopt;
            if (mat.find("normalTexture") != mat.end())
            {
                normal_texture = Aux::decode_texture_info(mat.at("normalTexture").object({}));
            }

            std::optional<Material::TextureInfo> occlusion_texture = std::nullopt;
            if (mat.find("occlusionTexture") != mat.end())
            {
                occlusion_texture = Aux::decode_texture_info(mat.at("occlusionTexture").object({}));
            }

            std::optional<Material::TextureInfo> emissive_texture = std::nullopt;
            if (mat.find("emissiveTexture") != mat.end())
            {
                emissive_texture = Aux::decode_texture_info(mat.at("emissiveTexture").object({}));
            }

            Vec3 emissive_factor = Vec3(1.0f, 1.0f, 1.0f);
            if (mat.find("emissiveFactor") != mat.end())
            {
                auto& factor = mat.at("emissiveFactor").array({1.0f,1.0f,1.0f});
                emissive_factor = Vec3(
                    factor[0].number(0.0f),
                    factor[1].number(0.0f),
                    factor[2].number(0.0f)
                );
            }

            Material::AlphaMode alpha_mode = mat.find("alphaMode") != mat.end()
                                            ? Aux::decode_alpha_mode(mat.at("alphaMode").string("OPAQUE"))
                                            : Material::AlphaMode::AM_OPAQUE;
            float alpha_cutoff = mat.find("alphaCutoff") != mat.end() 
                               ?  mat.at("alphaCutoff").number(0.5f)
                               : 0.5f;
            bool double_sided = mat.find("doubleSided") != mat.end() 
                              ? mat.at("doubleSided").boolean(false) 
                              : false;

            std::optional<Material::SpecularGlossiness> extra_specular_glossiness = std::nullopt;
            if (mat.find("extensions") != mat.end())
            {
                auto& exts = mat.at("extensions").object();
                if (exts.find("KHR_materials_pbrSpecularGlossiness") != exts.end())
                {
                    extra_specular_glossiness = Aux::decode_specular_glossiness(exts.at("KHR_materials_pbrSpecularGlossiness").object({}));
                }
            }

            // Add the material to the vector
            out_materials.emplace_back(
                name,
                pbr_metallic_roughness,
                normal_texture,
                occlusion_texture,
                emissive_texture,
                emissive_factor,
                alpha_mode,
                alpha_cutoff,
                double_sided,
                extra_specular_glossiness
            );
        }

        return out_materials;
    }

    // Function to decode GLTF nodes from JSON data
    static Nodes decode_nodes(const JsonValue& nodes)
    {
        Nodes out_nodes;
        auto& jnodes = nodes.array({});
        out_nodes.reserve(jnodes.size());

        for (auto& jnode : jnodes)
        {
            auto& node = jnode.object({});
            std::string name = node.at("name").string({});
            NodeList children;
            std::optional<Node::NodeContent> content = std::nullopt;

            // Decode children
            const auto& jchildren = node.find("children");
            if (jchildren != node.end())
            {
                for (auto& child : jchildren->second.array({}))
                {
                    children.push_back(child.number(0));
                }
            }

            // Decode mesh reference (if present)
            if (node.find("mesh") != node.end())
            {
                content = Node::NodeContent
                {
                    Node::NodeContentType::NT_MESH,
                    static_cast<size_t>(node.at("mesh").number(0))
                };
            }
            else if (node.find("camera") != node.end())
            {
                content = Node::NodeContent
                {
                    Node::NodeContentType::NT_CAMERA,
                    static_cast<size_t>(node.at("camera").number(0))
                };
            }
            else if (node.find("extensions") != node.end())
            {
                auto& extensions = node.at("extensions").object({});
                if (extensions.find("KHR_lights_punctual") != extensions.end())
                {
                    const auto& jhr_lights_punctual = extensions.at("KHR_lights_punctual").object({});
                    if (jhr_lights_punctual.find("light") != jhr_lights_punctual.end())
                    {
                        content = Node::NodeContent
                        {
                            Node::NodeContentType::NT_LIGHT,
                            static_cast<size_t>(jhr_lights_punctual.at("light").number(0))
                        };
                    }
                }
            }

            // Decode transformation (use variant for translation, rotation, scale, or matrix)
            Vec3 translation(0, 0, 0);
            Quat rotation(0,0,0,0);
            Vec3 scale(1,1,1);
            Mat4 matrix(1);
            unsigned char transform_type = TransformType::NONE;

            // Check for translation
            if (node.find("translation") != node.end())
            {
                auto& trans_values = node.at("translation").array({});
                translation = Vec3(trans_values[0].number(0.0f), trans_values[1].number(0.0f), trans_values[2].number(0.0f));
                transform_type |= TransformType::TRANSLATION;
            }

            // Check for rotation
            if (node.find("rotation") != node.end())
            {
                auto& rot_values = node.at("rotation").array({});
                rotation = Quat(rot_values[0].number(1.0f), rot_values[1].number(0.0f), rot_values[2].number(0.0f), rot_values[3].number(0.0f));
                transform_type |= TransformType::ROTATION;
            }

            // Check for scale
            if (node.find("scale") != node.end())
            {
                auto& scale_values = node.at("scale").array({});
                scale = Vec3(scale_values[0].number(1.0f), scale_values[1].number(1.0f), scale_values[2].number(1.0f));
                transform_type |= TransformType::SCALE;
            }

            // Check for matrix (if present, overrides any T/R/S provided)
            if (node.find("matrix") != node.end())
            {
                auto& matrix_values = node.at("matrix").array({
                    1,0,0,0,
                    0,1,0,0,
                    0,0,1,0,
                    0,0,0,1
                });
                matrix = Mat4({
                    matrix_values[0].number(1.0f), matrix_values[1].number(0.0f), matrix_values[2].number(0.0f), matrix_values[3].number(0.0f),
                    matrix_values[4].number(0.0f), matrix_values[5].number(1.0f), matrix_values[6].number(0.0f), matrix_values[7].number(0.0f),
                    matrix_values[8].number(0.0f), matrix_values[9].number(0.0f), matrix_values[10].number(1.0f), matrix_values[11].number(0.0f),
                    matrix_values[12].number(0.0f), matrix_values[13].number(0.0f), matrix_values[14].number(0.0f), matrix_values[15].number(1.0f)
                });
                transform_type |= TransformType::MATRIX;
            }

            // Add node to the list
            out_nodes.emplace_back(
                  name
                , std::move(children)
                , std::move(content)
                , std::move(translation)
                , std::move(rotation)
                , std::move(scale)
                , std::move(matrix)
                , TransformType(transform_type)
            );
        }

        return out_nodes;
    }

    // Function to decode GLTF scenes from JSON data
    static Scenes decode_scenes(const JsonValue& scenes)
    {
        Scenes out_scenes;
        auto& jscenes = scenes.array({});
        out_scenes.reserve(jscenes.size());

        for (auto& jscene : jscenes)
        {
            auto& scene = jscene.object({});
            std::string name = scene.find("name") != scene.end() ? scene.at("name").string({}) : "";

            // Decode nodes
            NodeList nodes;
            auto& jnodes = scene.at("nodes").array({});
            nodes.reserve(jnodes.size());

            for (auto& jnode : jnodes)
            {
                nodes.push_back(jnode.number(0));
            }

            // Add scene to the list
            out_scenes.emplace_back(std::move(name), std::move(nodes));
        }

        return out_scenes;
    }

    // Function to decode GLTF scene from JSON data
    static size_t decode_scene(const JsonObject& document)
    {
        return size_t(document.find("scene") != document.end() ? document.at("scene").number(0) : 0);
    }

    namespace Aux
    {
        // Decode GLTF from json and bins
        static GLTFReturn decode_from_json_and_buffers(Json&& jmodel, Buffers&& buffers)
        {
            // Get document:
            JsonObject& document = jmodel.document().object();
            JsonObject extensions = document["extensions"].object({});
            JsonObject KHR_lights_punctual = extensions["KHR_lights_punctual"].object({});
            // Get all components
            return GLTF
            {
                  std::move(buffers)
                , std::move(decode_views(document["bufferViews"]))
                , std::move(decode_accessors(document["accessors"]))
                , std::move(decode_meshes(document["meshes"]))
                , std::move(decode_cameras(document["cameras"]))
                , std::move(decode_lights(KHR_lights_punctual["lights"]))
                , std::move(decode_images(document["images"]))
                , std::move(decode_samplers(document["samplers"]))
                , std::move(decode_textures(document["textures"]))
                , std::move(decode_materials(document["materials"]))
                , std::move(decode_nodes(document["nodes"]))
                , std::move(decode_scenes(document["scenes"]))
                , decode_scene(document)
            };
        }
    }

    // Load GLTF from GLTF file
    static GLTFReturn from_gltf(const std::string& path)
    {
        // Load Json
        Json jmodel(Filesystem::text_file_read_all(path));
        // Get errors
        std::string parser_errors = jmodel.errors();
        //
        if (parser_errors.size())
        {
            return parser_errors;
        }
        // Or test if it is an object
        if (!jmodel.document().is_object())
        {
            return "Invalid GLTF";
        }
        // Get buffers:
        Buffers buffers = load_buffers(path, jmodel.document()["buffers"]);
        // Decode
        return Aux::decode_from_json_and_buffers(std::move(jmodel), std::move(buffers));
    }
    
    // Load GLTF from GLB bin
    static GLTFReturn from_glb(const GLB::GLB& glb)
    {
        // Load Json
        Json jmodel(glb.json);
        // Get errors
        std::string parser_errors = jmodel.errors();
        //
        if (parser_errors.size())
        {
            return parser_errors;
        }
        // Or test if it is an object
        if (!jmodel.document().is_object())
        {
            return "Invalid GLTF";
        }
        // As a vector
        Buffers buffers{ std::move(glb.bin) };
        // Decode
        return Aux::decode_from_json_and_buffers(std::move(jmodel), std::move(buffers));
    }

    // Load GLTF
    static GLTFReturn load(const std::string& path)
    {
        // Get Type
        auto model_extension = Filesystem::get_extension(path);
        // Model
        if (case_insensitive_equal(model_extension, GLB::extension))
        {
            auto glb_model = GLB::decode_glb(path);
            if (glb_model.has_value())
                return std::move(from_glb(glb_model.value()));
        }
        else if (case_insensitive_equal(model_extension, extension))
        {
            return std::move(from_gltf(path));
        }
        return "invalid format";
    }
} // namespace GLTF
} // namespace Data
} // namespace Square