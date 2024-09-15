//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Variant.h"
#include "Square/Core/Allocator.h"
#include "Square/Core/Logger.h"


//declare
namespace Square
{
namespace Video
{
	class Input;
	class Window;
	struct DeviceResources;
}
}

namespace Square
{
namespace Render
{
	////////////////////////////////////////////////
	// CLASS
	class Texture;
	class ConstBuffer;
	class Target;
	class VertexBuffer;
	class IndexBuffer;
	class InputLayout;
	class Shader;
    class Context;
    class Uniform;
    class UniformConstBuffer;
    class MapConstBuffer;
	// POINTERS
	using TextureSPtr	   = Shared< Texture >;
	using TargetSPtr	   = Shared< Target >;
	using VertexBufferSPtr = Shared< VertexBuffer >;
	using IndexBufferSPtr  = Shared< IndexBuffer >;
	using InputLayoutSPtr  = Shared< InputLayout >;
	using ShaderSPtr       = Shared< Shader >;
	////////////////////////////////////////////////
	//shader type
	enum ShaderType
	{
		ST_VERTEX_SHADER,
		ST_FRAGMENT_SHADER,
		ST_GEOMETRY_SHADER,
		ST_TASSELLATION_CONTROL_SHADER,
		ST_TASSELLATION_EVALUATION_SHADER,
		ST_COMPUTE_SHADER,
		ST_N_SHADER
	};
	//shader info
	struct ShaderSourceInformation
	{
		const ShaderType  m_type;
		const std::string  m_entry_point{ "main" };
		const std::string  m_shader_header;
		const std::string& m_shader_source;
		const size_t m_line;

		ShaderSourceInformation(const ShaderSourceInformation& info)
		: m_type(info.m_type)
		, m_shader_header(info.m_shader_header)
		, m_shader_source(info.m_shader_source)
		, m_entry_point(info.m_entry_point)
		, m_line(info.m_line)
		{

		}

		ShaderSourceInformation
		(
			ShaderType type,
			const std::string& shader_source,
			const size_t line = 0
		)
		: m_type(type)
		, m_shader_source(shader_source)
		, m_line(line)
		{
		}

		ShaderSourceInformation
		(
			ShaderType type,
			const std::string& shader_header,
			const std::string& shader_source,
			const size_t line = 0
		)
		: m_type(type)
		, m_shader_header(shader_header)
		, m_shader_source(shader_source)
		, m_line(line)
		{
		}
		ShaderSourceInformation
		(
			ShaderType type,
			const std::string& shader_header,
			const std::string& shader_source,
			const std::string& entry_point,
			const size_t line = 0
		)
		: m_type(type)
		, m_shader_header(shader_header)
		, m_shader_source(shader_source)
		, m_entry_point(entry_point)
		, m_line(line)
		{
		}
	};   
	////////////////////////////////////////////////
	// Types
	enum CullfaceType : unsigned char
	{
		CF_DISABLE = 0,
		CF_FRONT,
		CF_BACK,
		CF_FRONT_AND_BACK,

		CF_INVALID = 0xFF
	};

	enum DrawType : unsigned char
	{
		DRAW_POINTS,
		DRAW_LINES,
		DRAW_LINE_LOOP,
		DRAW_TRIANGLES,
		DRAW_TRIANGLE_STRIP,

		DRAW_INVALID = 0xFF
	};

	enum BlendType : unsigned char
	{
		BLEND_ZERO,
		BLEND_ONE,

		BLEND_ONE_MINUS_DST_COLOR,
		BLEND_ONE_MINUS_DST_ALPHA,
		BLEND_ONE_MINUS_SRC_COLOR,
		BLEND_ONE_MINUS_SRC_ALPHA,

		BLEND_DST_COLOR,
		BLEND_DST_ALPHA,

		BLEND_SRC_COLOR,
		BLEND_SRC_ALPHA,
		BLEND_SRC_ALPHA_SATURATE,

		BLEND_INVALID = 0xFF
	};
	
	enum DepthFuncType : unsigned char
	{
		DT_NEVER,
		DT_LESS,          // <
		DT_GREATER,       // >
		DT_EQUAL,         // ==
		DT_LESS_EQUAL,    // <=
		DT_GREATER_EQUAL, // >=
		DT_NOT_EQUAL,     // !=
		DT_ALWAYS,

		DT_INVALID = 0xFF
	};

	enum MappingType : unsigned char
	{
		MAP_WRITE,
		MAP_READ,
		MAP_WRITE_AND_READ,

		MAP_INVALID = 0xFF
	};
	
	enum ClearType : unsigned char
	{
		CLEAR_COLOR = 0x01,
		CLEAR_DEPTH = 0x02,
		CLEAR_COLOR_DEPTH = 0x03,

