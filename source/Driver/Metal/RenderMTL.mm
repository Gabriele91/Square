//
//  RenderMTL.mm  —  Square Metal render driver implementation
//
#include "RenderMTL.h"
#include "Square/Driver/Window.h"

#include <sstream>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cmath>
#include <simd/simd.h>

using namespace Square;
using namespace Square::Render;

#if SQ_MTL_DEBUG_LOG
    #define SQMTL_LOG(MSG)                                                       \
        do {                                                                     \
            if (m_frame_index < SQ_MTL_DEBUG_FRAMES && logger())                 \
            {                                                                    \
                std::ostringstream _sqmtl_ss;                                    \
                _sqmtl_ss << "[MTL f" << m_frame_index << "] " << MSG;           \
                logger()->info(_sqmtl_ss.str());                                 \
            }                                                                    \
        } while (0)
#else
    #define SQMTL_LOG(MSG) ((void)0)
#endif

// ──────────────────────────────────────────────────────────────────────────────
// Helpers: enum → Metal conversions
// ──────────────────────────────────────────────────────────────────────────────

static MTLPrimitiveType to_mtl_primitive(DrawType dt)
{
    switch (dt)
    {
    case DRAW_POINTS:         return MTLPrimitiveTypePoint;
    case DRAW_LINES:          return MTLPrimitiveTypeLine;
    case DRAW_LINE_LOOP:      return MTLPrimitiveTypeLine;
    case DRAW_TRIANGLES:      return MTLPrimitiveTypeTriangle;
    case DRAW_TRIANGLE_STRIP: return MTLPrimitiveTypeTriangleStrip;
    default:                  return MTLPrimitiveTypeTriangle;
    }
}

static MTLCullMode to_mtl_cull(CullfaceType ct)
{
    switch (ct)
    {
    case CF_FRONT:          return MTLCullModeFront;
    case CF_BACK:           return MTLCullModeBack;
    default:                return MTLCullModeNone;
    }
}

static MTLCompareFunction to_mtl_depth_func(DepthFuncType dt)
{
    switch (dt)
    {
    case DT_NEVER:         return MTLCompareFunctionNever;
    case DT_LESS:          return MTLCompareFunctionLess;
    case DT_GREATER:       return MTLCompareFunctionGreater;
    case DT_EQUAL:         return MTLCompareFunctionEqual;
    case DT_LESS_EQUAL:    return MTLCompareFunctionLessEqual;
    case DT_GREATER_EQUAL: return MTLCompareFunctionGreaterEqual;
    case DT_NOT_EQUAL:     return MTLCompareFunctionNotEqual;
    case DT_ALWAYS:        return MTLCompareFunctionAlways;
    default:               return MTLCompareFunctionLess;
    }
}

static MTLBlendFactor to_mtl_blend(BlendType bt)
{
    switch (bt)
    {
    case BLEND_ZERO:                return MTLBlendFactorZero;
    case BLEND_ONE:                 return MTLBlendFactorOne;
    case BLEND_SRC_COLOR:           return MTLBlendFactorSourceColor;
    case BLEND_ONE_MINUS_SRC_COLOR: return MTLBlendFactorOneMinusSourceColor;
    case BLEND_DST_COLOR:           return MTLBlendFactorDestinationColor;
    case BLEND_ONE_MINUS_DST_COLOR: return MTLBlendFactorOneMinusDestinationColor;
    case BLEND_SRC_ALPHA:           return MTLBlendFactorSourceAlpha;
    case BLEND_ONE_MINUS_SRC_ALPHA: return MTLBlendFactorOneMinusSourceAlpha;
    case BLEND_DST_ALPHA:           return MTLBlendFactorDestinationAlpha;
    case BLEND_ONE_MINUS_DST_ALPHA: return MTLBlendFactorOneMinusDestinationAlpha;
    case BLEND_SRC_ALPHA_SATURATE:  return MTLBlendFactorSourceAlphaSaturated;
    default:                        return MTLBlendFactorOne;
    }
}

static MTLPixelFormat to_mtl_pixel_format(TextureFormat tf, bool srgb)
{
    switch (tf)
    {
    case TF_RGBA8:    return srgb ? MTLPixelFormatRGBA8Unorm_sRGB : MTLPixelFormatRGBA8Unorm;
    case TF_RGB8:     return srgb ? MTLPixelFormatRGBA8Unorm_sRGB : MTLPixelFormatRGBA8Unorm;
    case TF_RGBA16F:  return MTLPixelFormatRGBA16Float;
    case TF_RGBA32F:  return MTLPixelFormatRGBA32Float;
    case TF_RG8:      return MTLPixelFormatRG8Unorm;
    case TF_RG16F:    return MTLPixelFormatRG16Float;
    case TF_RG32F:    return MTLPixelFormatRG32Float;
    case TF_R8:       return MTLPixelFormatR8Unorm;
    case TF_R16F:     return MTLPixelFormatR16Float;
    case TF_R32F:     return MTLPixelFormatR32Float;
    case TF_R16UI:    return MTLPixelFormatR16Uint;
    case TF_R32UI:    return MTLPixelFormatR32Uint;
    case TF_RGBA16UI: return MTLPixelFormatRGBA16Uint;
    case TF_RGBA32UI: return MTLPixelFormatRGBA32Uint;
    case TF_DEPTH16_STENCIL8:
    case TF_DEPTH24_STENCIL8:
    case TF_DEPTH32_STENCIL8:  return MTLPixelFormatDepth32Float_Stencil8;
    case TF_DEPTH_COMPONENT16: return MTLPixelFormatDepth16Unorm;
    case TF_DEPTH_COMPONENT24:
    case TF_DEPTH_COMPONENT32: return MTLPixelFormatDepth32Float;
    default:          return MTLPixelFormatRGBA8Unorm;
    }
}

static bool is_depth_format(TextureFormat tf)
{
    return tf == TF_DEPTH16_STENCIL8 || tf == TF_DEPTH24_STENCIL8
        || tf == TF_DEPTH32_STENCIL8 || tf == TF_DEPTH_COMPONENT16
        || tf == TF_DEPTH_COMPONENT24 || tf == TF_DEPTH_COMPONENT32;
}

static MTLSamplerMinMagFilter to_mtl_mag(TextureMagFilterType f)
{
    switch (f)
    {
    case TMAG_NEAREST:
    case TMAG_NEAREST_MIPMAP_NEAREST:
    case TMAG_NEAREST_MIPMAP_LINEAR: return MTLSamplerMinMagFilterNearest;
    default:                         return MTLSamplerMinMagFilterLinear;
    }
}

static MTLSamplerMinMagFilter to_mtl_min(TextureMinFilterType f)
{
    switch (f)
    {
    case TMIN_NEAREST:
    case TMIN_NEAREST_MIPMAP_NEAREST:
    case TMIN_NEAREST_MIPMAP_LINEAR: return MTLSamplerMinMagFilterNearest;
    default:                         return MTLSamplerMinMagFilterLinear;
    }
}

static MTLSamplerMipFilter to_mtl_mip(TextureMinFilterType f)
{
    switch (f)
    {
    case TMIN_NEAREST:
    case TMIN_LINEAR:                return MTLSamplerMipFilterNotMipmapped;
    case TMIN_NEAREST_MIPMAP_NEAREST:
    case TMIN_LINEAR_MIPMAP_NEAREST: return MTLSamplerMipFilterNearest;
    default:                         return MTLSamplerMipFilterLinear;
    }
}

static MTLSamplerAddressMode to_mtl_edge(TextureEdgeType e)
{
    switch (e)
    {
    case TEDGE_CLAMP:  return MTLSamplerAddressModeClampToEdge;
    case TEDGE_REPEAT: return MTLSamplerAddressModeRepeat;
    default:           return MTLSamplerAddressModeClampToEdge;
    }
}

