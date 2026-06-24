//
//  RenderMTL.h  —  Square Metal render driver
//
#pragma once
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "Square/Config.h"
#include "Square/Driver/Render.h"

namespace Square
{
namespace Render
{
    // ── Forward declarations ────────────────────────────────────────────────
    class ContextMTL;
    class Shader;   // concrete definition below

    // ── Buffers ─────────────────────────────────────────────────────────────
    class ConstBuffer
    {
    public:
        id<MTLBuffer> m_buffer { nil };
        size_t        m_size   { 0   };
    };

    class VertexBuffer
    {
    public:
        id<MTLBuffer> m_buffer { nil };
        size_t        m_stride { 0   };
        size_t        m_count  { 0   };
        size_t        m_size   { 0   };
    };

    class IndexBuffer
    {
    public:
        id<MTLBuffer> m_buffer { nil };
        size_t        m_size   { 0   };
    };

    // ── Texture ──────────────────────────────────────────────────────────────
    class Texture
    {
    public:
        id<MTLTexture>      m_texture { nil };
        id<MTLSamplerState> m_sampler { nil };
        int                 m_last_bind{ -1 };
    };

    // ── Input layout ─────────────────────────────────────────────────────────
    class InputLayout
    {
    public:
        AttributeList         m_list;
        MTLVertexDescriptor*  m_vertex_desc { nil }; // built lazily
    };

    // ── Render target ────────────────────────────────────────────────────────
    class Target
    {
    public:
        struct Attachment { Texture* texture; TargetType type; };
        std::vector<Attachment> m_attachments;
        MTLPixelFormat m_color_fmt { MTLPixelFormatBGRA8Unorm };
        MTLPixelFormat m_depth_fmt { MTLPixelFormatDepth32Float_Stencil8 };
    };

    // ── Reflection data (parsed from the MSL header comment) ─────────────────
    struct MTLReflection
    {
        std::string entry_point;
        std::unordered_map<std::string, int>    textures; // name → slot
        std::unordered_map<std::string, int>    samplers; // name → slot
        std::unordered_map<std::string, int>    buffers;  // cbuffer name → slot
        struct Member { int buffer_slot; uint32_t offset; uint32_t size; };
        std::unordered_map<std::string, Member> members;  // loose uniform name → info
    };

    // ── Uniform for a texture slot ────────────────────────────────────────────
    class UniformTextureMTL : public Uniform
    {
    public:
        UniformTextureMTL(ContextMTL* ctx, Shader* sh, int slot)
            : m_context(ctx), m_shader(sh), m_slot(slot) {}

        virtual void set(Texture* t) override;
        virtual void set(int)                                    override {}
        virtual void set(float)                                  override {}
        virtual void set(double)                                 override {}
        virtual void set(const IVec2&)                           override {}
        virtual void set(const IVec3&)                           override {}
        virtual void set(const IVec4&)                           override {}
        virtual void set(const Vec2&)                            override {}
        virtual void set(const Vec3&)                            override {}
        virtual void set(const Vec4&)                            override {}
        virtual void set(const Mat3&)                            override {}
        virtual void set(const Mat4&)                            override {}
        virtual void set(const DVec2&)                           override {}
        virtual void set(const DVec3&)                           override {}
        virtual void set(const DVec4&)                           override {}
        virtual void set(const DMat3&)                           override {}
        virtual void set(const DMat4&)                           override {}
        virtual void set(Texture* t, size_t)                     override { set(t); }
        virtual void set(const int*, size_t)                     override {}
        virtual void set(const float*, size_t)                   override {}
        virtual void set(const double*, size_t)                  override {}
        virtual void set(const IVec2*, size_t)                   override {}
        virtual void set(const IVec3*, size_t)                   override {}
        virtual void set(const IVec4*, size_t)                   override {}
        virtual void set(const Vec2*, size_t)                    override {}
        virtual void set(const Vec3*, size_t)                    override {}
        virtual void set(const Vec4*, size_t)                    override {}
        virtual void set(const Mat3*, size_t)                    override {}
        virtual void set(const Mat4*, size_t)                    override {}
        virtual void set(const DVec2*, size_t)                   override {}
        virtual void set(const DVec3*, size_t)                   override {}
        virtual void set(const DVec4*, size_t)                   override {}
        virtual void set(const DMat3*, size_t)                   override {}
        virtual void set(const DMat4*, size_t)                   override {}
        virtual void set(const std::vector<Texture*>&)           override {}
        virtual void set(const std::vector<int>&)                override {}
        virtual void set(const std::vector<float>&)              override {}
        virtual void set(const std::vector<double>&)             override {}
        virtual void set(const std::vector<IVec2>&)              override {}
        virtual void set(const std::vector<IVec3>&)              override {}
        virtual void set(const std::vector<IVec4>&)              override {}
        virtual void set(const std::vector<Vec2>&)               override {}
        virtual void set(const std::vector<Vec3>&)               override {}
        virtual void set(const std::vector<Vec4>&)               override {}
        virtual void set(const std::vector<Mat3>&)               override {}
        virtual void set(const std::vector<Mat4>&)               override {}
        virtual void set(const std::vector<DVec2>&)              override {}
        virtual void set(const std::vector<DVec3>&)              override {}
        virtual void set(const std::vector<DVec4>&)              override {}
        virtual void set(const std::vector<DMat3>&)              override {}
        virtual void set(const std::vector<DMat4>&)              override {}
        virtual bool is_valid()   override { return m_slot >= 0; }
        virtual Shader* get_shader() override;
        virtual ~UniformTextureMTL() {}