		CLEAR_INVALID = 0xFF
	};

	enum DepthMode : unsigned char
	{
		DM_DISABLE,
		DM_ENABLE_AND_WRITE,
		DM_ENABLE_ONLY_READ,

		DM_INVALID = 0xFF
	};
	////////////////////////////////////////////////
	// TEXTURE
	enum TextureFormat : unsigned short
	{
		////////////////////
		//RGBA
		TF_RGBA8,
		//uint
		TF_RGBA16UI,
		TF_RGBA32UI,
		//int
		TF_RGBA16I,
		TF_RGBA32I,
		//float
		TF_RGBA16F,
		TF_RGBA32F,
		////////////////////
		//RGB
		TF_RGB8,
		//uint
		TF_RGB16UI,
		TF_RGB32UI,
		//int
		TF_RGB16I,
		TF_RGB32I,
		//float
		TF_RGB16F,
		TF_RGB32F,
		////////////////////
		//RG
		TF_RG8,
		//uint
		TF_RG16UI,
		TF_RG32UI,
		//int
		TF_RG16I,
		TF_RG32I,
		//float
		TF_RG16F,
		TF_RG32F,
		////////////////////
		//R
		TF_R8,
		//uint
		TF_R16UI,
		TF_R32UI,
		//int
		TF_R16I,
		TF_R32I,
		//float
		TF_R16F,
		TF_R32F,
		////////////////////
		TF_RGB565,
		TF_RGB5A1,
		TF_RGBA4,
		////////////////////
		TF_DEPTH16_STENCIL8,
		TF_DEPTH24_STENCIL8,
		TF_DEPTH32_STENCIL8,
		TF_DEPTH_COMPONENT16,
		TF_DEPTH_COMPONENT24,
		TF_DEPTH_COMPONENT32,

		TF_INVALID = 0xFFFF
	};

	enum TextureType : unsigned char
	{
		TT_R,
		TT_RG,
		TT_RGB,
		TT_RGBA,
		TT_DEPTH,
		TT_DEPTH_STENCIL,
		
		TT_INVALID = 0xFF
	};

	enum TextureTypeFormat : unsigned char
	{
		TTF_FLOAT,
		TTF_UNSIGNED_BYTE,
		TTF_UNSIGNED_SHORT,
		TTF_UNSIGNED_INT,
        TTF_UNSIGNED_INT_24_8,
        TTF_FLOAT_32_UNSIGNED_INT_24_8,

		TTF_INVALID = 0xFF
	};

	enum TextureMagFilterType : unsigned char
	{
		TMAG_NEAREST,
		TMAG_LINEAR,

		TMAG_INVALID = 0xFF
	};

	enum TextureMinFilterType : unsigned char
	{
		TMIN_NEAREST,
		TMIN_NEAREST_MIPMAP_NEAREST,
		TMIN_NEAREST_MIPMAP_LINEAR,
		TMIN_LINEAR,
		TMIN_LINEAR_MIPMAP_NEAREST,
		TMIN_LINEAR_MIPMAP_LINEAR,
		
		TMIN_INVALID = 0xFF
	};

	enum TextureEdgeType : unsigned char
	{
		TEDGE_CLAMP,
		TEDGE_REPEAT,

		TEDGE_INVALID = 0xFF
	};
	
	struct TextureRawDataInformation
	{
		TextureFormat		m_format;
		unsigned int		m_width;
		unsigned int		m_height;
		const unsigned char*m_bytes;
		TextureType		m_type;
		TextureTypeFormat m_type_format;
	};

	struct TextureGpuDataInformation
	{
		TextureMinFilterType		 m_min_type;
		TextureMagFilterType		 m_mag_type;
		TextureEdgeType				 m_edge_s;
		TextureEdgeType				 m_edge_t;
		TextureEdgeType				 m_edge_r;
		int							 m_mipmap_min;
		int							 m_mipmap_max;
		bool						 m_build_mipmap;
		float						 m_anisotropy;
		bool						 m_read_from_cpu;
		//2d/cude texture
		TextureGpuDataInformation
		(
			TextureMinFilterType		 min_type,
			TextureMagFilterType		 mag_type,
			TextureEdgeType				 edge_s,
			TextureEdgeType				 edge_t,
			TextureEdgeType				 edge_r = TEDGE_REPEAT,
			bool				         build_mipmap = false,
			int						     mipmap_min = 0,
			int							 mipmap_max = 10,
			float						 anisotropy = 1.0f,
			bool						 read_from_cpu = false
		)
		{
			m_min_type = min_type;
			m_mag_type = mag_type;
			m_edge_s = edge_s;
			m_edge_t = edge_t;
			m_edge_r = edge_r;
			m_build_mipmap = build_mipmap;
			m_mipmap_min = mipmap_min;
			m_mipmap_max = mipmap_max;
			m_anisotropy = anisotropy;
			m_read_from_cpu = read_from_cpu;
		}
	};
	////////////////////////////////////////////////
	// TARGET
	enum TargetType : unsigned char
	{
		RT_COLOR,
		RT_DEPTH,
		RT_DEPTH_STENCIL,
		RT_STENCIL,