static MTLVertexFormat to_mtl_vertex_format(AttributeStripType ast)
{
    switch (ast)
    {
    case AST_FLOAT:  return MTLVertexFormatFloat;
    case AST_FLOAT2: return MTLVertexFormatFloat2;
    case AST_FLOAT3: return MTLVertexFormatFloat3;
    case AST_FLOAT4: return MTLVertexFormatFloat4;
    case AST_INT:    return MTLVertexFormatInt;
    case AST_INT2:   return MTLVertexFormatInt2;
    case AST_INT3:   return MTLVertexFormatInt3;
    case AST_INT4:   return MTLVertexFormatInt4;
    case AST_UINT:   return MTLVertexFormatUInt;
    case AST_UINT2:  return MTLVertexFormatUInt2;
    case AST_UINT3:  return MTLVertexFormatUInt3;
    case AST_UINT4:  return MTLVertexFormatUInt4;
    default:         return MTLVertexFormatFloat4;
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// Reflection parser
// Format per stage line:
//   // SQUARE_MTL_REFL entry:NAME textures:N=S[,...] samplers:N=S[,...] buffers:N=S[,...] members:N=BS:OFF:SZ[,...]
// ──────────────────────────────────────────────────────────────────────────────

static void parse_refl_line(const std::string& line, MTLReflection& r)
{
    auto extract = [&](const char* key, std::string& out) -> bool {
        auto p = line.find(key);
        if (p == std::string::npos) return false;
        size_t start = p + strlen(key);
        auto e = line.find(' ', start);
        out = line.substr(start, e == std::string::npos ? e : e - start);
        return true;
    };

    std::string val;
    if (extract("entry:", val)) r.entry_point = val;

    auto parse_pairs = [&](const char* key, std::unordered_map<std::string,int>& map) {
        if (!extract(key, val) || val == "-") return;
        std::istringstream ss(val);
        std::string tok;
        while (std::getline(ss, tok, ','))
        {
            auto eq = tok.find('=');
            if (eq != std::string::npos)
                map[tok.substr(0, eq)] = std::stoi(tok.substr(eq + 1));
        }
    };

    parse_pairs("textures:", r.textures);
    parse_pairs("samplers:", r.samplers);
    parse_pairs("buffers:",  r.buffers);

    if (extract("members:", val) && val != "-")
    {
        std::istringstream ss(val);
        std::string tok;
        while (std::getline(ss, tok, ','))
        {
            auto eq = tok.find('=');
            auto c1 = tok.find(':', eq + 1);
            auto c2 = tok.find(':', c1 + 1);
            if (eq != std::string::npos && c1 != std::string::npos && c2 != std::string::npos)
            {
                MTLReflection::Member m;
                m.buffer_slot = std::stoi(tok.substr(eq + 1, c1 - eq - 1));
                m.offset      = (uint32_t)std::stoul(tok.substr(c1 + 1, c2 - c1 - 1));
                m.size        = (uint32_t)std::stoul(tok.substr(c2 + 1));
                r.members[tok.substr(0, eq)] = m;
            }
        }
    }
}

static MTLReflection parse_reflection(const std::string& msl_source)
{
    MTLReflection r;
    std::istringstream ss(msl_source);
    std::string line;
    while (std::getline(ss, line))
    {
        if (line.find("SQUARE_MTL_REFL") != std::string::npos)
        {
            parse_refl_line(line, r);
            break;
        }
    }
    return r;
}

// ──────────────────────────────────────────────────────────────────────────────
// Shader
// ──────────────────────────────────────────────────────────────────────────────

Shader::Shader(ContextMTL& ctx) : m_context(ctx)
{
    for (int i = 0; i < ST_N_SHADER; ++i)
    {
        m_functions[i]  = nil;
        m_auto_gpu[i]   = nil;
        m_auto_dirty[i] = false;
    }
    std::memset(m_bound_cb_per_stage, 0, sizeof(m_bound_cb_per_stage));
    std::memset(m_bound_textures, 0, sizeof(m_bound_textures));
}

Shader::~Shader()
{
    for (int i = 0; i < ST_N_SHADER; ++i)
    {
        m_functions[i] = nil;
        m_auto_gpu[i]  = nil;
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// Uniform implementations
// ──────────────────────────────────────────────────────────────────────────────

void UniformTextureMTL::set(Texture* t)
{
    if (m_slot >= 0 && m_slot < Shader::MAX_TEXTURES)
        m_shader->m_bound_textures[m_slot] = t;
}

Shader* UniformTextureMTL::get_shader() { return m_shader; }

void UniformMemberMTL::write_raw(const void* data, size_t sz)
{
    if (m_buf_slot < 0) return;
    for (int stage = 0; stage < ST_N_SHADER; ++stage)
    {
        if (!m_shader->m_functions[stage]) continue;
        auto& refl = m_shader->m_refl[stage];
        for (auto& b : refl.buffers)
        {
            if (b.second != m_buf_slot) continue;
            auto& cpu = m_shader->m_auto_cpu[stage];
            size_t need = m_offset + sz;
            if (cpu.size() < need) cpu.resize(need, 0);
            std::memcpy(cpu.data() + m_offset, data, sz);
            m_shader->mark_auto_dirty(stage);
        }
    }
}

Shader* UniformMemberMTL::get_shader() { return m_shader; }

void UniformConstBufferMTL::bind(const ConstBuffer* cb)
{
    m_bound = cb;
    for (const auto& ss : m_bindings)
        if (ss.slot >= 0 && ss.slot < Shader::MAX_BUFFERS)
            m_shader->m_bound_cb_per_stage[ss.stage][ss.slot] = cb;
}

void UniformConstBufferMTL::unbind()
{
    m_bound = nullptr;
    for (const auto& ss : m_bindings)
        if (ss.slot >= 0 && ss.slot < Shader::MAX_BUFFERS)
            m_shader->m_bound_cb_per_stage[ss.stage][ss.slot] = nullptr;
}

Shader* UniformConstBufferMTL::get_shader() { return m_shader; }

// ──────────────────────────────────────────────────────────────────────────────
// ContextMTL helpers
// ──────────────────────────────────────────────────────────────────────────────

id<MTLBuffer> ContextMTL::make_buffer(const void* data, size_t size)
{
    if (data)
        return [m_device newBufferWithBytes:data length:size options:MTLResourceStorageModeShared];
    return [m_device newBufferWithLength:size options:MTLResourceStorageModeShared];
}

void ContextMTL::ensure_command_buffer()
{
    if (!m_cmd_buf)
    {
        // Open the frame's autorelease pool before the first autoreleased Metal
        // object of the frame is created; commit_frame() drains it.
        if (!m_frame_pool) m_frame_pool = [[NSAutoreleasePool alloc] init];
        m_cmd_buf = [m_queue commandBuffer];
    }
}

void ContextMTL::ensure_depth_texture(uint32_t w, uint32_t h)
{
    if (m_depth_tex && m_depth_tex.width == w && m_depth_tex.height == h) return;
    m_depth_tex = nil;
    MTLTextureDescriptor* td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:m_depth_fmt
                                                                                  width:w
                                                                                 height:h
                                                                              mipmapped:NO];
    td.storageMode = MTLStorageModePrivate;
    td.usage       = MTLTextureUsageRenderTarget;
    m_depth_tex = [m_device newTextureWithDescriptor:td];
}

MTLRenderPassDescriptor* ContextMTL::make_default_rp()
{
    // Acquire the drawable once per frame — if clear() ends an in-flight
    // encoder mid-frame we must reuse the same drawable, not grab a new one.
    if (!m_drawable)
        m_drawable = [m_layer nextDrawable];
    if (!m_drawable)
    {
        SQMTL_LOG("make_default_rp: nextDrawable returned NIL (drawable pool exhausted?) -> frame skipped");
        return nil;
    }

    uint32_t w = (uint32_t)m_layer.drawableSize.width;
    uint32_t h = (uint32_t)m_layer.drawableSize.height;
    ensure_depth_texture(w, h);

    SQMTL_LOG("make_default_rp: drawable OK size=" << w << "x" << h
              << " clear_color=" << m_pending_clear_color
              << " clear_depth=" << m_pending_clear_depth);

    MTLRenderPassDescriptor* rpd = [MTLRenderPassDescriptor renderPassDescriptor];
    const Vec4& cc = m_render_state.m_clear_color.m_color;

    MTLLoadAction color_load = m_pending_clear_color ? MTLLoadActionClear : MTLLoadActionLoad;
    MTLLoadAction depth_load = m_pending_clear_depth ? MTLLoadActionClear : MTLLoadActionLoad;
    m_pending_clear_color = false;
    m_pending_clear_depth = false;

    rpd.colorAttachments[0].texture     = m_drawable.texture;
    rpd.colorAttachments[0].loadAction  = color_load;
    rpd.colorAttachments[0].storeAction = MTLStoreActionStore;
    rpd.colorAttachments[0].clearColor  = MTLClearColorMake(cc.x, cc.y, cc.z, cc.w);

    rpd.depthAttachment.texture     = m_depth_tex;
    rpd.depthAttachment.loadAction  = depth_load;
    rpd.depthAttachment.storeAction = MTLStoreActionDontCare;
    rpd.depthAttachment.clearDepth  = 1.0;

    if (m_depth_fmt == MTLPixelFormatDepth32Float_Stencil8)
    {
        rpd.stencilAttachment.texture      = m_depth_tex;
        rpd.stencilAttachment.loadAction   = depth_load;
        rpd.stencilAttachment.storeAction  = MTLStoreActionDontCare;
        rpd.stencilAttachment.clearStencil = 0;
    }
    return rpd;
}

MTLRenderPassDescriptor* ContextMTL::make_target_rp(Target* t)
{
    MTLRenderPassDescriptor* rpd = [MTLRenderPassDescriptor renderPassDescriptor];
    const Vec4& cc = m_render_state.m_clear_color.m_color;

    MTLLoadAction color_load = m_pending_clear_color ? MTLLoadActionClear : MTLLoadActionLoad;
    MTLLoadAction depth_load = m_pending_clear_depth ? MTLLoadActionClear : MTLLoadActionLoad;
    m_pending_clear_color = false;
    m_pending_clear_depth = false;

    SQMTL_LOG("make_target_rp: target=" << (void*)t
              << " clear_color=" << m_pending_clear_color
              << " clear_depth=" << m_pending_clear_depth);

    // Number of layers for layered rendering (cube faces / array slices). When a
    // target is layered the whole texture is bound (no per-slice binding) and the
    // vertex shader routes each draw to its layer via SV_RenderTargetArrayIndex.
    auto layer_count = [](id<MTLTexture> tex) -> NSUInteger {
        if (!tex) return 1;
        if (tex.textureType == MTLTextureTypeCube)      return 6;
        if (tex.textureType == MTLTextureTypeCubeArray) return 6 * tex.arrayLength;
        if (tex.textureType == MTLTextureType2DArray)   return tex.arrayLength;
        return 1;
    };
    NSUInteger layers = 1;

    int color_idx = 0;
    for (auto& att : t->m_attachments)
    {
        if (!att.texture) continue;
        layers = std::max(layers, layer_count(att.texture->m_texture));
        if (att.type == RT_COLOR)
        {
            MTLRenderPassColorAttachmentDescriptor* ca = rpd.colorAttachments[color_idx++];
            ca.texture     = att.texture->m_texture;
            ca.loadAction  = color_load;
            ca.storeAction = MTLStoreActionStore;
            ca.clearColor  = MTLClearColorMake(cc.x, cc.y, cc.z, cc.w);
        }
        else if (att.type == RT_DEPTH || att.type == RT_DEPTH_STENCIL)
        {
            rpd.depthAttachment.texture     = att.texture->m_texture;
            rpd.depthAttachment.loadAction  = depth_load;
            rpd.depthAttachment.storeAction = MTLStoreActionStore;
            rpd.depthAttachment.clearDepth  = 1.0;
            if (att.type == RT_DEPTH_STENCIL)
            {
                rpd.stencilAttachment.texture     = att.texture->m_texture;
                rpd.stencilAttachment.loadAction  = depth_load;
                rpd.stencilAttachment.storeAction = MTLStoreActionStore;
            }
        }
    }
    //enable layered rendering when the target has more than one layer
    if (layers > 1) rpd.renderTargetArrayLength = layers;
    return rpd;
}

void ContextMTL::ensure_encoder(MTLRenderPassDescriptor* rpd)
{
    if (m_encoder) return;
    ensure_command_buffer();
    if (!rpd)
    {
        // Must use the render target if one is active; otherwise use the
        // default drawable. This is especially important after the lazy
        // enable_render_target() path where the encoder is not created
        // immediately but deferred to the first actual draw call.
        if (m_bind.render_target)
            rpd = make_target_rp(m_bind.render_target);
        else
            rpd = make_default_rp();
    }
    if (!rpd) { SQMTL_LOG("ensure_encoder: rpd is NIL -> no encoder created"); return; }
    m_encoder = [m_cmd_buf renderCommandEncoderWithDescriptor:rpd];
    SQMTL_LOG("ensure_encoder: created encoder (" << (m_bind.render_target ? "TARGET" : "DEFAULT") << ")");
}

void ContextMTL::end_encoder()
{
    if (m_encoder) { [m_encoder endEncoding]; m_encoder = nil; }
}

void ContextMTL::commit_frame()
{
    end_encoder();
    if (m_cmd_buf)
    {
        if (m_drawable) [m_cmd_buf presentDrawable:m_drawable];
        [m_cmd_buf commit];
        m_cmd_buf = nil;
    }
    m_drawable = nil;
    // Drain the frame's autorelease pool: present/commit already retain what the
    // GPU needs, so the autoreleased drawable/command-buffer/encoders can now be
    // released back to the layer's drawable pool.
    if (m_frame_pool) { [m_frame_pool drain]; m_frame_pool = nil; }
    ++m_frame_index;
}

// ──────────────────────────────────────────────────────────────────────────────
// PSO and depth-stencil state cache
// ──────────────────────────────────────────────────────────────────────────────

MTLVertexDescriptor* ContextMTL::build_vertex_desc(InputLayout* il)
{
    if (il->m_vertex_desc) return il->m_vertex_desc;

    MTLVertexDescriptor* vd = [MTLVertexDescriptor vertexDescriptor];

    // SPIRV-Cross emits MSL [[attribute(N)]] where N is the SPIR-V input
    // location, which follows the HLSL struct declaration order — i.e. the
    // attributes in order of increasing byte offset. The engine's AttributeType
    // enum is NOT offset-ordered (TEXCOORD0=2 is declared after TANGENT0=3 /
    // BINORMAL0=4 in the vertex struct), so using the enum value as the Metal
    // attribute index scrambles UV vs tangent/binormal. DX11 dodges this by
    // matching on semantic name; for Metal we must assign the attribute index
    // by offset rank so it lines up with the shader's [[attribute(N)]].
    std::vector<const Attribute*> sorted;
    sorted.reserve(il->m_list.count_attribute());
    for (const auto& attr : il->m_list) sorted.push_back(&attr);
    std::sort(sorted.begin(), sorted.end(),
              [](const Attribute* a, const Attribute* b){ return a->m_offset < b->m_offset; });

    NSUInteger loc = 0;
    for (const Attribute* attr : sorted)
    {
        vd.attributes[loc].format      = to_mtl_vertex_format(attr->m_strip);
        vd.attributes[loc].offset      = attr->m_offset;
        vd.attributes[loc].bufferIndex = VERTEX_BUFFER_BINDING;
        SQMTL_LOG("  build_vertex_desc loc=" << loc
                  << " (enum=" << (int)attr->m_attribute << ")"
                  << " strip=" << (int)attr->m_strip
                  << " offset=" << attr->m_offset);
        ++loc;
    }
    SQMTL_LOG("build_vertex_desc: n_attr=" << il->m_list.count_attribute()
              << " stride(bytes)=" << il->m_list.size());
    vd.layouts[VERTEX_BUFFER_BINDING].stride       = il->m_list.size();
    vd.layouts[VERTEX_BUFFER_BINDING].stepFunction = MTLVertexStepFunctionPerVertex;
    vd.layouts[VERTEX_BUFFER_BINDING].stepRate     = 1;
    // [MTLVertexDescriptor vertexDescriptor] is autoreleased; retain it so the
    // cache survives the per-frame autorelease pool drain (released in delete_IL).
    il->m_vertex_desc = [vd retain];
    return vd;
}

id<MTLRenderPipelineState> ContextMTL::get_or_create_pso()
{
    Shader* sh = m_bind.shader;
    if (!sh) return nil;

    PSOKey key;
    key.shader       = sh;
    key.input_layout = m_bind.input_layout;
    key.blend        = m_render_state.m_blend;
    key.color_fmt    = m_bind.render_target ? m_bind.render_target->m_color_fmt : m_color_fmt;
    key.depth_fmt    = m_bind.render_target ? m_bind.render_target->m_depth_fmt : m_depth_fmt;

    auto it = m_pso_cache.find(key);
    if (it != m_pso_cache.end()) return it->second;

    SQMTL_LOG("get_or_create_pso: cache MISS, building."
              << " has_VS=" << (sh->m_functions[ST_VERTEX_SHADER] != nil)
              << " has_FS=" << (sh->m_functions[ST_FRAGMENT_SHADER] != nil)
              << " has_IL=" << (m_bind.input_layout != nullptr)
              << " color_fmt=" << (int)key.color_fmt
              << " depth_fmt=" << (int)key.depth_fmt);

    MTLRenderPipelineDescriptor* pd = [[MTLRenderPipelineDescriptor alloc] init];

    if (sh->m_functions[ST_VERTEX_SHADER])
        pd.vertexFunction   = sh->m_functions[ST_VERTEX_SHADER];
    if (sh->m_functions[ST_FRAGMENT_SHADER])
        pd.fragmentFunction = sh->m_functions[ST_FRAGMENT_SHADER];

    if (m_bind.input_layout)
        pd.vertexDescriptor = build_vertex_desc(m_bind.input_layout);

    // Layered rendering: a VS that writes [[render_target_array_index]] requires
    // the primitive topology class to be declared. Shadow casters are triangles.
    if (sh->m_writes_layer)
        pd.inputPrimitiveTopology = MTLPrimitiveTopologyClassTriangle;

    pd.colorAttachments[0].pixelFormat = key.color_fmt;

    const BlendState& bs = m_render_state.m_blend;
    if (bs.m_enable)
    {
        pd.colorAttachments[0].blendingEnabled             = YES;
        pd.colorAttachments[0].sourceRGBBlendFactor        = to_mtl_blend(bs.m_src);
        pd.colorAttachments[0].destinationRGBBlendFactor   = to_mtl_blend(bs.m_dst);
        pd.colorAttachments[0].sourceAlphaBlendFactor      = to_mtl_blend(bs.m_src);
        pd.colorAttachments[0].destinationAlphaBlendFactor = to_mtl_blend(bs.m_dst);
    }

    if (key.depth_fmt != MTLPixelFormatInvalid)
        pd.depthAttachmentPixelFormat = key.depth_fmt;
    if (key.depth_fmt == MTLPixelFormatDepth32Float_Stencil8)
        pd.stencilAttachmentPixelFormat = key.depth_fmt;

    NSError* err = nil;
    id<MTLRenderPipelineState> pso = [m_device newRenderPipelineStateWithDescriptor:pd error:&err];
    if (!pso && logger())
        logger()->warning(std::string("Metal PSO creation failed: ") + err.localizedDescription.UTF8String);
    else
        SQMTL_LOG("get_or_create_pso: PSO built OK");

    m_pso_cache[key] = pso;
    return pso;
}

id<MTLDepthStencilState> ContextMTL::get_or_create_ds()
{
    const DepthBufferState& dbs = m_render_state.m_depth;
    uint32_t key = (uint32_t)dbs.m_mode << 16 | (uint32_t)dbs.m_type;

    auto it = m_ds_cache.find(key);
    if (it != m_ds_cache.end()) return it->second;

    MTLDepthStencilDescriptor* dd = [[MTLDepthStencilDescriptor alloc] init];
    if (dbs.m_mode == DM_DISABLE)
    {
        dd.depthCompareFunction = MTLCompareFunctionAlways;
        dd.depthWriteEnabled    = NO;
    }
    else
    {
        dd.depthCompareFunction = to_mtl_depth_func(dbs.m_type);
        dd.depthWriteEnabled    = (dbs.m_mode == DM_ENABLE_AND_WRITE) ? YES : NO;
    }

    id<MTLDepthStencilState> ds = [m_device newDepthStencilStateWithDescriptor:dd];
    m_ds_cache[key] = ds;
    return ds;
}

void ContextMTL::apply_state_to_encoder()
{
    if (!m_encoder) return;

    const Vec4& vp = m_render_state.m_viewport.m_viewport;
    MTLViewport mtlvp;
    mtlvp.originX = vp.x;
    mtlvp.originY = vp.y;
    mtlvp.width   = vp.z;
    mtlvp.height  = vp.w;
    mtlvp.znear   = 0.0;
    mtlvp.zfar    = 1.0;
    [m_encoder setViewport:mtlvp];

    [m_encoder setFrontFacingWinding:MTLWindingClockwise]; // match DX11/HLSL CW convention
    [m_encoder setCullMode:to_mtl_cull(m_render_state.m_cullface.m_cullface)];
    [m_encoder setDepthStencilState:get_or_create_ds()];

    id<MTLRenderPipelineState> pso = get_or_create_pso();
    if (pso) [m_encoder setRenderPipelineState:pso];
}

// ──────────────────────────────────────────────────────────────────────────────
// ContextMTL — init / close
// ──────────────────────────────────────────────────────────────────────────────

bool ContextMTL::init(Video::DeviceResources* resource)
{
    m_device = (__bridge id<MTLDevice>)resource->get_device();
    m_layer  = (__bridge CAMetalLayer*)resource->get_device_context();

    if (!m_device)
    {
        if (logger()) logger()->error("Metal: MTLDevice is null — the window was not created with CTX_METAL context type.");
        return false;
    }
    if (!m_layer)
    {
        if (logger()) logger()->error("Metal: CAMetalLayer is null — DeviceResources did not provide a valid Metal layer.");
        return false;
    }

    m_srgb_fb   = resource->get_context_info().m_srgb;
    m_color_fmt = m_srgb_fb ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;
    m_depth_fmt = MTLPixelFormatDepth32Float_Stencil8;

    m_queue = [m_device newCommandQueue];
    if (!m_queue)
    {
        if (logger()) logger()->error("Metal: failed to create MTLCommandQueue.");
        return false;
    }

    resource->callback_target_changed([this](Video::DeviceResources*)
    {
        this->commit_frame();
    });

    m_driver_info.m_render_driver   = DR_METAL;
    m_driver_info.m_name            = "Metal";
    m_driver_info.m_major_version   = 3;
    m_driver_info.m_minor_version   = 0;
    m_driver_info.m_shader_language = "MSL";
    m_driver_info.m_shader_version  = 20000;
    m_driver_info.m_geometry_shader = false; // Metal has no geometry shaders → multipass shadows

    return true;
}

void ContextMTL::close()
{
    end_encoder();
    if (m_cmd_buf) { [m_cmd_buf commit]; m_cmd_buf = nil; }
    m_drawable  = nil;
    m_depth_tex = nil;
    m_queue     = nil;
    for (auto& p : m_pso_cache) p.second = nil;
    m_pso_cache.clear();
    for (auto& p : m_ds_cache) p.second = nil;
    m_ds_cache.clear();
}

// ──────────────────────────────────────────────────────────────────────────────
// State queries / setters
// ──────────────────────────────────────────────────────────────────────────────

RenderDriver ContextMTL::get_render_driver() { return DR_METAL; }
const RenderDriverInfo& ContextMTL::get_render_driver_info() const { return m_driver_info; }
bool ContextMTL::is_srgb_framebuffer() const { return m_srgb_fb; }

void ContextMTL::print_info()
{
    if (logger()) logger()->info(std::string("Metal device: ") + m_device.name.UTF8String);
}

const ClearColorState& ContextMTL::get_clear_color_state() { return m_render_state.m_clear_color; }
void ContextMTL::set_clear_color_state(const ClearColorState& s) { m_render_state.m_clear_color = s; }

void ContextMTL::clear(int type)
{
    // In Metal, clearing is done via the render-pass load action, not a draw call.
    // End the current encoder so the NEXT draw starts a fresh pass, and set the
    // appropriate pending-clear flags so make_default_rp / make_target_rp use
    // MTLLoadActionClear instead of MTLLoadActionLoad.
    end_encoder();
    if (type & CLEAR_COLOR) m_pending_clear_color = true;
    if (type & CLEAR_DEPTH) m_pending_clear_depth = true;
}

const DepthBufferState& ContextMTL::get_depth_buffer_state() { return m_render_state.m_depth; }
void ContextMTL::set_depth_buffer_state(const DepthBufferState& s) { m_render_state.m_depth = s; }

const CullfaceState& ContextMTL::get_cullface_state() { return m_render_state.m_cullface; }
void ContextMTL::set_cullface_state(const CullfaceState& s) { m_render_state.m_cullface = s; }

const ViewportState& ContextMTL::get_viewport_state() { return m_render_state.m_viewport; }
void ContextMTL::set_viewport_state(const ViewportState& s) { m_render_state.m_viewport = s; }

const BlendState& ContextMTL::get_blend_state() { return m_render_state.m_blend; }
void ContextMTL::set_blend_state(const BlendState& s) { m_render_state.m_blend = s; }

const State& ContextMTL::get_render_state() { return m_render_state; }
void ContextMTL::set_render_state(const State& s) { m_render_state = s; }

// ──────────────────────────────────────────────────────────────────────────────
// Buffer management
// ──────────────────────────────────────────────────────────────────────────────

static id<MTLBuffer> create_mtl_buf(id<MTLDevice> dev, const void* data, size_t size)
{
    if (data)
        return [dev newBufferWithBytes:data length:size options:MTLResourceStorageModeShared];
    return [dev newBufferWithLength:size options:MTLResourceStorageModeShared];
}

ConstBuffer* ContextMTL::create_stream_CB(const unsigned char* data, size_t size)
{
    auto* cb = new ConstBuffer();
    cb->m_buffer = create_mtl_buf(m_device, data, size);
    cb->m_size   = size;
    return cb;
}

VertexBuffer* ContextMTL::create_stream_VBO(const unsigned char* data, size_t stride, size_t n)
{
    auto* vb = new VertexBuffer();
    vb->m_stride = stride;
    vb->m_count  = n;
    vb->m_size   = stride * n;
    vb->m_buffer = create_mtl_buf(m_device, data, vb->m_size);
    return vb;
}

IndexBuffer* ContextMTL::create_stream_IBO(const unsigned int* data, size_t n)
{
    auto* ib = new IndexBuffer();
    ib->m_size   = sizeof(unsigned int) * n;
    ib->m_buffer = create_mtl_buf(m_device, data, ib->m_size);
    return ib;
}

ConstBuffer*  ContextMTL::create_CB(const unsigned char* data, size_t size) { return create_stream_CB(data, size); }
VertexBuffer* ContextMTL::create_VBO(const unsigned char* data, size_t stride, size_t n) { return create_stream_VBO(data, stride, n); }
IndexBuffer*  ContextMTL::create_IBO(const unsigned int* data, size_t n)  { return create_stream_IBO(data, n); }

uint64 ContextMTL::get_native_CB(const ConstBuffer* cb)   const { return cb ? (uint64)(uintptr_t)((__bridge void*)cb->m_buffer) : 0; }
uint64 ContextMTL::get_native_VBO(const VertexBuffer* vb) const { return vb ? (uint64)(uintptr_t)((__bridge void*)vb->m_buffer) : 0; }
uint64 ContextMTL::get_native_IBO(const IndexBuffer* ib)  const { return ib ? (uint64)(uintptr_t)((__bridge void*)ib->m_buffer) : 0; }

size_t ContextMTL::get_size_CB(const ConstBuffer* cb)   const { return cb ? cb->m_size : 0; }
size_t ContextMTL::get_size_VBO(const VertexBuffer* vb) const { return vb ? vb->m_size : 0; }
size_t ContextMTL::get_size_IBO(const IndexBuffer* ib)  const { return ib ? ib->m_size : 0; }

void ContextMTL::update_steam_CB(ConstBuffer* cb, const unsigned char* data, size_t n)
{
    if (cb && cb->m_buffer && data)
        std::memcpy(cb->m_buffer.contents, data, std::min(n, cb->m_size));
}

void ContextMTL::update_steam_VBO(VertexBuffer* vb, const unsigned char* data, size_t n)
{
    if (vb && vb->m_buffer && data)
        std::memcpy(vb->m_buffer.contents, data, std::min(n, vb->m_size));
}

void ContextMTL::update_steam_IBO(IndexBuffer* ib, const unsigned int* data, size_t n)
{
    if (ib && ib->m_buffer && data)
        std::memcpy(ib->m_buffer.contents, data, std::min(n * sizeof(unsigned int), ib->m_size));
}

void ContextMTL::bind_CB(ConstBuffer*) {}
void ContextMTL::bind_VBO(VertexBuffer* vb) { m_bind.vertex_buffer = vb; }
void ContextMTL::bind_IBO(IndexBuffer*  ib) { m_bind.index_buffer  = ib; }
void ContextMTL::unbind_CB(ConstBuffer*) {}
void ContextMTL::unbind_VBO(VertexBuffer*) { m_bind.vertex_buffer = nullptr; }
void ContextMTL::unbind_IBO(IndexBuffer*)  { m_bind.index_buffer  = nullptr; }

std::vector<unsigned char> ContextMTL::copy_buffer_CB(const ConstBuffer* cb)
{
    if (!cb || !cb->m_buffer) return {};
    auto* p = (const unsigned char*)cb->m_buffer.contents;
    return {p, p + cb->m_size};
}

std::vector<unsigned char> ContextMTL::copy_buffer_VBO(const VertexBuffer* vb)
{
    if (!vb || !vb->m_buffer) return {};
    auto* p = (const unsigned char*)vb->m_buffer.contents;
    return {p, p + vb->m_size};
}

std::vector<unsigned char> ContextMTL::copy_buffer_IBO(const IndexBuffer* ib)
{
    if (!ib || !ib->m_buffer) return {};
    auto* p = (const unsigned char*)ib->m_buffer.contents;
    return {p, p + ib->m_size};
}

unsigned char* ContextMTL::map_CB(ConstBuffer* cb, size_t start, size_t, MappingType)
{
    return (cb && cb->m_buffer) ? (unsigned char*)cb->m_buffer.contents + start : nullptr;
}
unsigned char* ContextMTL::map_CB(ConstBuffer* cb, MappingType t)
{
    return map_CB(cb, 0, cb ? cb->m_size : 0, t);
}
void ContextMTL::unmap_CB(ConstBuffer*) {}

unsigned char* ContextMTL::map_VBO(VertexBuffer* vb, size_t start, size_t, MappingType)
{
    return (vb && vb->m_buffer) ? (unsigned char*)vb->m_buffer.contents + start : nullptr;
}
unsigned char* ContextMTL::map_VBO(VertexBuffer* vb, MappingType t)
{
    return map_VBO(vb, 0, vb ? vb->m_size : 0, t);
}
void ContextMTL::unmap_VBO(VertexBuffer*) {}

unsigned int* ContextMTL::map_IBO(IndexBuffer* ib, size_t start, size_t, MappingType)
{
    return (ib && ib->m_buffer)
        ? (unsigned int*)((unsigned char*)ib->m_buffer.contents + start * sizeof(unsigned int))
        : nullptr;
}
unsigned int* ContextMTL::map_IBO(IndexBuffer* ib, MappingType t)
{
    return map_IBO(ib, 0, ib ? ib->m_size / sizeof(unsigned int) : 0, t);
}
void ContextMTL::unmap_IBO(IndexBuffer*) {}

unsigned char* ContextMTL::map_TBO(Texture*, MappingType) { return nullptr; }
void ContextMTL::unmap_TBO(Texture*) {}

void ContextMTL::delete_CB(ConstBuffer*& cb)   { delete cb; cb = nullptr; }
void ContextMTL::delete_VBO(VertexBuffer*& vb) { delete vb; vb = nullptr; }
void ContextMTL::delete_IBO(IndexBuffer*& ib)  { delete ib; ib = nullptr; }

// ──────────────────────────────────────────────────────────────────────────────
// Input Layout
// ──────────────────────────────────────────────────────────────────────────────

InputLayout* ContextMTL::create_IL(const AttributeList& atl)
{
    auto* il   = new InputLayout();
    il->m_list = atl;
    return il;
}

void ContextMTL::delete_IL(InputLayout*& il)     { if (il) { [il->m_vertex_desc release]; } delete il; il = nullptr; }
void ContextMTL::bind_IL(InputLayout* il)         { m_bind.input_layout = il; }
void ContextMTL::unbind_IL(InputLayout*)          { m_bind.input_layout = nullptr; }
size_t ContextMTL::size_IL(const InputLayout* il) { return il ? il->m_list.size() : 0; }

bool ContextMTL::has_a_position_IL(const InputLayout* il)
{
    if (!il) return false;
    for (const auto& a : il->m_list) if (a.is_position_transform()) return true;
    return false;
}

size_t ContextMTL::position_offset_IL(const InputLayout* il)
{
    if (!il) return 0;
    for (const auto& a : il->m_list) if (a.is_position_transform()) return a.m_offset;
    return 0;
}

// ──────────────────────────────────────────────────────────────────────────────
// Texture
// ──────────────────────────────────────────────────────────────────────────────

static id<MTLTexture> upload_texture(id<MTLDevice> dev,
                                     const TextureRawDataInformation& raw,
                                     const TextureGpuDataInformation& gpu,
                                     int slices = 1, bool cube = false)
{
    MTLPixelFormat fmt = to_mtl_pixel_format(raw.m_format, raw.m_is_srgb);
    MTLTextureDescriptor* td;

    if (cube)
    {
        td = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:fmt
                                                                   size:raw.m_width
                                                              mipmapped:gpu.m_build_mipmap ? YES : NO];
    }
    else if (slices > 1)
    {
        td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:fmt
                                                                width:raw.m_width
                                                               height:raw.m_height
                                                            mipmapped:gpu.m_build_mipmap ? YES : NO];
        td.textureType = MTLTextureType2DArray;
        td.arrayLength = slices;
    }
    else
    {
        td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:fmt
                                                                width:raw.m_width
                                                               height:raw.m_height
                                                            mipmapped:gpu.m_build_mipmap ? YES : NO];
    }

    // replaceRegion from the CPU requires Shared (or Managed) storage.
    // Private textures can only be written via a blit encoder from the GPU.
    // Use Shared whenever we have initial data to upload; otherwise respect
    // the caller's preference (Private for GPU-only render targets, etc.).
    if (raw.m_bytes)
        td.storageMode = MTLStorageModeShared;
    else
        td.storageMode = gpu.m_read_from_cpu ? MTLStorageModeShared : MTLStorageModePrivate;

    td.usage = MTLTextureUsageShaderRead;
    if (is_depth_format(raw.m_format)) td.usage |= MTLTextureUsageRenderTarget;
    // generateMipmapsForTexture renders into the mip levels, so the texture must
    // also be usable as a render target.
    if (gpu.m_build_mipmap) td.usage |= MTLTextureUsageRenderTarget;

    id<MTLTexture> tex = [dev newTextureWithDescriptor:td];
    if (!tex || !raw.m_bytes) return tex;

    // Bytes per component
    size_t bpc = 1;
    switch (raw.m_type_format)
    {
    case TTF_FLOAT:          bpc = 4; break;
    case TTF_UNSIGNED_SHORT: bpc = 2; break;
    default:                 bpc = 1; break;
    }

    // Channels actually present in the raw CPU buffer
    size_t src_ch;
    switch (raw.m_type)
    {
    case TT_R:   src_ch = 1; break;
    case TT_RG:  src_ch = 2; break;
    case TT_RGB: src_ch = 3; break;
    default:     src_ch = 4; break;
    }

    // Channels required by the Metal pixel format.
    // Metal has no RGB8 format — TF_RGB8 maps to RGBA8 (4 channels).
    size_t dst_ch;
    switch (raw.m_type)
    {
    case TT_R:  dst_ch = 1; break;
    case TT_RG: dst_ch = 2; break;
    default:    dst_ch = 4; break;
    }

    size_t bpr  = raw.m_width * dst_ch * bpc;
    size_t face = bpr * raw.m_height;

    // If the raw data has fewer channels than Metal expects (e.g. RGB → RGBA8),
    // expand the data before upload. Alpha is filled with 0xFF.
    std::vector<unsigned char> expanded_buf;
    const unsigned char* upload_ptr = raw.m_bytes;
    if (src_ch != dst_ch)
    {
        size_t n_faces = cube ? 6 : std::max(1, slices);
        size_t n_pix   = (size_t)raw.m_width * raw.m_height * n_faces;
        expanded_buf.assign(n_pix * dst_ch * bpc, static_cast<unsigned char>(0xFF));
        const unsigned char* s = raw.m_bytes;
        unsigned char*       d = expanded_buf.data();
        size_t src_stride = src_ch * bpc;
        size_t dst_stride = dst_ch * bpc;
        for (size_t i = 0; i < n_pix; ++i)
            std::memcpy(d + i * dst_stride, s + i * src_stride, src_stride);
        upload_ptr = expanded_buf.data();
    }

    if (cube)
    {
        for (int f = 0; f < 6; ++f)
        {
            [tex replaceRegion:MTLRegionMake2D(0, 0, raw.m_width, raw.m_height)
                   mipmapLevel:0 slice:f
                     withBytes:upload_ptr + f * face
                   bytesPerRow:bpr bytesPerImage:face];
        }
    }
    else if (slices > 1)
    {
        for (int s = 0; s < slices; ++s)
        {
            [tex replaceRegion:MTLRegionMake2D(0, 0, raw.m_width, raw.m_height)
                   mipmapLevel:0 slice:s
                     withBytes:upload_ptr + s * face
                   bytesPerRow:bpr bytesPerImage:face];
        }
    }
    else
    {
        [tex replaceRegion:MTLRegionMake2D(0, 0, raw.m_width, raw.m_height)
               mipmapLevel:0
                 withBytes:upload_ptr
               bytesPerRow:bpr];
    }
    return tex;
}