        ContextMTL* m_context;
        Shader*     m_shader;
        int         m_slot;
    };

    // ── Uniform for a loose scalar/vector/matrix inside a cbuffer ────────────
    class UniformMemberMTL : public Uniform
    {
    public:
        UniformMemberMTL(ContextMTL* ctx, Shader* sh, int buf_slot, uint32_t offset, uint32_t sz)
            : m_context(ctx), m_shader(sh), m_buf_slot(buf_slot), m_offset(offset), m_size(sz) {}

        template<typename T>
        void write(const T& val) { write_raw(&val, sizeof(T)); }
        void write_raw(const void* data, size_t sz);

        virtual void set(Texture*)                               override {}
        virtual void set(int i)                                  override { write(i); }
        virtual void set(float f)                                override { write(f); }
        virtual void set(double d)                               override { write(d); }
        virtual void set(const IVec2& v)                         override { write(v); }
        virtual void set(const IVec3& v)                         override { write(v); }
        virtual void set(const IVec4& v)                         override { write(v); }
        virtual void set(const Vec2& v)                          override { write(v); }
        virtual void set(const Vec3& v)                          override { write(v); }
        virtual void set(const Vec4& v)                          override { write(v); }
        virtual void set(const Mat3& m)                          override { write(m); }
        virtual void set(const Mat4& m)                          override { write(m); }
        virtual void set(const DVec2& v)                         override { write(v); }
        virtual void set(const DVec3& v)                         override { write(v); }
        virtual void set(const DVec4& v)                         override { write(v); }
        virtual void set(const DMat3& m)                         override { write(m); }
        virtual void set(const DMat4& m)                         override { write(m); }
        virtual void set(Texture*, size_t)                       override {}
        virtual void set(const int* p, size_t n)                 override { write_raw(p, sizeof(int)*n); }
        virtual void set(const float* p, size_t n)               override { write_raw(p, sizeof(float)*n); }
        virtual void set(const double* p, size_t n)              override { write_raw(p, sizeof(double)*n); }
        virtual void set(const IVec2* p, size_t n)               override { write_raw(p, sizeof(IVec2)*n); }
        virtual void set(const IVec3* p, size_t n)               override { write_raw(p, sizeof(IVec3)*n); }
        virtual void set(const IVec4* p, size_t n)               override { write_raw(p, sizeof(IVec4)*n); }
        virtual void set(const Vec2* p, size_t n)                override { write_raw(p, sizeof(Vec2)*n); }
        virtual void set(const Vec3* p, size_t n)                override { write_raw(p, sizeof(Vec3)*n); }
        virtual void set(const Vec4* p, size_t n)                override { write_raw(p, sizeof(Vec4)*n); }
        virtual void set(const Mat3* p, size_t n)                override { write_raw(p, sizeof(Mat3)*n); }
        virtual void set(const Mat4* p, size_t n)                override { write_raw(p, sizeof(Mat4)*n); }
        virtual void set(const DVec2* p, size_t n)               override { write_raw(p, sizeof(DVec2)*n); }
        virtual void set(const DVec3* p, size_t n)               override { write_raw(p, sizeof(DVec3)*n); }
        virtual void set(const DVec4* p, size_t n)               override { write_raw(p, sizeof(DVec4)*n); }
        virtual void set(const DMat3* p, size_t n)               override { write_raw(p, sizeof(DMat3)*n); }
        virtual void set(const DMat4* p, size_t n)               override { write_raw(p, sizeof(DMat4)*n); }
        virtual void set(const std::vector<Texture*>&)           override {}
        virtual void set(const std::vector<int>& v)              override { write_raw(v.data(), sizeof(int)*v.size()); }
        virtual void set(const std::vector<float>& v)            override { write_raw(v.data(), sizeof(float)*v.size()); }
        virtual void set(const std::vector<double>& v)           override { write_raw(v.data(), sizeof(double)*v.size()); }
        virtual void set(const std::vector<IVec2>& v)            override { write_raw(v.data(), sizeof(IVec2)*v.size()); }
        virtual void set(const std::vector<IVec3>& v)            override { write_raw(v.data(), sizeof(IVec3)*v.size()); }
        virtual void set(const std::vector<IVec4>& v)            override { write_raw(v.data(), sizeof(IVec4)*v.size()); }
        virtual void set(const std::vector<Vec2>& v)             override { write_raw(v.data(), sizeof(Vec2)*v.size()); }
        virtual void set(const std::vector<Vec3>& v)             override { write_raw(v.data(), sizeof(Vec3)*v.size()); }
        virtual void set(const std::vector<Vec4>& v)             override { write_raw(v.data(), sizeof(Vec4)*v.size()); }
        virtual void set(const std::vector<Mat3>& v)             override { write_raw(v.data(), sizeof(Mat3)*v.size()); }
        virtual void set(const std::vector<Mat4>& v)             override { write_raw(v.data(), sizeof(Mat4)*v.size()); }
        virtual void set(const std::vector<DVec2>& v)            override { write_raw(v.data(), sizeof(DVec2)*v.size()); }
        virtual void set(const std::vector<DVec3>& v)            override { write_raw(v.data(), sizeof(DVec3)*v.size()); }
        virtual void set(const std::vector<DVec4>& v)            override { write_raw(v.data(), sizeof(DVec4)*v.size()); }
        virtual void set(const std::vector<DMat3>& v)            override { write_raw(v.data(), sizeof(DMat3)*v.size()); }
        virtual void set(const std::vector<DMat4>& v)            override { write_raw(v.data(), sizeof(DMat4)*v.size()); }
        virtual bool is_valid()   override { return m_buf_slot >= 0; }
        virtual Shader* get_shader() override;
        virtual ~UniformMemberMTL() {}

