//
//  RenderVK.h
//  Square, the driver of Vulkan
//
//  The context of the engine (an immediate API: states, binds, draws) over Vulkan, as the one of
//  Metal does: the draws recorded in the command buffer of the frame, their render pass begun at
//  the first draw into a target (its clears as its load), the pipelines and the render passes
//  made once (caches), the layouts of the images followed (barriers between their uses).
//  - Vulkan 1.1 (its optional features when the device has them: geometry shaders, the layer
//    from the vertex shader, anisotropy, BC/ASTC);
//  - the conventions of DirectX (the SPIR-V of HLSL2ALL in its layout for Vulkan): depth in
//    [0, 1], rows down (the viewport flipped), clockwise fronts;
//  - the constant buffers: their bytes on the CPU, copied in the ring of the frame when a draw
//    reads them changed (each draw sees the values given before it, as DirectX does);
//  - the frames in flight: their command buffers, their rings, their descriptors, what they
//    used deleted only when the GPU is done with them.
//
#pragma once
#include "VulkanConfig.h"
#include <array>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "Square/Config.h"
#include "Square/Driver/Render.h"
#include "Square/Driver/RenderInspector.h"
#include "Square/Driver/Window.h"
#include "SpirvReflection.h"

namespace Square
{
namespace Render
{
	class ContextVK;

	//the frames recorded while the GPU draws the ones before
	static constexpr size_t VK_FRAMES = 2;
	//the color attachments of a target (MRT: the G-Buffer)
	static constexpr size_t VK_MAX_COLORS = 8;

	////////////////////////////////////////////////////////////////////////////////////////////
	//a buffer of the device (its memory by VMA)
	struct BufferVK
	{
		VkBuffer      m_buffer{ VK_NULL_HANDLE };
		VmaAllocation m_allocation{ VK_NULL_HANDLE };
		unsigned char* m_mapped{ nullptr }; //host visible: its bytes
		VkDeviceSize  m_size{ 0 };
	};

	//a slice of the ring of a frame (a constant buffer as a draw reads it)
	struct SliceVK
	{
		VkBuffer     m_buffer{ VK_NULL_HANDLE };
		VkDeviceSize m_offset{ 0 };
		uint64       m_frame{ ~uint64(0) }; //the frame of the ring (another one: copied again)
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	//a constant buffer: its bytes (the draws read a copy of them, in the ring of their frame)
	class ConstBuffer
	{
	public:
		Vector<unsigned char> m_bytes;
		bool                       m_dirty{ true };
		SliceVK                    m_slice;
	};

	//a vertex or an index buffer: static (the device memory, its bytes uploaded) or stream (host
	//visible, written by the CPU; a new buffer when the GPU may still read the old one)
	class GeometryBufferVK
	{
	public:
		BufferVK m_buffer;
		size_t   m_size{ 0 };
		bool     m_stream{ false };
		uint64   m_last_frame{ 0 }; //the last frame that read it (+1: 0 never)
		Vector<unsigned char> m_read; //mapped to be read (a static buffer: its bytes read back)
	};

	class VertexBuffer : public GeometryBufferVK
	{
	public:
		size_t m_stride{ 0 };
		size_t m_count{ 0 };
	};