static id<MTLSamplerState> create_sampler(id<MTLDevice> dev, const TextureGpuDataInformation& gpu)
{
    MTLSamplerDescriptor* sd = [[MTLSamplerDescriptor alloc] init];
    sd.minFilter     = to_mtl_min(gpu.m_min_type);
    sd.magFilter     = to_mtl_mag(gpu.m_mag_type);
    sd.mipFilter     = to_mtl_mip(gpu.m_min_type);
    sd.sAddressMode  = to_mtl_edge(gpu.m_edge_s);
    sd.tAddressMode  = to_mtl_edge(gpu.m_edge_t);
    sd.rAddressMode  = to_mtl_edge(gpu.m_edge_r);
    sd.maxAnisotropy = std::max(1, gpu.m_anisotropy);
    return [dev newSamplerStateWithDescriptor:sd];
}

// Metal does not auto-generate mipmaps: only level 0 is uploaded, so the higher
// levels stay uninitialised (black) unless we fill them with a blit. Without this
// distant samples read black mips → dark surfaces + broken alpha-test at range.
void ContextMTL::generate_mipmaps(id<MTLTexture> tex)
{
    if (!tex || tex.mipmapLevelCount <= 1) return;
    id<MTLCommandBuffer>      cb   = [m_queue commandBuffer];
    id<MTLBlitCommandEncoder> blit = [cb blitCommandEncoder];
    [blit generateMipmapsForTexture:tex];
    [blit endEncoding];
    [cb commit];
    [cb waitUntilCompleted];
}