        ContextMTL* m_context;
        Shader*     m_shader;
        int         m_buf_slot;
        uint32_t    m_offset;
        uint32_t    m_size;
    };

    // ── UniformConstBuffer binding (binds an explicit MTLBuffer to a slot) ────
    // Each CB may map to a different slot in the VS and the FS (SPIRV-Cross
    // assigns slots per-stage independently), so we store all {stage, slot}
    // pairs and update every matching entry on bind/unbind.
    class UniformConstBufferMTL : public UniformConstBuffer
    {
    public:
        struct StageSlot { int stage; int slot; };

        UniformConstBufferMTL() {}
        UniformConstBufferMTL(ContextMTL* ctx, Shader* sh, std::vector<StageSlot> bindings)
            : m_context(ctx), m_shader(sh), m_bindings(std::move(bindings)) {}

        virtual void bind(const ConstBuffer* cb) override;
        virtual void unbind()                    override;
        virtual bool is_valid()                  override { return !m_bindings.empty(); }
        virtual Shader* get_shader()             override;
        virtual ~UniformConstBufferMTL()         {}

        ContextMTL*              m_context  { nullptr };
        Shader*                  m_shader   { nullptr };
        std::vector<StageSlot>   m_bindings;
        const ConstBuffer*       m_bound    { nullptr };
    };

    // ── PSO cache key ─────────────────────────────────────────────────────────
    struct PSOKey
    {
        Shader*        shader       { nullptr };
        InputLayout*   input_layout { nullptr };
        BlendState     blend;
        MTLPixelFormat color_fmt    { MTLPixelFormatBGRA8Unorm };
        MTLPixelFormat depth_fmt    { MTLPixelFormatDepth32Float_Stencil8 };