	class IndexBuffer : public GeometryBufferVK
	{
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	//a texture: its image (all its layers and levels in one layout, followed), its views, its sampler
	class Texture
	{
	public:
		VkImage            m_image{ VK_NULL_HANDLE };
		VmaAllocation      m_allocation{ VK_NULL_HANDLE }; //none: not owned (a swap chain image)
		VkFormat           m_format{ VK_FORMAT_UNDEFINED };
		TextureFormat      m_texture_format{ TF_INVALID };
		VkImageAspectFlags m_aspect{ VK_IMAGE_ASPECT_COLOR_BIT };     //of the image
		VkImageAspectFlags m_view_aspect{ VK_IMAGE_ASPECT_COLOR_BIT }; //sampled (a depth: its depth)
		uint32_t           m_width{ 0 };
		uint32_t           m_height{ 0 };
		uint32_t           m_layers{ 1 };
		uint32_t           m_levels{ 1 };
		bool               m_cube{ false };
		bool               m_array{ false };
		bool               m_depth{ false };
		bool               m_srgb{ false };
		VkImageLayout      m_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkSampler          m_sampler{ VK_NULL_HANDLE }; //of the cache of the context
		//its views: to sample it (by the shape a shader reads), to draw into it (all its layers,
		//one of them), made when asked
		UnorderedMap<uint64, VkImageView> m_views;
		uint64             m_last_frame{ 0 }; //the last frame that used it (+1)
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	//an input layout: its attributes (at the location of their type: the layout of the SPIR-V)
	class InputLayout
	{
	public:
		AttributeList m_list;
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	//a render target: its textures (a layer of them, or all of them: layered), its framebuffers
	class Target
	{
	public:
		struct Attachment
		{
			Texture*   m_texture{ nullptr };
			TargetType m_type{ RT_COLOR };
			int        m_layer{ -1 };
		};
		Vector<Attachment> m_colors;
		Attachment              m_depth;
		uint32_t                m_width{ 0 };
		uint32_t                m_height{ 0 };
		uint32_t                m_layers{ 1 };
		//its framebuffers, by their render pass
		UnorderedMap<VkRenderPass, VkFramebuffer> m_framebuffers;
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	//the uniforms of a shader: a texture (its binding), a loose uniform (its bytes in the global
	//block), a constant buffer (its binding)
	class Shader;

	class UniformVK : public Uniform
	{
	public:
		//a texture
		UniformVK(Shader* shader, uint32_t binding) : m_shader(shader), m_binding(binding), m_texture(true) {}
		//a loose uniform
		UniformVK(Shader* shader, uint32_t offset, uint32_t size) : m_shader(shader), m_offset(offset), m_size(size) {}

		void write(const void* data, size_t size);
		template < class T > void write_values(const T* values, size_t count) { write(values, sizeof(T) * count); }

		virtual void set(Texture* texture) override;
		virtual void set(int i) override { write_values(&i, 1); }
		virtual void set(float f) override { write_values(&f, 1); }
		virtual void set(double d) override { write_values(&d, 1); }
		virtual void set(const IVec2& v) override { write_values(&v, 1); }
		virtual void set(const IVec3& v) override { write_values(&v, 1); }
		virtual void set(const IVec4& v) override { write_values(&v, 1); }
		virtual void set(const Vec2& v) override { write_values(&v, 1); }
		virtual void set(const Vec3& v) override { write_values(&v, 1); }
		virtual void set(const Vec4& v) override { write_values(&v, 1); }
		virtual void set(const Mat3& m) override { write_values(&m, 1); }
		virtual void set(const Mat4& m) override { write_values(&m, 1); }
		virtual void set(const DVec2& v) override { write_values(&v, 1); }
		virtual void set(const DVec3& v) override { write_values(&v, 1); }
		virtual void set(const DVec4& v) override { write_values(&v, 1); }
		virtual void set(const DMat3& m) override { write_values(&m, 1); }
		virtual void set(const DMat4& m) override { write_values(&m, 1); }
		virtual void set(Texture* t, size_t n) override { if (n) set(t); }
		virtual void set(const int* p, size_t n) override { write_values(p, n); }
		virtual void set(const float* p, size_t n) override { write_values(p, n); }
		virtual void set(const double* p, size_t n) override { write_values(p, n); }
		virtual void set(const IVec2* p, size_t n) override { write_values(p, n); }
		virtual void set(const IVec3* p, size_t n) override { write_values(p, n); }
		virtual void set(const IVec4* p, size_t n) override { write_values(p, n); }
		virtual void set(const Vec2* p, size_t n) override { write_values(p, n); }
		virtual void set(const Vec3* p, size_t n) override { write_values(p, n); }
		virtual void set(const Vec4* p, size_t n) override { write_values(p, n); }
		virtual void set(const Mat3* p, size_t n) override { write_values(p, n); }
		virtual void set(const Mat4* p, size_t n) override { write_values(p, n); }
		virtual void set(const DVec2* p, size_t n) override { write_values(p, n); }
		virtual void set(const DVec3* p, size_t n) override { write_values(p, n); }
		virtual void set(const DVec4* p, size_t n) override { write_values(p, n); }
		virtual void set(const DMat3* p, size_t n) override { write_values(p, n); }
		virtual void set(const DMat4* p, size_t n) override { write_values(p, n); }
		virtual void set(const Vector<Texture*>& v) override { if (v.size()) set(v[0]); }
		virtual void set(const Vector<int>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<float>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<double>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<IVec2>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<IVec3>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<IVec4>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<Vec2>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<Vec3>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<Vec4>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<Mat3>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<Mat4>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<DVec2>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<DVec3>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<DVec4>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<DMat3>& v) override { write_values(v.data(), v.size()); }
		virtual void set(const Vector<DMat4>& v) override { write_values(v.data(), v.size()); }
		virtual bool is_valid() override { return m_shader != nullptr; }
		virtual Shader* get_shader() override { return m_shader; }