Texture* ContextMTL::create_texture(const TextureRawDataInformation& raw, const TextureGpuDataInformation& gpu)
{
    auto* t = new Texture();
    t->m_texture = upload_texture(m_device, raw, gpu);
    if (gpu.m_build_mipmap) generate_mipmaps(t->m_texture);
    t->m_sampler = create_sampler(m_device, gpu);
    return t;
}

Texture* ContextMTL::create_texture_array(const TextureRawDataInformation& raw, const TextureGpuDataInformation& gpu, int n)
{
    auto* t = new Texture();
    t->m_texture = upload_texture(m_device, raw, gpu, n);
    if (gpu.m_build_mipmap) generate_mipmaps(t->m_texture);
    t->m_sampler = create_sampler(m_device, gpu);
    return t;
}

Texture* ContextMTL::create_cube_texture(const TextureRawDataInformation data[6], const TextureGpuDataInformation& gpu)
{
    auto* t = new Texture();
    t->m_texture = upload_texture(m_device, data[0], gpu, 1, true);
    if (gpu.m_build_mipmap) generate_mipmaps(t->m_texture);
    t->m_sampler = create_sampler(m_device, gpu);
    return t;
}

std::vector<unsigned char> ContextMTL::get_texture(Texture* t, int level)
{
    if (!t || !t->m_texture) return {};
    NSUInteger w = std::max<NSUInteger>(1, t->m_texture.width  >> level);
    NSUInteger h = std::max<NSUInteger>(1, t->m_texture.height >> level);
    size_t bpr = w * 4;
    std::vector<unsigned char> buf(bpr * h);
    [t->m_texture getBytes:buf.data() bytesPerRow:bpr
                fromRegion:MTLRegionMake2D(0,0,w,h) mipmapLevel:level];
    return buf;
}