        bool operator==(const PSOKey& o) const
        {
            return shader == o.shader && input_layout == o.input_layout
                && blend == o.blend && color_fmt == o.color_fmt
                && depth_fmt == o.depth_fmt;
        }
    };
    struct PSOKeyHash
    {
        size_t operator()(const PSOKey& k) const
        {
            size_t h = std::hash<void*>()(k.shader);
            h ^= std::hash<void*>()(k.input_layout) << 1;
            h ^= std::hash<int>()(k.color_fmt) << 2;
            h ^= std::hash<int>()(k.depth_fmt) << 3;
            h ^= std::hash<bool>()(k.blend.m_enable) << 4;
            return h;
        }
    };

    // ── Shader (concrete definition of the forward-declared Render::Shader) ───
    class Shader
    {
    public:
        id<MTLFunction>  m_functions[ST_N_SHADER];
        MTLReflection    m_refl[ST_N_SHADER];

        // Per-stage CPU-side "auto" cbuffer for loose uniforms
        std::vector<unsigned char> m_auto_cpu[ST_N_SHADER];
        id<MTLBuffer>              m_auto_gpu[ST_N_SHADER];
        bool                       m_auto_dirty[ST_N_SHADER];

        // Explicitly bound cbuffers — indexed [stage][slot].
        // SPIRV-Cross auto-assigns slots independently per stage, so the same
        // named cbuffer may occupy a different slot in VS vs FS.
        static constexpr int MAX_BUFFERS = 16;
        const ConstBuffer* m_bound_cb_per_stage[ST_N_SHADER][MAX_BUFFERS];

        std::vector<std::string> m_compiler_errors;
        std::string              m_linker_error;

        // True when the vertex stage writes [[render_target_array_index]] (layered
        // rendering). Metal then requires the PSO to declare inputPrimitiveTopology.
        bool                     m_writes_layer { false };

        // Bound textures (index = texture slot)
        static constexpr int MAX_TEXTURES = 32;
        Texture* m_bound_textures[MAX_TEXTURES];

        ContextMTL& m_context;

        // Uniform handles are owned by the shader (lifetime == shader) and handed
        // out as non-owning pointers, mirroring the OpenGL/DirectX backends. The
        // caller must NOT delete them.
        std::unordered_map<std::string, std::unique_ptr<Uniform>>            m_uniform_cache;
        std::unordered_map<std::string, std::unique_ptr<UniformConstBuffer>> m_ucb_cache;

        explicit Shader(ContextMTL& ctx);
        ~Shader();

        void mark_auto_dirty(int stage) { m_auto_dirty[stage] = true; }
    };

    // ── Bind context ──────────────────────────────────────────────────────────
    struct BindContextMTL
    {
        Shader*       shader        { nullptr };
        InputLayout*  input_layout  { nullptr };
        Target*       render_target { nullptr };
        VertexBuffer* vertex_buffer { nullptr };
        IndexBuffer*  index_buffer  { nullptr };
    };

    // ── Main context ──────────────────────────────────────────────────────────
    class ContextMTL : public Context
    {
    public:
        ContextMTL(Allocator* alloc, Logger* log) : Context(alloc, log) {}
        ~ContextMTL() { close(); }

        virtual RenderDriver get_render_driver() override;
        virtual const RenderDriverInfo& get_render_driver_info() const override;
        virtual void print_info() override;

        virtual bool init(Video::DeviceResources* resource) override;
        virtual void close() override;
        virtual bool is_srgb_framebuffer() const override;

        virtual const ClearColorState& get_clear_color_state() override;
        virtual void set_clear_color_state(const ClearColorState& cf) override;
        virtual void clear(int type = CLEAR_COLOR_DEPTH) override;

        virtual const DepthBufferState& get_depth_buffer_state() override;
        virtual void set_depth_buffer_state(const DepthBufferState& cf) override;

        virtual const CullfaceState& get_cullface_state() override;
        virtual void set_cullface_state(const CullfaceState& cf) override;

        virtual const ViewportState& get_viewport_state() override;
        virtual void set_viewport_state(const ViewportState& vs) override;

        virtual const BlendState& get_blend_state() override;
        virtual void set_blend_state(const BlendState& bs) override;

        virtual const State& get_render_state() override;
        virtual void set_render_state(const State& rs) override;