		RT_INVALID = 0xFF
	};

	struct TargetField
	{
		Texture*   m_texture;
		TargetType m_type;
	};
	////////////////////////////////////////////////
	// ATTRIBUTES
	enum AttributeType : unsigned short
	{
		//POSITION TRANSFORM
		ATT_POSITION = 0,
		ATT_POSITION0 = 0,
		ATT_NORMAL0 = 1,
		ATT_TEXCOORD0 = 2,
		ATT_TANGENT0 = 3,
		ATT_BINORMAL0 = 4,
		ATT_COLOR0 = 5,

		//POSITION 1
		ATT_POSITION1 = 6,
		ATT_NORMAL1 = 7,
		ATT_TEXCOORD1 = 8,
		ATT_TANGENT1 = 9,
		ATT_BINORMAL1 = 10,
		ATT_COLOR1 = 11,

		//POSITION 2
		ATT_NORMAL2 = 12,
		ATT_TEXCOORD2 = 13,
		ATT_TANGENT2 = 14,
		ATT_BINORMAL2 = 15,
		ATT_COLOR2 = 16,

		ATT_INVALID = 0xFFFF
	};

	enum AttributeStripType : unsigned short
	{
		AST_FLOAT,
		AST_FLOAT2,
		AST_FLOAT3,
		AST_FLOAT4,

		AST_INT,
		AST_INT2,
		AST_INT3,
		AST_INT4,

		AST_UINT,
		AST_UINT2,
		AST_UINT3,
		AST_UINT4,

		AST_TLESS,
		AST_TLESS2,
		AST_TLESS3,
		AST_TLESS4,

		AST_INVALID = 0xFFFF
	};

	struct Attribute
	{
		AttributeType       m_attribute;
		AttributeStripType  m_strip;
		size_t              m_offset;

		Attribute(AttributeType       attribute,
				  AttributeStripType strip,
				  size_t               offset)
		{
			m_attribute = attribute;
			m_strip = strip;
			m_offset = offset;
		}

		bool is_position_transform() const
		{
			return m_attribute == ATT_POSITION;
		}

		size_t components() const
		{
			switch (m_strip)
			{
			case AST_FLOAT:
			case AST_INT:
			case AST_UINT:
			case AST_TLESS: return 1;

			case AST_FLOAT2:
			case AST_INT2:
			case AST_UINT2:
			case AST_TLESS2: return 2;

			case AST_FLOAT3:
			case AST_INT3:
			case AST_UINT3:
			case AST_TLESS3: return 3;

			case AST_FLOAT4:
			case AST_INT4:
			case AST_UINT4:
			case AST_TLESS4: return 4;

			default: return 0; break;
			}
		}

		size_t size() const
		{
			return components() * 4;
		}
	};

	class AttributeList
	{

	public:

		using raw_attribute_list = std::vector<Attribute>;

		AttributeList() :m_size(0)
		{
		}

		AttributeList(const std::initializer_list<Attribute>& atts) :m_size(0)
		{
			for (const auto& att : atts)
			{
				push(att);
			}
		}

		AttributeList(size_t size, const std::initializer_list<Attribute>& atts) :m_size(size)
		{
			for (const auto& att : atts)
			{
				m_list.push_back(att);
			}
		}

		void push(const Attribute& attr)
		{
			m_list.push_back(attr);
			m_size += attr.size();
		}

		void size(size_t s)
		{
			m_size = s;
		}

		size_t size() const
		{
			return m_size;
		}

		size_t count_attribute() const
		{
			return m_list.size();
		}

		raw_attribute_list::iterator begin() { return m_list.begin(); }
		raw_attribute_list::iterator end() { return m_list.end(); }
		raw_attribute_list::const_iterator begin() const { return m_list.begin(); }
		raw_attribute_list::const_iterator end() const { return m_list.end(); }


	protected:

		raw_attribute_list m_list;
		size_t             m_size;

	};
	/////////////////////////////////
	// STATE
	struct ViewportState
	{

		Vec4 m_viewport;
		//costructors
		ViewportState() {}
		ViewportState(const Vec4& vp) : m_viewport(vp) {}
		//operators
		bool operator==(const ViewportState& vs) const
		{
			return m_viewport == vs.m_viewport;
		}
		bool operator!=(const ViewportState& vs) const
		{
			return m_viewport != vs.m_viewport;
		}

	};