std::vector<unsigned char> ContextMTL::get_texture(Texture* t, int face, int level)
{
    if (!t || !t->m_texture) return {};
    NSUInteger w = std::max<NSUInteger>(1, t->m_texture.width  >> level);
    NSUInteger h = std::max<NSUInteger>(1, t->m_texture.height >> level);
    size_t bpr = w * 4;
    std::vector<unsigned char> buf(bpr * h);
    [t->m_texture getBytes:buf.data() bytesPerRow:bpr bytesPerImage:bpr * h
                fromRegion:MTLRegionMake2D(0,0,w,h) mipmapLevel:level slice:face];
    return buf;
}

void ContextMTL::bind_texture(Texture* t, int texture_id, int /*sample_id*/)
{
    if (!t || texture_id < 0 || texture_id >= Shader::MAX_TEXTURES) return;
    if (m_bind.shader) m_bind.shader->m_bound_textures[texture_id] = t;
}

void ContextMTL::unbind_texture(Texture* t)
{
    if (!t || !m_bind.shader) return;
    for (int i = 0; i < Shader::MAX_TEXTURES; ++i)
        if (m_bind.shader->m_bound_textures[i] == t)
            m_bind.shader->m_bound_textures[i] = nullptr;
}

void ContextMTL::unbind_texture(int n)
{
    if (m_bind.shader && n >= 0 && n < Shader::MAX_TEXTURES)
        m_bind.shader->m_bound_textures[n] = nullptr;
}