	private:
		Shader*  m_shader{ nullptr };
		uint32_t m_binding{ 0 };
		uint32_t m_offset{ 0 };
		uint32_t m_size{ 0 };
		bool     m_texture{ false };
	};

	class UniformConstBufferVK : public UniformConstBuffer
	{
	public:
		UniformConstBufferVK(Shader* shader, uint32_t binding) : m_shader(shader), m_binding(binding) {}
		virtual void bind(const ConstBuffer* buffer) override;
		virtual void unbind() override;
		virtual bool is_valid() override { return m_shader != nullptr; }
		virtual Shader* get_shader() override { return m_shader; }

	private:
		Shader*  m_shader{ nullptr };
		uint32_t m_binding{ 0 };
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	//a binding of a shader (its descriptor)
	struct BindingVK
	{
		uint32_t                       m_binding{ 0 };
		VkDescriptorType               m_type{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC };
		VkShaderStageFlags             m_stages{ 0 };
		uint32_t                       m_count{ 1 };
		String                    m_name;
		uint32_t                       m_size{ 0 };       //a buffer: its block
		bool                           m_global{ false }; //the block of the loose uniforms
		SpirvReflection::ImageDim      m_dim{ SpirvReflection::ImageDim::DIM_2D };
		bool                           m_arrayed{ false };
	};

	//a shader: its stages (the modules of their SPIR-V), its bindings (one set), what is bound to
	//them (by binding: the constant buffers, the textures), its loose uniforms (their block)
	class Shader
	{
	public:
		VkShaderModule              m_modules[ST_N_SHADER]{};
		String                 m_entries[ST_N_SHADER];
		Vector<BindingVK>      m_bindings;
		Vector<SpirvReflection::Input> m_inputs;
		bool                        m_writes_layer{ false };
		VkDescriptorSetLayout       m_set_layout{ VK_NULL_HANDLE };
		VkPipelineLayout            m_pipeline_layout{ VK_NULL_HANDLE };
		uint32_t                    m_dynamic_buffers{ 0 }; //the dynamic offsets of its set
		//bound to its bindings (by their binding)
		Vector<const ConstBuffer*> m_buffers;
		Vector<Texture*>           m_textures;
		//the loose uniforms: their block, its binding, their offsets
		int                         m_global_binding{ -1 };
		ConstBuffer                 m_global;
		UnorderedMap<String, SpirvReflection::Member> m_members;
		//the uniforms given (owned by it)
		UnorderedMap<String, UniquePtr<UniformVK>>            m_uniforms;
		UnorderedMap<String, UniquePtr<UniformConstBufferVK>> m_uniform_buffers;
		//the last set of its draws (the same resources: the same set, other offsets)
		VkDescriptorSet             m_set{ VK_NULL_HANDLE };
		uint64                      m_set_frame{ ~uint64(0) };
		Vector<uint64>         m_set_key;
		//errors
		Vector<String>    m_errors;
		String                 m_link_error;
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	//what makes a pipeline (the states of the context at a draw)
	struct PipelineKeyVK
	{
		Shader*        m_shader{ nullptr };
		InputLayout*   m_input_layout{ nullptr };
		VkRenderPass   m_render_pass{ VK_NULL_HANDLE }; //(of the formats of the target: compatible)
		uint32_t       m_colors{ 0 };
		BlendState     m_blend;
		DepthBufferState m_depth;
		CullfaceType   m_cullface{ CF_DISABLE };
		VkPrimitiveTopology m_topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };

		bool operator==(const PipelineKeyVK& other) const
		{
			return m_shader == other.m_shader
				&& m_input_layout == other.m_input_layout
				&& m_render_pass == other.m_render_pass
				&& m_colors == other.m_colors
				&& m_blend == other.m_blend
				&& m_depth == other.m_depth
				&& m_cullface == other.m_cullface
				&& m_topology == other.m_topology;
		}
	};

	struct PipelineKeyHashVK
	{
		size_t operator()(const PipelineKeyVK& key) const
		{
			size_t h = std::hash<const void*>()(key.m_shader);
			h = h * 31 + std::hash<const void*>()(key.m_input_layout);
			h = h * 31 + std::hash<uint64>()(uint64(key.m_render_pass));
			h = h * 31 + key.m_colors;
			h = h * 31 + (size_t(key.m_blend.m_enable) << 16 | size_t(key.m_blend.m_src) << 8 | size_t(key.m_blend.m_dst));
			h = h * 31 + (size_t(key.m_depth.m_mode) << 8 | size_t(key.m_depth.m_type));
			h = h * 31 + size_t(key.m_cullface);
			h = h * 31 + size_t(key.m_topology);
			return h;
		}
	};

	//what makes a render pass: the formats of its attachments, the clears of its load
	struct RenderPassKeyVK
	{
		Array<VkFormat, VK_MAX_COLORS> m_colors{};
		uint32_t m_count{ 0 };
		VkFormat m_depth{ VK_FORMAT_UNDEFINED };
		bool     m_clear_color{ false };
		bool     m_clear_depth{ false };

		bool operator==(const RenderPassKeyVK& other) const
		{
			return m_colors == other.m_colors && m_count == other.m_count && m_depth == other.m_depth
				&& m_clear_color == other.m_clear_color && m_clear_depth == other.m_clear_depth;
		}
	};

	struct RenderPassKeyHashVK
	{
		size_t operator()(const RenderPassKeyVK& key) const
		{
			size_t h = key.m_count;
			for (VkFormat format : key.m_colors) h = h * 31 + size_t(format);
			h = h * 31 + size_t(key.m_depth);
			h = h * 31 + (size_t(key.m_clear_color) << 1 | size_t(key.m_clear_depth));
			return h;
		}
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	//a frame in flight: its command buffers, its ring, its descriptors, what it deletes when done
	struct FrameVK
	{
		VkCommandPool                m_pool{ VK_NULL_HANDLE };
		VkCommandBuffer              m_commands{ VK_NULL_HANDLE }; //the draws
		VkCommandBuffer              m_uploads{ VK_NULL_HANDLE };  //the uploads (before the draws)
		bool                         m_uploads_open{ false };
		VkFence                      m_fence{ VK_NULL_HANDLE };
		VkSemaphore                  m_acquired{ VK_NULL_HANDLE };
		//the ring of the constant buffers: its buffers, the one in use, its offset
		Vector<BufferVK>        m_ring;
		size_t                       m_ring_buffer{ 0 };
		VkDeviceSize                 m_ring_offset{ 0 };
		//its descriptor pools
		Vector<VkDescriptorPool> m_descriptor_pools;
		size_t                       m_descriptor_pool{ 0 };
		//the bytes of the staging buffers of its uploads
		VkDeviceSize                 m_staging_bytes{ 0 };
#if defined(RENDER_PROFILER)
		VkQueryPool                  m_queries{ VK_NULL_HANDLE };
#endif
	};