	struct BlendState
	{
		bool      m_enable;
		BlendType m_src, m_dst;
		//constructor
		BlendState() : m_enable(false), m_src(BLEND_ONE), m_dst(BLEND_ZERO) {}
		BlendState(BlendType src, BlendType dst) : m_enable(true), m_src(src), m_dst(dst) {}
		//operators
		bool operator==(const BlendState& bs) const
		{
			return m_src == bs.m_src && m_dst == bs.m_dst && m_enable == bs.m_enable;
		}
		bool operator!=(const BlendState& bs) const
		{
			return m_src != bs.m_src || m_dst != bs.m_dst || m_enable != bs.m_enable;
		}
		//for mapping
		bool operator<(const BlendState& bs)const
		{
			int ithis = m_enable << 30    | (m_src & 0x00007FFF) << 15    | (m_dst & 0x00007FFF);
			int iobs  = bs.m_enable << 30 | (bs.m_src & 0x00007FFF) << 15 | (bs.m_dst & 0x00007FFF);
			return ithis < iobs;
		}
	};

	struct CullfaceState
	{
		//face cull
		CullfaceType m_cullface;
		//cullface
		CullfaceState(CullfaceType cullface = CF_DISABLE) : m_cullface(cullface) {}
		//operators
		bool operator==(const CullfaceState& cl)const
		{
			return m_cullface == cl.m_cullface;
		}
		bool operator!=(const CullfaceState& cl)const
		{
			return m_cullface != cl.m_cullface;
		}
		//cast operator
		operator int() const
		{
			return (int)m_cullface;
		}
		operator CullfaceType () const
		{
			return m_cullface;
		}

	};
	
	struct DepthBufferState
	{
		//value
		DepthMode     m_mode;
		DepthFuncType m_type;
		//zbuffer
		DepthBufferState(DepthFuncType type, DepthMode mode = DM_ENABLE_AND_WRITE) : m_mode(mode), m_type(type) {}
		DepthBufferState(DepthMode mode = DM_ENABLE_AND_WRITE) : m_mode(mode), m_type(DT_LESS) {}
		//operators
		bool operator==(const DepthBufferState& zb)const
		{
			return m_mode == zb.m_mode && m_type == zb.m_type;
		}
		bool operator!=(const DepthBufferState& zb)const
		{
			return m_mode != zb.m_mode || m_type != zb.m_type;
		}
		//cast operator
		operator DepthMode() const
		{
			return m_mode;
		}
		//for mapping
		bool operator<(const DepthBufferState& dbs)const
		{
			int ithis = m_mode << 16 | (m_type & 0x0000FFFF);
			int iodbs = dbs.m_mode << 16 | (dbs.m_type & 0x0000FFFF);
			return ithis < iodbs;
		}
	};

	struct ClearColorState
	{
		//color
		Vec4 m_color;
		//color
		ClearColorState() :m_color(0, 0, 0, 0) {}
		ClearColorState(const Vec4& color) :m_color(color) {}
		//operators
		bool operator==(const ClearColorState& c)const
		{
			return m_color == c.m_color;
		}
		bool operator!=(const ClearColorState& c)const
		{
			return m_color != c.m_color;
		}
		//cast operator
		operator Vec4& ()
		{
			return m_color;
		}
		operator const Vec4& () const
		{
			return m_color;
		}

	};
	
	struct State
	{
		ClearColorState   m_clear_color;
		ViewportState     m_viewport;
		CullfaceState     m_cullface;
		DepthBufferState  m_depth;
		BlendState        m_blend;

		bool operator == (const State& rs) const
		{
			return
				m_cullface == rs.m_cullface
				&& m_viewport == rs.m_viewport
				&& m_clear_color == rs.m_clear_color
				&& m_depth == rs.m_depth
				&& m_blend == rs.m_blend;
		}
		bool operator != (const State& rs) const
		{
			return
				m_cullface != rs.m_cullface
				&& m_viewport != rs.m_viewport
				&& m_clear_color != rs.m_clear_color
				&& m_depth != rs.m_depth
				&& m_blend != rs.m_blend;
		}
	};
	/////////////////////////////////
	// DRIVER INFO
	enum RenderDriver : unsigned char
	{
		DR_OPENGL,
		DR_OPENGL_ES,
		DR_VULKAN,
		DR_DIRECTX,

		DR_INVALID = 0xFF
	};

	static const char* render_driver_str[]
	{
		"OpenGL",
		"OpenGLES",
		"Vulkan",
		"DirectX"
	};