void ContextMTL::delete_texture(Texture*& t) { delete t; t = nullptr; }

// ──────────────────────────────────────────────────────────────────────────────
// Shader
// ──────────────────────────────────────────────────────────────────────────────

Shader* ContextMTL::create_shader(const std::vector<ShaderSourceInformation>& infos)
{
    auto* sh = new Shader(*this);

    for (const auto& info : infos)
    {
        int stage = (int)info.m_type;
        if (stage < 0 || stage >= ST_N_SHADER) continue;
        if (info.m_type == ST_GEOMETRY_SHADER
         || info.m_type == ST_TASSELLATION_CONTROL_SHADER
         || info.m_type == ST_TASSELLATION_EVALUATION_SHADER
         || info.m_type == ST_COMPUTE_SHADER) continue;

        const std::string src = info.m_shader_header.empty()
                              ? info.m_shader_source
                              : info.m_shader_header + "\n" + info.m_shader_source;

        sh->m_refl[stage] = parse_reflection(src);

        // Layered rendering: the VS emits [[render_target_array_index]] → the PSO
        // must declare a primitive topology class (set in get_or_create_pso).
        if (stage == ST_VERTEX_SHADER && src.find("render_target_array_index") != std::string::npos)
            sh->m_writes_layer = true;

        // Build auto CPU buffer
        uint32_t auto_sz = 0;
        for (auto& m : sh->m_refl[stage].members)
            auto_sz = std::max(auto_sz, m.second.offset + m.second.size);
        if (auto_sz)
        {
            sh->m_auto_cpu[stage].resize(auto_sz, 0);
            sh->m_auto_gpu[stage] = [m_device newBufferWithLength:auto_sz
                                                          options:MTLResourceStorageModeShared];
        }

        NSString* msl   = [NSString stringWithUTF8String:src.c_str()];
        MTLCompileOptions* opts = [[MTLCompileOptions alloc] init];
        opts.languageVersion    = MTLLanguageVersion2_0;

        NSError* err = nil;
        id<MTLLibrary> lib = [m_device newLibraryWithSource:msl options:opts error:&err];
        if (!lib)
        {
            if (err) sh->m_compiler_errors.push_back(err.localizedDescription.UTF8String);
            continue;
        }

        const std::string& entry = sh->m_refl[stage].entry_point.empty()
                                 ? info.m_entry_point : sh->m_refl[stage].entry_point;
        sh->m_functions[stage] = [lib newFunctionWithName:[NSString stringWithUTF8String:entry.c_str()]];
        if (!sh->m_functions[stage] && logger())
            logger()->warning(std::string("Metal: function '") + entry + "' not found in shader library.");
    }

    return sh;
}