	//the swap chain of the window, its images (as textures of the default target)
	struct SwapChainVK
	{
		VkSwapchainKHR           m_swapchain{ VK_NULL_HANDLE };
		VkFormat                 m_format{ VK_FORMAT_UNDEFINED };
		VkExtent2D               m_extent{ 0, 0 };
		bool                     m_vsync{ false };
		Vector<UniquePtr<Texture>> m_images;
		Vector<VkSemaphore> m_presented; //by image: the draws done, it can be presented
		uint32_t                 m_image{ 0 };
		bool                     m_acquired{ false };      //an image of this frame
		bool                     m_acquired_waited{ false }; //its semaphore waited by a submit
		bool                     m_dirty{ false };         //made again before the next image
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	class ContextVK : public Context
	{
	public:
		ContextVK(Allocator* allocator, Logger* logger) : Context(allocator, logger) {}
		virtual ~ContextVK() { close(); }

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

		virtual const ScissorState& get_scissor_state() override;
		virtual void set_scissor_state(const ScissorState& ss) override;

		virtual const BlendState& get_blend_state() override;
		virtual void set_blend_state(const BlendState& bs) override;

		virtual const State& get_render_state() override;
		virtual void set_render_state(const State& rs) override;

		virtual ConstBuffer* create_stream_CB(const unsigned char* data, size_t size) override;
		virtual VertexBuffer* create_stream_VBO(const unsigned char* vbo, size_t stride, size_t n) override;
		virtual IndexBuffer* create_stream_IBO(const unsigned int* ibo, size_t size) override;

		virtual ConstBuffer* create_CB(const unsigned char* data, size_t size) override;
		virtual VertexBuffer* create_VBO(const unsigned char* vbo, size_t stride, size_t n) override;
		virtual IndexBuffer* create_IBO(const unsigned int* ibo, size_t size) override;

		virtual uint64 get_native_CB(const ConstBuffer*) const override;
		virtual uint64 get_native_VBO(const VertexBuffer*) const override;
		virtual uint64 get_native_IBO(const IndexBuffer*) const override;

		virtual size_t get_size_CB(const ConstBuffer*) const override;
		virtual size_t get_size_VBO(const VertexBuffer*) const override;
		virtual size_t get_size_IBO(const IndexBuffer*) const override;

		virtual void update_steam_CB(ConstBuffer* cb, const unsigned char* vb, size_t n) override;
		virtual void update_steam_VBO(VertexBuffer* vbo, const unsigned char* vb, size_t n) override;
		virtual void update_steam_IBO(IndexBuffer* vbo, const unsigned int* ib, size_t n) override;

		virtual void bind_CB(ConstBuffer*) override;
		virtual void bind_VBO(VertexBuffer*) override;
		virtual void bind_IBO(IndexBuffer*) override;

		virtual void unbind_CB(ConstBuffer*) override;
		virtual void unbind_VBO(VertexBuffer*) override;
		virtual void unbind_IBO(IndexBuffer*) override;

		virtual Vector<unsigned char> copy_buffer_CB(const ConstBuffer*) override;
		virtual Vector<unsigned char> copy_buffer_VBO(const VertexBuffer*) override;
		virtual Vector<unsigned char> copy_buffer_IBO(const IndexBuffer*) override;

		virtual unsigned char* map_CB(ConstBuffer*, size_t start, size_t n, MappingType type) override;
		virtual unsigned char* map_CB(ConstBuffer*, MappingType type) override;
		virtual void unmap_CB(ConstBuffer*) override;

		virtual unsigned char* map_VBO(VertexBuffer*, size_t start, size_t n, MappingType type) override;
		virtual unsigned char* map_VBO(VertexBuffer*, MappingType type) override;
		virtual void unmap_VBO(VertexBuffer*) override;

		virtual unsigned int* map_IBO(IndexBuffer*, size_t start, size_t n, MappingType type) override;
		virtual unsigned int* map_IBO(IndexBuffer*, MappingType type) override;
		virtual void unmap_IBO(IndexBuffer*) override;

		virtual unsigned char* map_TBO(Texture*, MappingType type) override;
		virtual void unmap_TBO(Texture*) override;

		virtual void delete_CB(ConstBuffer*&) override;
		virtual void delete_VBO(VertexBuffer*&) override;
		virtual void delete_IBO(IndexBuffer*&) override;

		virtual void draw_arrays(DrawType type, unsigned int n) override;
		virtual void draw_arrays(DrawType type, unsigned int start, unsigned int size) override;
		virtual void draw_elements(DrawType type, unsigned int n) override;
		virtual void draw_elements(DrawType type, unsigned int start, unsigned int n) override;
		virtual void draw_arrays_instanced(DrawType type, unsigned int start, unsigned int size, unsigned int instances) override;
		virtual void draw_elements_instanced(DrawType type, unsigned int start, unsigned int n, unsigned int instances) override;

		virtual InputLayout* create_IL(const AttributeList& atl) override;
		virtual void delete_IL(InputLayout*&) override;
		virtual void bind_IL(InputLayout*) override;
		virtual void unbind_IL(InputLayout* il) override;
		virtual size_t size_IL(const InputLayout* layout) override;
		virtual bool   has_a_position_IL(const InputLayout* layout) override;
		virtual size_t position_offset_IL(const InputLayout* layout) override;

		virtual float get_depth(const Vec2& pixel) const override { return 0.0f; }
		virtual Vec4 get_color(const Vec2& pixel) const override { return Vec4(0.0f); }

		virtual Texture* create_texture(const TextureRawDataInformation& data, const TextureGpuDataInformation& info) override;
		virtual Texture* create_texture_array(const TextureRawDataInformation& data, const TextureGpuDataInformation& info, int size) override;
		virtual Texture* create_cube_texture(const TextureRawDataInformation data[6], const TextureGpuDataInformation& info) override;
		virtual Vector<unsigned char> get_texture(Texture*, int level = 0) override;
		virtual void copy_texture(Texture* source, Texture* destination) override;
		virtual void copy_texture_layer(Texture* source, unsigned int source_layer, Texture* destination, unsigned int destination_layer) override;
		virtual Vector<unsigned char> get_texture(Texture*, int face, int level) override;
		virtual void bind_texture(Texture*, int texture_id, int sample_id) override;
		virtual void unbind_texture(Texture*) override;
		virtual void unbind_texture(int n) override;
		virtual void delete_texture(Texture*&) override;
		virtual void update_texture(Texture* texture, const TextureRawDataInformation& data) override;

		virtual Shader* create_shader(const Vector<ShaderSourceInformation>& infos) override;
		virtual bool shader_compiled_with_errors(Shader* shader) override;
		virtual bool shader_linked_with_error(Shader* shader) override;
		virtual Vector<String> get_shader_compiler_errors(Shader* shader) override;
		virtual String get_shader_linker_error(Shader* shader) override;
		virtual void bind_shader(Shader* shader) override;
		virtual void unbind_shader(Shader* shader) override;
		virtual void bind_uniform_CB(const ConstBuffer*, Shader*, const String& uname) override;
		virtual void unbind_uniform_CB(Shader*, const String& uname) override;
		virtual void delete_shader(Shader*&) override;
		virtual Shader* get_bind_shader() const override;
		virtual Uniform* get_uniform(Shader*, const String& uname) const override;
		virtual UniformConstBuffer* get_uniform_const_buffer(Shader*, const String& uname) const override;

		virtual Target* create_render_target(const Vector<TargetField>& textures) override;
		virtual void enable_render_target(Target*) override;
		virtual void disable_render_target(Target*) override;
		virtual void delete_render_target(Target*&) override;
		virtual void copy_target_to_target(const IVec4& from_area, Target* from, const IVec4& to_area, Target* to, TargetType mask) override;

		virtual bool print_errors() const override;
		virtual bool print_errors(const char* source_file_name, int line) const override;

#if defined(RENDER_PROFILER)
		virtual bool gpu_timer_supported() const override;
		virtual void gpu_timer_begin_frame() override;
		virtual void gpu_timer_end_frame() override;
		virtual int  gpu_timer_timestamp() override;
		virtual bool gpu_timer_read_frame(uint64& frame_id, Vector<double>& timestamps_ms) override;
#endif

	private:
		//the device
		bool create_instance(bool debug);
		bool pick_device(Video::ContextInfo::gpu_type gpu);
		bool create_device();
		bool create_frames();
		void compute_driver_info();
		//the swap chain (made again when the window changes)
		bool create_swapchain();
		void destroy_swapchain();
		bool acquire_image();
#if defined(__ANDROID__)
		//a new window (the activity back from the background): its surface, its swap chain
		bool recreate_surface();
#endif
		void present_frame();
		//the frames
		FrameVK& frame() { return m_frames[m_frame_index % VK_FRAMES]; }
		void begin_frame();
		VkCommandBuffer commands();
		VkCommandBuffer uploads();
		void submit_frame(bool present);
		void flush_and_wait();
		void destroy_later(std::function<void()> destroy);
		void destroy_garbage(bool all);
		uint64 completed_frame() const { return m_completed_frame; }
		//the ring of the frame: a slice of it, its bytes copied
		SliceVK ring_copy(const void* data, size_t size, size_t reserve);
		//buffers
		BufferVK create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, bool host, bool readback = false);
		void destroy_buffer_later(BufferVK& buffer);
		bool upload_buffer(BufferVK& buffer, const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
		void stream_write(GeometryBufferVK& buffer, VkBufferUsageFlags usage, const void* data, size_t size);
		Vector<unsigned char> read_buffer(const BufferVK& buffer, size_t size);
		//textures
		Texture* make_texture(const TextureRawDataInformation* data, int layers, bool cube, const TextureGpuDataInformation& info);
		VkImageView view_of(Texture* texture, VkImageViewType type, uint32_t layer, uint32_t layers, VkImageAspectFlags aspect, bool all_levels);
		VkImageView sampled_view(Texture* texture, SpirvReflection::ImageDim dim, bool arrayed);
		VkSampler sampler_of(const TextureGpuDataInformation& info, uint32_t levels);
		void transition(VkCommandBuffer commands, Texture* texture, VkImageLayout layout);
		void generate_mipmaps(VkCommandBuffer commands, Texture* texture);
		void destroy_texture_later(Texture* texture);
		Texture* dummy_texture(SpirvReflection::ImageDim dim, bool arrayed);
		//targets, render passes
		Target* current_target();
		VkRenderPass render_pass(const RenderPassKeyVK& key);
		RenderPassKeyVK render_pass_key(const Target* target, bool clear_color, bool clear_depth) const;
		VkFramebuffer framebuffer(Target* target, VkRenderPass pass);
		void destroy_framebuffers_later(Target* target);
		bool begin_render_pass();
		void end_render_pass();
		void prepare_target_textures(Target* target);
		//draws
		bool prepare_draw(DrawType type);
		VkPipeline pipeline(const PipelineKeyVK& key);
		bool bind_resources(VkCommandBuffer commands);
		bool prepare_textures();
		void forget(const ConstBuffer* buffer);
		void forget(const Texture* texture);
		void forget_pipelines(const Shader* shader, const InputLayout* layout);
		VkFormat depth_format(TextureFormat format) const;

		//the device
		VkInstance               m_instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT m_messenger{ VK_NULL_HANDLE };
		VkSurfaceKHR             m_surface{ VK_NULL_HANDLE };
#if defined(__ANDROID__)
		void*                    m_surface_window{ nullptr }; //the native window of the surface
#endif
		VkPhysicalDevice         m_physical{ VK_NULL_HANDLE };
		VkPhysicalDeviceProperties m_properties{};
		VkPhysicalDeviceFeatures m_features{};
		VkDevice                 m_device{ VK_NULL_HANDLE };
		uint32_t                 m_queue_family{ 0 };
		VkQueue                  m_queue{ VK_NULL_HANDLE };
		VmaAllocator             m_allocator{ VK_NULL_HANDLE };
		VkPipelineCache          m_pipeline_cache{ VK_NULL_HANDLE };
		uint32_t                 m_api_version{ VK_API_VERSION_1_1 };
		bool                     m_vertex_layer{ false };
		Video::DeviceResources*  m_resources{ nullptr };
		RenderDriverInfo         m_info;
		bool                     m_srgb{ false };
		//the frames
		Array<FrameVK, VK_FRAMES> m_frames;
		uint64                   m_frame_index{ 0 };   //the frame recorded (its slot: % VK_FRAMES)
		uint64                   m_completed_frame{ 0 }; //the frames done by the GPU (+1)
		bool                     m_frame_open{ false };
		VkFence                  m_wait_fence{ VK_NULL_HANDLE }; //a submit waited (flush_and_wait)
		//what is deleted: the last frame that may use it, its destruction (when that frame is done)
		Vector<std::pair<uint64, std::function<void()>>> m_garbage;
		//the swap chain, the default target (its image, the depth of the driver)
		SwapChainVK              m_swapchain;
		UniquePtr<Texture> m_default_depth;
		Target                   m_default_target;
		//the states
		State                    m_state;
		ScissorState             m_scissor;
		//what is bound
		Shader*                  m_shader{ nullptr };
		InputLayout*             m_input_layout{ nullptr };
		VertexBuffer*            m_vertex_buffer{ nullptr };
		IndexBuffer*             m_index_buffer{ nullptr };
		Target*                  m_target{ nullptr };      //nullptr: the default one
		//the render pass recording (its target, its clears)
		bool                     m_pass_open{ false };
		Target*                  m_pass_target{ nullptr };
		VkRenderPass             m_pass{ VK_NULL_HANDLE };
		bool                     m_clear_color{ true };
		bool                     m_clear_depth{ true };
		VkPipeline               m_bound_pipeline{ VK_NULL_HANDLE };
		bool                     m_dynamic_dirty{ true };
		//the caches
		UnorderedMap<RenderPassKeyVK, VkRenderPass, RenderPassKeyHashVK> m_render_passes;
		UnorderedMap<PipelineKeyVK, VkPipeline, PipelineKeyHashVK>     m_pipelines;
		UnorderedMap<uint64, VkSampler>                               m_samplers;
		Vector<Shader*>     m_shaders; //alive (what they bind forgotten when it is deleted)
		//the resources of the missing bindings (zeros), the attributes missing (zeros)
		UnorderedMap<uint32_t, UniquePtr<Texture>> m_dummy_textures;
		BufferVK                 m_zero_buffer;
		VkDeviceSize             m_ring_alignment{ 256 };
		//the lists of bind_resources, of every draw (cleared, their memory kept: no block a draw)
		Vector<uint64>                 m_bind_key;
		Vector<uint32_t>               m_bind_offsets;
		Vector<VkDescriptorBufferInfo> m_bind_buffers;
		Vector<VkDescriptorImageInfo>  m_bind_images;
		Vector<VkWriteDescriptorSet>   m_bind_writes;
		//errors
		mutable Vector<String> m_errors;
#if defined(RENDER_PROFILER)
		struct GpuTimerFrame
		{
			uint64   m_id{ 0 };
			uint64   m_frame{ 0 };     //the frame recorded with them
			uint32_t m_count{ 0 };
			bool     m_pending{ false };
		};
		static constexpr uint32_t GPU_TIMER_MAX_TIMESTAMPS = 8192;
		Array<GpuTimerFrame, VK_FRAMES> m_gpu_timer_frames;
		uint64                   m_gpu_timer_frame_id{ 0 };
		bool                     m_gpu_timer_in_frame{ false };
		bool                     m_gpu_timer_supported{ false };
#endif
	};
}
}