    struct RenderDriverInfo
    {
        RenderDriver  m_render_driver;
        std::string   m_name;
        int           m_major_version;
        int           m_minor_version;
        std::string   m_shader_language;
        int           m_shader_version;
    };
	/////////////////////////////////
	//Uniform global //legacy
	class Uniform
	{
	public:
		virtual void set(Texture* in_texture) = 0;
		virtual void set(int i) = 0;
        virtual void set(float f) = 0;
        virtual void set(double d) = 0;
        
        virtual void set(const IVec2& v2) = 0;
        virtual void set(const IVec3& v3) = 0;
        virtual void set(const IVec4& v4) = 0;
        
        virtual void set(const Vec2& v2) = 0;
        virtual void set(const Vec3& v3) = 0;
        virtual void set(const Vec4& v4) = 0;
        virtual void set(const Mat3& m3) = 0;
        virtual void set(const Mat4& m4) = 0;
        
        virtual void set(const DVec2& v2) = 0;
        virtual void set(const DVec3& v3) = 0;
        virtual void set(const DVec4& v4) = 0;
        virtual void set(const DMat3& m3) = 0;
        virtual void set(const DMat4& m4) = 0;

        virtual void set(Texture* t, size_t n) = 0;
        virtual void set(const int* i, size_t n) = 0;
        virtual void set(const float* f, size_t n) = 0;
        virtual void set(const double* d, size_t n) = 0;
        
        virtual void set(const IVec2* v2, size_t n) = 0;
        virtual void set(const IVec3* v3, size_t n) = 0;
        virtual void set(const IVec4* v4, size_t n) = 0;
        
        virtual void set(const Vec2* v2, size_t n) = 0;
        virtual void set(const Vec3* v3, size_t n) = 0;
        virtual void set(const Vec4* v4, size_t n) = 0;
        virtual void set(const Mat3* m3, size_t n) = 0;
        virtual void set(const Mat4* m4, size_t n) = 0;
        
        virtual void set(const DVec2* v2, size_t n) = 0;
        virtual void set(const DVec3* v3, size_t n) = 0;
        virtual void set(const DVec4* v4, size_t n) = 0;
        virtual void set(const DMat3* m3, size_t n) = 0;
        virtual void set(const DMat4* m4, size_t n) = 0;

        virtual void set(const std::vector < Texture* >& i) = 0;
        virtual void set(const std::vector < int >& i) = 0;
        virtual void set(const std::vector < float >& f) = 0;
        virtual void set(const std::vector < double >& d) = 0;
        
        virtual void set(const std::vector < IVec2 >& v2) = 0;
        virtual void set(const std::vector < IVec3 >& v3) = 0;
        virtual void set(const std::vector < IVec4 >& v4) = 0;
        
        virtual void set(const std::vector < Vec2 >& v2) = 0;
        virtual void set(const std::vector < Vec3 >& v3) = 0;
        virtual void set(const std::vector < Vec4 >& v4) = 0;
        virtual void set(const std::vector < Mat3 >& m3) = 0;
        virtual void set(const std::vector < Mat4 >& m4) = 0;
        
        virtual void set(const std::vector < DVec2 >& v2) = 0;
        virtual void set(const std::vector < DVec3 >& v3) = 0;
        virtual void set(const std::vector < DVec4 >& v4) = 0;
        virtual void set(const std::vector < DMat3 >& m3) = 0;
        virtual void set(const std::vector < DMat4 >& m4) = 0;

		virtual bool is_valid() = 0;
		virtual Shader* get_shader() = 0;
		virtual ~Uniform() {};
	};
    //buffer
    class UniformConstBuffer
    {
    public:
        //bind
        virtual void bind(const ConstBuffer*) = 0;
        virtual void unbind() = 0;
        //buffer info
        virtual bool is_valid() = 0;
        virtual Shader* get_shader() = 0;
        virtual ~UniformConstBuffer() {};
    };
    //ConstBuffer wrapper
    class MapConstBuffer
    {
    public:
        //Map Helper
        MapConstBuffer();
        //Add Value
        virtual void begin(Context* context, ConstBuffer* buffer);
        virtual void end();
        
        virtual void add(int i);
        virtual void add(float f);
        virtual void add(double d);
        
        virtual void add(const IVec2& v2);
        virtual void add(const IVec3& v3);
        virtual void add(const IVec4& v4);
        
        virtual void add(const Vec2& v2);
        virtual void add(const Vec3& v3);
        virtual void add(const Vec4& v4);
        virtual void add(const Mat3& m3);
        virtual void add(const Mat4& m4);
        
        virtual void add(const DVec2& v2);
        virtual void add(const DVec3& v3);
        virtual void add(const DVec4& v4);
        virtual void add(const DMat3& m3);
        virtual void add(const DMat4& m4);
        
        virtual void add(const int* i, size_t n);
        virtual void add(const float* f, size_t n);
        virtual void add(const double* d, size_t n);
        