bool ContextMTL::shader_compiled_with_errors(Shader* s)
{
    return s && !s->m_compiler_errors.empty();
}

bool ContextMTL::shader_linked_with_error(Shader* s)
{
    return s && !s->m_linker_error.empty();
}

std::vector<std::string> ContextMTL::get_shader_compiler_errors(Shader* s)
{
    return s ? s->m_compiler_errors : std::vector<std::string>{};
}

std::string ContextMTL::get_shader_linker_error(Shader* s)
{
    return s ? s->m_linker_error : std::string{};
}

void ContextMTL::bind_shader(Shader* s)   { m_bind.shader = s; }
void ContextMTL::unbind_shader(Shader*)   { m_bind.shader = nullptr; }

void ContextMTL::bind_uniform_CB(const ConstBuffer* cb, Shader* s, const std::string& uname)
{
    // get_uniform_const_buffer returns a shader-owned handle — do NOT delete it.
    auto* ucb = static_cast<UniformConstBufferMTL*>(get_uniform_const_buffer(s, uname));
    if (ucb) ucb->bind(cb);
}

void ContextMTL::unbind_uniform_CB(Shader* s, const std::string& uname)
{
    auto* ucb = static_cast<UniformConstBufferMTL*>(get_uniform_const_buffer(s, uname));
    if (ucb) ucb->unbind();
}

void ContextMTL::delete_shader(Shader*& s) { delete s; s = nullptr; }
Shader* ContextMTL::get_bind_shader() const { return m_bind.shader; }

Uniform* ContextMTL::get_uniform(Shader* s, const std::string& name) const
{
    if (!s) return nullptr;

    // Return the cached handle if we already created one (lifetime == shader).
    auto cached = s->m_uniform_cache.find(name);
    if (cached != s->m_uniform_cache.end()) return cached->second.get();

    std::unique_ptr<Uniform> u;

    for (int stage = 0; stage < ST_N_SHADER && !u; ++stage)
    {
        if (!s->m_functions[stage]) continue;
        auto it = s->m_refl[stage].textures.find(name);
        if (it != s->m_refl[stage].textures.end())
            u.reset(new UniformTextureMTL(const_cast<ContextMTL*>(this), s, it->second));
    }

    for (int stage = 0; stage < ST_N_SHADER && !u; ++stage)
    {
        if (!s->m_functions[stage]) continue;
        auto it = s->m_refl[stage].members.find(name);
        if (it != s->m_refl[stage].members.end())
        {
            const auto& m = it->second;
            u.reset(new UniformMemberMTL(const_cast<ContextMTL*>(this), s, m.buffer_slot, m.offset, m.size));
        }
    }

    if (!u) return nullptr;
    Uniform* raw = u.get();
    s->m_uniform_cache.emplace(name, std::move(u));
    return raw;
}

UniformConstBuffer* ContextMTL::get_uniform_const_buffer(Shader* s, const std::string& name) const
{
    if (!s) return nullptr;

    // Return the cached handle if we already created one (lifetime == shader).
    auto cached = s->m_ucb_cache.find(name);
    if (cached != s->m_ucb_cache.end()) return cached->second.get();

    // Collect {stage, slot} for every stage that declares this buffer.
    // SPIRV-Cross assigns slots independently per stage, so Camera may be
    // at [[buffer(0)]] in the VS but [[buffer(1)]] in the FS.
    std::vector<UniformConstBufferMTL::StageSlot> bindings;
    for (int stage = 0; stage < ST_N_SHADER; ++stage)
    {
        if (!s->m_functions[stage]) continue;
        auto it = s->m_refl[stage].buffers.find(name);
        if (it != s->m_refl[stage].buffers.end() && it->second < Shader::MAX_BUFFERS)
            bindings.push_back({stage, it->second});
    }

    if (bindings.empty()) return nullptr;
    auto ucb = std::unique_ptr<UniformConstBuffer>(
        new UniformConstBufferMTL(const_cast<ContextMTL*>(this), s, std::move(bindings)));
    UniformConstBuffer* raw = ucb.get();
    s->m_ucb_cache.emplace(name, std::move(ucb));
    return raw;
}

// ──────────────────────────────────────────────────────────────────────────────
// Draw calls
// ──────────────────────────────────────────────────────────────────────────────

// Metal's setBytes copies data inline into the command buffer, so each draw
// gets its OWN snapshot of the constant data — matching DX11/GL, where a
// const-buffer update between draws is ordered in the command stream. Binding
// the shared MTLBuffer directly would instead let every queued draw read the
// LAST CPU write (Shared storage is not synchronised with GPU execution), which
// collapses per-object Transform / per-material data onto the final value and
// makes complex scenes jitter and explode. setBytes is capped at 4 KB; anything
// larger falls back to a plain buffer bind.
static constexpr NSUInteger MTL_SET_BYTES_MAX = 4096;