        virtual ConstBuffer*  create_stream_CB(const unsigned char*, size_t) override;
        virtual VertexBuffer* create_stream_VBO(const unsigned char*, size_t, size_t) override;
        virtual IndexBuffer*  create_stream_IBO(const unsigned int*, size_t) override;
        virtual ConstBuffer*  create_CB(const unsigned char*, size_t) override;
        virtual VertexBuffer* create_VBO(const unsigned char*, size_t, size_t) override;
        virtual IndexBuffer*  create_IBO(const unsigned int*, size_t) override;
        virtual uint64 get_native_CB(const ConstBuffer*) const override;
        virtual uint64 get_native_VBO(const VertexBuffer*) const override;
        virtual uint64 get_native_IBO(const IndexBuffer*) const override;
        virtual size_t get_size_CB(const ConstBuffer*) const override;
        virtual size_t get_size_VBO(const VertexBuffer*) const override;
        virtual size_t get_size_IBO(const IndexBuffer*) const override;
        virtual void update_steam_CB(ConstBuffer*, const unsigned char*, size_t) override;
        virtual void update_steam_VBO(VertexBuffer*, const unsigned char*, size_t) override;
        virtual void update_steam_IBO(IndexBuffer*, const unsigned int*, size_t) override;
        virtual void bind_CB(ConstBuffer*) override;
        virtual void bind_VBO(VertexBuffer*) override;
        virtual void bind_IBO(IndexBuffer*) override;
        virtual void unbind_CB(ConstBuffer*) override;
        virtual void unbind_VBO(VertexBuffer*) override;
        virtual void unbind_IBO(IndexBuffer*) override;
        virtual std::vector<unsigned char> copy_buffer_CB(const ConstBuffer*) override;
        virtual std::vector<unsigned char> copy_buffer_VBO(const VertexBuffer*) override;
        virtual std::vector<unsigned char> copy_buffer_IBO(const IndexBuffer*) override;
        virtual unsigned char* map_CB(ConstBuffer*, size_t, size_t, MappingType) override;
        virtual unsigned char* map_CB(ConstBuffer*, MappingType) override;
        virtual void unmap_CB(ConstBuffer*) override;
        virtual unsigned char* map_VBO(VertexBuffer*, size_t, size_t, MappingType) override;
        virtual unsigned char* map_VBO(VertexBuffer*, MappingType) override;
        virtual void unmap_VBO(VertexBuffer*) override;
        virtual unsigned int* map_IBO(IndexBuffer*, size_t, size_t, MappingType) override;
        virtual unsigned int* map_IBO(IndexBuffer*, MappingType) override;
        virtual void unmap_IBO(IndexBuffer*) override;
        virtual unsigned char* map_TBO(Texture*, MappingType) override;
        virtual void unmap_TBO(Texture*) override;
        virtual void delete_CB(ConstBuffer*&) override;
        virtual void delete_VBO(VertexBuffer*&) override;
        virtual void delete_IBO(IndexBuffer*&) override;

        virtual void draw_arrays(DrawType, unsigned int) override;
        virtual void draw_arrays(DrawType, unsigned int, unsigned int) override;
        virtual void draw_elements(DrawType, unsigned int) override;
        virtual void draw_elements(DrawType, unsigned int, unsigned int) override;
        virtual void draw_arrays_instanced(DrawType, unsigned int start, unsigned int size, unsigned int instances) override;
        virtual void draw_elements_instanced(DrawType, unsigned int start, unsigned int n, unsigned int instances) override;

        virtual InputLayout* create_IL(const AttributeList&) override;
        virtual void delete_IL(InputLayout*&) override;
        virtual void bind_IL(InputLayout*) override;
        virtual void unbind_IL(InputLayout*) override;
        virtual size_t size_IL(const InputLayout*) override;
        virtual bool   has_a_position_IL(const InputLayout*) override;
        virtual size_t position_offset_IL(const InputLayout*) override;

        virtual float get_depth(const Vec2&) const override { return 0.f; }
        virtual Vec4  get_color(const Vec2&) const override { return Vec4(0,0,0,0); }

        virtual Texture* create_texture(const TextureRawDataInformation&, const TextureGpuDataInformation&) override;
        virtual Texture* create_texture_array(const TextureRawDataInformation&, const TextureGpuDataInformation&, int) override;
        virtual Texture* create_cube_texture(const TextureRawDataInformation[6], const TextureGpuDataInformation&) override;
        virtual std::vector<unsigned char> get_texture(Texture*, int) override;
        virtual std::vector<unsigned char> get_texture(Texture*, int, int) override;
        virtual void bind_texture(Texture*, int, int) override;
        virtual void unbind_texture(Texture*) override;
        virtual void unbind_texture(int) override;
        virtual void delete_texture(Texture*&) override;