        virtual void add(const IVec2* v2, size_t n);
        virtual void add(const IVec3* v3, size_t n);
        virtual void add(const IVec4* v4, size_t n);
        
        virtual void add(const Vec2* v2, size_t n);
        virtual void add(const Vec3* v3, size_t n);
        virtual void add(const Vec4* v4, size_t n);
        virtual void add(const Mat3* m3, size_t n);
        virtual void add(const Mat4* m4, size_t n);
        
        virtual void add(const DVec2* v2, size_t n);
        virtual void add(const DVec3* v3, size_t n);
        virtual void add(const DVec4* v4, size_t n);
        virtual void add(const DMat3* m3, size_t n);
        virtual void add(const DMat4* m4, size_t n);
        
        virtual void add(const std::vector < int >& i);
        virtual void add(const std::vector < float >& f);
        virtual void add(const std::vector < double >& d);
        
        virtual void add(const std::vector < IVec2 >& v2);
        virtual void add(const std::vector < IVec3 >& v3);
        virtual void add(const std::vector < IVec4 >& v4);
        
        virtual void add(const std::vector < Vec2 >& v2);
        virtual void add(const std::vector < Vec3 >& v3);
        virtual void add(const std::vector < Vec4 >& v4);
        virtual void add(const std::vector < Mat3 >& m3);
        virtual void add(const std::vector < Mat4 >& m4);
        
        virtual void add(const std::vector < DVec2 >& v2);
        virtual void add(const std::vector < DVec3 >& v3);
        virtual void add(const std::vector < DVec4 >& v4);
        virtual void add(const std::vector < DMat3 >& m3);
        virtual void add(const std::vector < DMat4 >& m4);
        
        virtual ~MapConstBuffer();
        
    protected:
        Context*     m_context;
        ConstBuffer* m_const_buffer;
        void*        m_buffer_value;
        size_t       m_offset;
    };
    
	/////////////////////////////////
	// Draw context
	class Context
	{
	public:

		Context(Allocator* allocator, Logger* logger) : m_allocator(allocator), m_logger(logger) {}

		virtual RenderDriver get_render_driver() = 0;
		virtual RenderDriverInfo get_render_driver_info() = 0;
		virtual void print_info() = 0;

		virtual bool init(Video::DeviceResources* resource) = 0;
		virtual void close() = 0;

		virtual const ClearColorState& get_clear_color_state() = 0;
		virtual void set_clear_color_state(const ClearColorState& cf) = 0;
		virtual void clear(int type = CLEAR_COLOR_DEPTH) = 0;

		virtual const DepthBufferState& get_depth_buffer_state() = 0;
		virtual void set_depth_buffer_state(const DepthBufferState& cf) = 0;

		virtual const CullfaceState& get_cullface_state() = 0;
		virtual void set_cullface_state(const CullfaceState& cf) = 0;

		virtual const ViewportState& get_viewport_state() = 0;
		virtual void set_viewport_state(const ViewportState& vs) = 0;

		virtual const BlendState& get_blend_state() = 0;
		virtual void set_blend_state(const BlendState& bs) = 0;

		virtual const State& get_render_state() = 0;
		virtual void set_render_state(const State& rs) = 0;

		//BO
		virtual ConstBuffer* create_stream_CB(const unsigned char* data, size_t size) = 0;
		virtual VertexBuffer* create_stream_VBO(const unsigned char* vbo, size_t stride, size_t n) = 0;
		virtual IndexBuffer* create_stream_IBO(const unsigned int* ibo, size_t size) = 0;

		virtual ConstBuffer* create_CB(const unsigned char* data, size_t size) = 0;
		virtual VertexBuffer* create_VBO(const unsigned char* vbo, size_t stride, size_t n) = 0;
		virtual IndexBuffer* create_IBO(const unsigned int* ibo, size_t size) = 0;

		virtual uint64 get_native_CB(const ConstBuffer*) const = 0;
		virtual uint64 get_native_VBO(const VertexBuffer*) const = 0;
		virtual uint64 get_native_IBO(const IndexBuffer*) const = 0;
        
        virtual size_t get_size_CB(const ConstBuffer*) const = 0;
        virtual size_t get_size_VBO(const VertexBuffer*) const = 0;
        virtual size_t get_size_IBO(const IndexBuffer*) const = 0;

		virtual void update_steam_CB(ConstBuffer* cb, const unsigned char* vb, size_t n) = 0;
		virtual void update_steam_VBO(VertexBuffer* vbo, const unsigned char* vb, size_t n) = 0;
		virtual void update_steam_IBO(IndexBuffer* vbo, const unsigned int* ib, size_t n) = 0;