static void flush_auto_buffers(id<MTLRenderCommandEncoder> enc, Shader* sh)
{
    for (int stage = 0; stage < ST_N_SHADER; ++stage)
    {
        if (!sh->m_functions[stage] || sh->m_auto_cpu[stage].empty()) continue;
        int auto_slot = -1;
        for (auto& m : sh->m_refl[stage].members) { auto_slot = m.second.buffer_slot; break; }
        if (auto_slot < 0) continue;

        auto& cpu = sh->m_auto_cpu[stage];
        if (cpu.size() <= MTL_SET_BYTES_MAX)
        {
            if (stage == ST_VERTEX_SHADER)
                [enc setVertexBytes:cpu.data() length:cpu.size() atIndex:auto_slot];
            else if (stage == ST_FRAGMENT_SHADER)
                [enc setFragmentBytes:cpu.data() length:cpu.size() atIndex:auto_slot];
        }
        else
        {
            if (sh->m_auto_dirty[stage])
            {
                std::memcpy(sh->m_auto_gpu[stage].contents, cpu.data(), cpu.size());
                sh->m_auto_dirty[stage] = false;
            }
            if (stage == ST_VERTEX_SHADER)
                [enc setVertexBuffer:sh->m_auto_gpu[stage] offset:0 atIndex:auto_slot];
            else if (stage == ST_FRAGMENT_SHADER)
                [enc setFragmentBuffer:sh->m_auto_gpu[stage] offset:0 atIndex:auto_slot];
        }
    }
}

static void flush_bound_cbs(id<MTLRenderCommandEncoder> enc, Shader* sh)
{
    for (int slot = 0; slot < Shader::MAX_BUFFERS; ++slot)
    {
        if (const ConstBuffer* cb = sh->m_bound_cb_per_stage[ST_VERTEX_SHADER][slot])
            if (cb->m_buffer && cb->m_size)
            {
                if (cb->m_size <= MTL_SET_BYTES_MAX)
                    [enc setVertexBytes:cb->m_buffer.contents length:cb->m_size atIndex:slot];
                else
                    [enc setVertexBuffer:cb->m_buffer offset:0 atIndex:slot];
            }

        if (const ConstBuffer* cb = sh->m_bound_cb_per_stage[ST_FRAGMENT_SHADER][slot])
            if (cb->m_buffer && cb->m_size)
            {
                if (cb->m_size <= MTL_SET_BYTES_MAX)
                    [enc setFragmentBytes:cb->m_buffer.contents length:cb->m_size atIndex:slot];
                else
                    [enc setFragmentBuffer:cb->m_buffer offset:0 atIndex:slot];
            }
    }
}

static void flush_textures(id<MTLRenderCommandEncoder> enc, Shader* sh)
{
    for (int stage = 0; stage < ST_N_SHADER; ++stage)
    {
        if (!sh->m_functions[stage]) continue;
        for (auto& t : sh->m_refl[stage].textures)
        {
            int slot = t.second;
            if (slot < 0 || slot >= Shader::MAX_TEXTURES) continue;
            Texture* tex = sh->m_bound_textures[slot];
            if (!tex) continue;

            if (stage == ST_VERTEX_SHADER)
            {
                [enc setVertexTexture:tex->m_texture atIndex:slot];
                [enc setVertexSamplerState:tex->m_sampler atIndex:slot];
            }
            else if (stage == ST_FRAGMENT_SHADER)
            {
                [enc setFragmentTexture:tex->m_texture atIndex:slot];
                [enc setFragmentSamplerState:tex->m_sampler atIndex:slot];
            }
        }
    }
}

void ContextMTL::draw_arrays(DrawType dt, unsigned int n)         { draw_arrays(dt, 0, n); }
void ContextMTL::draw_elements(DrawType dt, unsigned int n)       { draw_elements(dt, 0, n); }

void ContextMTL::log_draw_state(const char* fn, unsigned int n)
{
#if SQ_MTL_DEBUG_LOG
    if (m_frame_index >= SQ_MTL_DEBUG_FRAMES) return;
    SQMTL_LOG(fn << ": count=" << n
              << " shader=" << (void*)m_bind.shader
              << " IL=" << (void*)m_bind.input_layout
              << " VBO=" << (void*)(m_bind.vertex_buffer ? (void*)m_bind.vertex_buffer->m_buffer : nullptr)
              << " IBO=" << (void*)(m_bind.index_buffer ? (void*)m_bind.index_buffer->m_buffer : nullptr)
              << " target=" << (void*)m_bind.render_target);
    if (Shader* sh = m_bind.shader)
    {
        for (int stage = 0; stage < ST_N_SHADER; ++stage)
        {
            if (!sh->m_functions[stage]) continue;
            for (auto& t : sh->m_refl[stage].textures)
            {
                int slot = t.second;
                bool bound = (slot >= 0 && slot < Shader::MAX_TEXTURES && sh->m_bound_textures[slot]);
                SQMTL_LOG("  tex stage=" << stage << " name=" << t.first
                          << " slot=" << slot << " bound=" << bound);
            }
            for (auto& b : sh->m_refl[stage].buffers)
                SQMTL_LOG("  cbuf stage=" << stage << " name=" << b.first << " slot=" << b.second);
        }
    }
#endif
}

void ContextMTL::draw_arrays(DrawType dt, unsigned int start, unsigned int n)
{
    ensure_encoder();
    if (!m_encoder) { SQMTL_LOG("draw_arrays: NO ENCODER -> skipped"); return; }

    apply_state_to_encoder();
    log_draw_state("draw_arrays", n);

    if (m_bind.vertex_buffer)
        [m_encoder setVertexBuffer:m_bind.vertex_buffer->m_buffer offset:0 atIndex:VERTEX_BUFFER_BINDING];

    if (m_bind.shader)
    {
        flush_auto_buffers(m_encoder, m_bind.shader);
        flush_bound_cbs(m_encoder, m_bind.shader);
        flush_textures(m_encoder, m_bind.shader);
    }

    [m_encoder drawPrimitives:to_mtl_primitive(dt) vertexStart:start vertexCount:n];
}

void ContextMTL::draw_elements(DrawType dt, unsigned int start, unsigned int n)
{
    ensure_encoder();
    if (!m_encoder || !m_bind.index_buffer)
    {
        SQMTL_LOG("draw_elements: SKIPPED (encoder=" << (void*)m_encoder
                  << " IBO=" << (void*)(m_bind.index_buffer ? (void*)m_bind.index_buffer->m_buffer : nullptr) << ")");
        return;
    }

    apply_state_to_encoder();
    log_draw_state("draw_elements", n);

    if (m_bind.vertex_buffer)
        [m_encoder setVertexBuffer:m_bind.vertex_buffer->m_buffer offset:0 atIndex:VERTEX_BUFFER_BINDING];

    if (m_bind.shader)
    {
        flush_auto_buffers(m_encoder, m_bind.shader);
        flush_bound_cbs(m_encoder, m_bind.shader);
        flush_textures(m_encoder, m_bind.shader);
    }

    [m_encoder drawIndexedPrimitives:to_mtl_primitive(dt)
                          indexCount:n
                           indexType:MTLIndexTypeUInt32
                         indexBuffer:m_bind.index_buffer->m_buffer
                   indexBufferOffset:start * sizeof(unsigned int)];
}

// ──────────────────────────────────────────────────────────────────────────────
// Render target
// ──────────────────────────────────────────────────────────────────────────────

Target* ContextMTL::create_render_target(const std::vector<TargetField>& fields)
{
    auto* t = new Target();
    for (const auto& f : fields)
    {
        Target::Attachment att;
        att.texture = static_cast<Texture*>(f.m_texture);
        att.type    = f.m_type;
        t->m_attachments.push_back(att);

        if (f.m_type == RT_COLOR && att.texture)
            t->m_color_fmt = att.texture->m_texture.pixelFormat;
        else if ((f.m_type == RT_DEPTH || f.m_type == RT_DEPTH_STENCIL) && att.texture)
            t->m_depth_fmt = att.texture->m_texture.pixelFormat;
    }
    return t;
}

void ContextMTL::enable_render_target(Target* t)
{
    // Flush any in-flight draws to the previous pass, then switch target.
    // The encoder for the new target is created lazily on the first draw so
    // that clear() can set the pending-clear flags before the pass begins.
    end_encoder();
    m_bind.render_target = t;
}

void ContextMTL::disable_render_target(Target*)
{
    end_encoder();
    m_bind.render_target = nullptr;
}

void ContextMTL::delete_render_target(Target*& t) { delete t; t = nullptr; }

void ContextMTL::copy_target_to_target(const IVec4&, Target*, const IVec4&, Target*, TargetType) {}