        virtual Shader* create_shader(const std::vector<ShaderSourceInformation>&) override;
        virtual bool shader_compiled_with_errors(Shader*) override;
        virtual bool shader_linked_with_error(Shader*) override;
        virtual std::vector<std::string> get_shader_compiler_errors(Shader*) override;
        virtual std::string get_shader_linker_error(Shader*) override;
        virtual void bind_shader(Shader*) override;
        virtual void unbind_shader(Shader*) override;
        virtual void bind_uniform_CB(const ConstBuffer*, Shader*, const std::string&) override;
        virtual void unbind_uniform_CB(Shader*, const std::string&) override;
        virtual void delete_shader(Shader*&) override;
        virtual Shader* get_bind_shader() const override;
        virtual Uniform* get_uniform(Shader*, const std::string&) const override;
        virtual UniformConstBuffer* get_uniform_const_buffer(Shader*, const std::string&) const override;

        virtual Target* create_render_target(const std::vector<TargetField>&) override;
        virtual void enable_render_target(Target*) override;
        virtual void disable_render_target(Target*) override;
        virtual void delete_render_target(Target*&) override;
        virtual void copy_target_to_target(const IVec4&, Target*, const IVec4&, Target*, TargetType) override;

        virtual bool print_errors() const override { return false; }
        virtual bool print_errors(const char*, int) const override { return false; }

        // ── Internal ─────────────────────────────────────────────────────────
        static constexpr int VERTEX_BUFFER_BINDING = 16;

        id<MTLDevice>               m_device       { nil };
        CAMetalLayer*               m_layer        { nil };
        id<MTLCommandQueue>         m_queue        { nil };
        id<MTLCommandBuffer>        m_cmd_buf      { nil };
        id<MTLRenderCommandEncoder> m_encoder      { nil };
        id<CAMetalDrawable>         m_drawable     { nil };
        id<MTLTexture>              m_depth_tex    { nil };

        // Diagnostic: index of the current frame, used to gate verbose logging to
        // the first few frames so the captured output stays readable.
        unsigned                    m_frame_index  { 0 };

        // Per-frame autorelease pool. The host render loop is a plain C++ while()
        // with no @autoreleasepool, so autoreleased Metal objects (the drawable,
        // command buffer, encoders, pass descriptors) would otherwise accumulate
        // until the small CAMetalLayer drawable pool is exhausted and nextDrawable
        // stalls/returns nil — producing frame drops and flicker. We open the pool
        // at frame start (command-buffer creation) and drain it after commit.
        NSAutoreleasePool*          m_frame_pool   { nil };

        MTLPixelFormat              m_color_fmt    { MTLPixelFormatBGRA8Unorm };
        MTLPixelFormat              m_depth_fmt    { MTLPixelFormatDepth32Float_Stencil8 };
        bool                        m_srgb_fb      { false };

        // Pending clear flags: set by clear(), consumed when the next encoder is created.
        // This decouples clear() from render-pass creation so the load action is
        // correctly set regardless of whether clear() is called before or after
        // enable_render_target().
        bool                        m_pending_clear_color { true  }; // true → clear on first pass
        bool                        m_pending_clear_depth { true  };

        State           m_render_state;
        BindContextMTL  m_bind;
        RenderDriverInfo m_driver_info;

        std::unordered_map<PSOKey, id<MTLRenderPipelineState>, PSOKeyHash> m_pso_cache;
        std::unordered_map<uint32_t, id<MTLDepthStencilState>>             m_ds_cache;

        void ensure_command_buffer();
        void ensure_encoder(MTLRenderPassDescriptor* rpd = nil);
        void end_encoder();
        void commit_frame();
        void log_draw_state(const char* fn, unsigned int n); // diagnostic logging

        id<MTLRenderPipelineState> get_or_create_pso();
        id<MTLDepthStencilState>   get_or_create_ds();
        MTLVertexDescriptor*       build_vertex_desc(InputLayout* il);

        void apply_state_to_encoder();
        MTLRenderPassDescriptor* make_default_rp();
        MTLRenderPassDescriptor* make_target_rp(Target* t);
        void ensure_depth_texture(uint32_t w, uint32_t h);

        id<MTLBuffer> make_buffer(const void* data, size_t size);
        void generate_mipmaps(id<MTLTexture> tex);
    };

} // Render
} // Square