		virtual void bind_CB(ConstBuffer*) = 0;
		virtual void bind_VBO(VertexBuffer*) = 0;
		virtual void bind_IBO(IndexBuffer*) = 0;

		virtual void unbind_CB(ConstBuffer*) = 0;
		virtual void unbind_VBO(VertexBuffer*) = 0;
		virtual void unbind_IBO(IndexBuffer*) = 0;

		virtual std::vector<unsigned char> copy_buffer_CB(const ConstBuffer*) = 0;
		virtual std::vector<unsigned char> copy_buffer_VBO(const VertexBuffer*) = 0;
		virtual std::vector<unsigned char> copy_buffer_IBO(const IndexBuffer*) = 0;

		virtual unsigned char* map_CB(ConstBuffer*, size_t start, size_t n, MappingType type) = 0;
		virtual unsigned char* map_CB(ConstBuffer*, MappingType type) = 0;
		virtual void unmap_CB(ConstBuffer*) = 0;

		virtual unsigned char* map_VBO(VertexBuffer*, size_t start, size_t n, MappingType type) = 0;
		virtual unsigned char* map_VBO(VertexBuffer*, MappingType type) = 0;
		virtual void unmap_VBO(VertexBuffer*) = 0;

		virtual unsigned int* map_IBO(IndexBuffer*, size_t start, size_t n, MappingType type) = 0;
		virtual unsigned int* map_IBO(IndexBuffer*, MappingType type) = 0;
		virtual void unmap_IBO(IndexBuffer*) = 0;

		virtual unsigned char* map_TBO(Texture*, MappingType type) = 0;
		virtual void unmap_TBO(Texture*) = 0;

		virtual void delete_CB(ConstBuffer*&) = 0;
		virtual void delete_VBO(VertexBuffer*&) = 0;
		virtual void delete_IBO(IndexBuffer*&) = 0;
		//draw
		virtual void draw_arrays(DrawType type, unsigned int n) = 0;
		virtual void draw_arrays(DrawType type, unsigned int start, unsigned int size) = 0;
		virtual void draw_elements(DrawType type, unsigned int n) = 0;
		virtual void draw_elements(DrawType type, unsigned int start, unsigned int n) = 0;

		//IL=Input Layaut
		virtual InputLayout* create_IL(const AttributeList& atl) = 0;
		virtual void delete_IL(InputLayout*&) = 0;

		virtual void bind_IL(InputLayout*) = 0;
		virtual void unbind_IL(InputLayout* il) = 0;

		virtual size_t size_IL(const InputLayout* layout) = 0;
		virtual bool   has_a_position_IL(const InputLayout* layout) = 0;
		virtual size_t position_offset_IL(const InputLayout* layout) = 0;


		//depth
		virtual float get_depth(const Vec2& pixel) const = 0;

		//color
		virtual Vec4 get_color(const Vec2& pixel) const = 0;

		//texture
		virtual Texture* create_texture
		(
			const TextureRawDataInformation& data,
			const TextureGpuDataInformation& info
		) = 0;
		virtual Texture* create_texture_array
		(
			const TextureRawDataInformation& data,
			const TextureGpuDataInformation& info,
			int   size
		) = 0;
		virtual Texture* create_cube_texture
		(
			const TextureRawDataInformation  data[6],
			const TextureGpuDataInformation& info
		) = 0;
		virtual  std::vector< unsigned char > get_texture(Texture*, int level = 0) = 0;
		virtual  std::vector< unsigned char > get_texture(Texture*, int face = 0, int level = 0) = 0;
		virtual void bind_texture(Texture*, int texture_id, int sample_id) = 0;
		virtual void unbind_texture(Texture*) = 0;
		virtual void unbind_texture(int n) = 0;
		virtual void delete_texture(Texture*&) = 0;

		//shader
		virtual Shader* create_shader(const std::vector< ShaderSourceInformation >& infos) = 0;

		virtual bool shader_compiled_with_errors(Shader* shader) = 0;
		virtual bool shader_linked_with_error(Shader* shader) = 0;
		virtual std::vector< std::string > get_shader_compiler_errors(Shader* shader) = 0;
		virtual std::string get_shader_linker_error(Shader* shader) = 0;

		virtual void bind_shader(Shader* shader) = 0;
		virtual void unbind_shader(Shader* shader) = 0;
        
        virtual void bind_uniform_CB(const ConstBuffer*, Shader*, const std::string& uname) = 0;
        virtual void unbind_uniform_CB(Shader*, const std::string& uname) = 0;

		virtual void delete_shader(Shader*&) = 0;

		virtual Shader*  get_bind_shader() const = 0;
        virtual Uniform* get_uniform(Shader*,const std::string& uname) const = 0;
        virtual UniformConstBuffer* get_uniform_const_buffer(Shader*,const std::string& uname) const = 0;

		//target
		virtual Target* create_render_target(const std::vector< TargetField >& textures) = 0;
		virtual void enable_render_target(Target*) = 0;
		virtual void disable_render_target(Target*) = 0;
		virtual void delete_render_target(Target*&) = 0;
		//copy target
		virtual void copy_target_to_target(
			const IVec4& from_area,
			Target* from,
			const IVec4& to_area,
			Target* to,
			TargetType	mask
		) = 0;
	
		//debug
		virtual bool print_errors() const = 0;
		//Output file name and line
        virtual bool print_errors(const char* source_file_name, int line) const = 0;
	
		Allocator* allocator() const { return m_allocator; }
		Logger* logger() const { return m_logger; }

	private:
		Allocator* m_allocator;
		Logger* m_logger;
	};
	/////////////////////////////////
	// Buffer smart pointer	
	DLL_EXPORT Shared<InputLayout>  input_layout(Context* ctx, const AttributeList& attrs);
	DLL_EXPORT Shared<ConstBuffer>  constant_buffer(Context* ctx,  unsigned char* data, size_t size);
	DLL_EXPORT Shared<VertexBuffer> vertex_buffer(Context* ctx, unsigned char* data, size_t stride, size_t n);
	DLL_EXPORT Shared<IndexBuffer>  index_buffer(Context* ctx, unsigned int* data, size_t n);
	DLL_EXPORT Shared<ConstBuffer>  stream_constant_buffer(Context* ctx, size_t size);
	DLL_EXPORT Shared<VertexBuffer> stream_vertex_buffer(Context* ctx, size_t stride, size_t n);
	DLL_EXPORT Shared<IndexBuffer>  stream_index_buffer(Context* ctx, size_t n);
	/////////////////////////////////
	template<class T> static inline Shared<ConstBuffer> constant_buffer(Context* ctx, unsigned char* data)
	{
		return constant_buffer(ctx, data, sizeof(T));
	}
	template<class T> static inline Shared<VertexBuffer> vertex_buffer(Context* ctx, unsigned char* data, size_t n)
	{
		return vertex_buffer(ctx, data, sizeof(T), n);
	}	
	template<class T> static inline Shared<ConstBuffer> stream_constant_buffer(Context* ctx)
	{
		return stream_constant_buffer(ctx, sizeof(T));
	}
	template<class T> static inline Shared<VertexBuffer> stream_vertex_buffer(Context* ctx, size_t n)
	{
		return stream_vertex_buffer(ctx, sizeof(T), n);
	}
	/////////////////////////////////
	template<class T> static inline void update_constant_buffer(Context* ctx, ConstBuffer* buffer, const T* data, size_t n = 1)
	{
		ctx->update_steam_CB(buffer, (const unsigned char*)data, sizeof(T) * n);
	}
	template<class T> static inline void update_vertex_buffer(Context* ctx, VertexBuffer* buffer, const T* data, size_t n = 1)
	{
		ctx->update_steam_VBO(buffer, (const unsigned char*)data, sizeof(T) * n);
	}
	static inline void update_vertex_buffer(Context* ctx, IndexBuffer* buffer, const unsigned int* data, size_t n = 1)
	{
		ctx->update_steam_IBO(buffer, data, n);
	}
	/////////////////////////////////
	template<class T> static inline  T* map_buffer(Context* ctx, ConstBuffer* buffer, size_t n = 1)
	{
		return (T*)ctx->map_CB(buffer, 0, sizeof(T) * n, Render::MAP_WRITE);
	}
	template<class T> static inline  T* map_buffer(Context* ctx, VertexBuffer* buffer, size_t n = 1)
	{
		return (T*)ctx->map_VBO(buffer, 0, sizeof(T) * n, Render::MAP_WRITE);
	}
	static inline  unsigned int* map_buffer(Context* ctx, IndexBuffer* buffer, size_t n = 1)
	{
		return (unsigned int*)ctx->map_IBO(buffer, 0, n, Render::MAP_WRITE);
	}
	static inline void unmap_buffer(Context* ctx, ConstBuffer* buffer)
	{
		ctx->unmap_CB(buffer);
	}
	static inline void unmap_buffer(Context* ctx, VertexBuffer* buffer)
	{
		ctx->unmap_VBO(buffer);
	}
	static inline void unmap_buffer(Context* ctx, IndexBuffer* buffer)
	{
		ctx->unmap_IBO(buffer);
	}
	/////////////////////////////////
	// Shared wrapper
	DLL_EXPORT std::vector<RenderDriver> list_of_render_driver();
	DLL_EXPORT Context* create_render_driver(Allocator*, Logger*, RenderDriver);
	DLL_EXPORT void delete_render_driver(Context*&);
}
}
