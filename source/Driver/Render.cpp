//include
#include <unordered_map>
#include "Square/Config.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/SharedLibrary.h"
#include "Square/Driver/Render.h"

namespace Square
{
namespace Render
{
	using square_render_get_type = Square::Render::RenderDriver(*)();
	using square_render_create_context = Square::Render::Context* (*)();
	using square_render_delete_context = void(*)(Square::Render::Context*&);

	//help
	Shared<InputLayout>  input_layout(Context* ctx, const AttributeList& attrs)
	{
		InputLayout* buffer = ctx->create_IL(attrs);
		return std::shared_ptr<InputLayout>(buffer, [=](InputLayout* ptr) { ctx->delete_IL(ptr); });
	}
	Shared<ConstBuffer>  constant_buffer(Context* ctx, unsigned char* data, size_t size)
	{
		ConstBuffer* buffer = ctx->create_CB(data, size);
		return std::shared_ptr<ConstBuffer>(buffer, [=](ConstBuffer* ptr) { ctx->delete_CB(ptr); });
	}
	Shared<VertexBuffer> vertex_buffer(Context* ctx, unsigned char* data, size_t stride, size_t n)
	{
		VertexBuffer* buffer = ctx->create_VBO(data, stride, n);
		return std::shared_ptr<VertexBuffer>(buffer, [=](VertexBuffer* ptr) { ctx->delete_VBO(ptr); });
	}
	Shared<IndexBuffer> index_buffer(Context* ctx, unsigned int* data, size_t n)
	{
		IndexBuffer* buffer = ctx->create_IBO(data, n);
		return std::shared_ptr<IndexBuffer>(buffer, [=](IndexBuffer* ptr) { ctx->delete_IBO(ptr); });
	}

	Shared<ConstBuffer>  stream_constant_buffer(Context* ctx, size_t size)
	{
		ConstBuffer* buffer = ctx->create_stream_CB(nullptr, size);
		return std::shared_ptr<ConstBuffer>(buffer, [=](ConstBuffer* ptr) { ctx->delete_CB(ptr); });
	}
	Shared<VertexBuffer> stream_vertex_buffer(Context* ctx, size_t stride, size_t n)
	{
		VertexBuffer* buffer = ctx->create_stream_VBO(nullptr, stride, n);
		return std::shared_ptr<VertexBuffer>(buffer, [=](VertexBuffer* ptr) { ctx->delete_VBO(ptr); });
	}
	Shared<IndexBuffer> stream_index_buffer(Context* ctx, size_t n)
	{
		IndexBuffer* buffer = ctx->create_stream_IBO(nullptr, n);
		return std::shared_ptr<IndexBuffer>(buffer, [=](IndexBuffer* ptr) { ctx->delete_IBO(ptr); });
	}

	//list
	std::vector<RenderDriver> list_of_render_driver()
	{
		//usings
		using namespace Square;
		using namespace Square::Filesystem;
		using namespace Square::SharedLibrary;
		//locs
		const std::string locations[] =
		{
			program_dir(),
			working_dir(),
            resource_dir(),
			home_dir(),
		};
		//output
		std::vector<RenderDriver> out;
		//try
		for (auto location : locations)
		if  (location.size())
		for (auto file : get_files(location))
		{
			auto extension = get_extension(file);
			if (extension == ".dll"  || extension == ".so"  || extension == ".dylib")
			if (auto lib = open(Filesystem::join(location, file)))
			{
				//get function
				if (auto get_type = (square_render_get_type)SharedLibrary::get(lib, "square_render_get_type"))
				{
					out.push_back(get_type());
				}
				close(lib);
			}
		}
		return out;
	}
	
	//alloc map
	static std::unordered_map<Context*, void*>& get_lib_map()
	{
		static std::unordered_map<Context*, void*> ctx_lib_map;
		return ctx_lib_map;
	}

	//alloc
	Context* create_render_driver(RenderDriver type)
	{		
		//usings
		using namespace Square;
		using namespace Square::Filesystem;
		using namespace Square::SharedLibrary;
		//locs
		const std::string locations[] =
		{
			program_dir(),
			working_dir(),
            resource_dir(),
			home_dir(),
		};
		//context
		Context* context = nullptr;
		//try
		for (auto location : locations)
		if  (location.size())
		for (auto file : get_files(location))
		{
			auto extension = get_extension(file);
			if (extension == ".dll"  || extension == ".so"  || extension == ".dylib")
			if (auto lib = open(Filesystem::join(location, file)))
			{
				//get function
				if (auto get_type = (square_render_get_type)SharedLibrary::get(lib, "square_render_get_type"))
				if (get_type() == type)
				if (auto create_context = (square_render_create_context)SharedLibrary::get(lib, "square_render_create_context"))
				{
					Context* context = create_context();
					get_lib_map()[context] = lib;
					return context;
				}
				close(lib);
			}
		}
		return context;
	}

	//dealloc
	void delete_render_driver(Context*& context)
	{
		//usings
		using namespace Square;
		using namespace Square::Filesystem;
		using namespace Square::SharedLibrary;
		//lib iterator
		auto lib_iterator = get_lib_map().find(context);
		//test
		if (lib_iterator != get_lib_map().end())
		{
			//lib ptr
			void* lib = lib_iterator->second;
			//delete
			if (auto delete_context = (square_render_delete_context)SharedLibrary::get(lib, "square_render_delete_context"))
			{
				delete_context(context);
			}
			//close
			close(lib);
			//erase
			get_lib_map().erase(lib_iterator);
		}
	}
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //help const buffer
    MapConstBuffer::MapConstBuffer()
    {
        m_context      = nullptr;
        m_const_buffer = nullptr;
        m_buffer_value = nullptr;
        m_offset = 0;
    }
    //buffer
    void MapConstBuffer::begin(Context* ctx,ConstBuffer* buffer)
    {
        m_context = ctx;
        m_const_buffer = buffer;
        m_buffer_value = m_context->map_CB(buffer,0, m_context->get_size_CB(buffer),MappingType::MAP_WRITE_AND_READ);
        m_offset = 0;
    }
    void MapConstBuffer::end()
    {
        m_context->unmap_CB(m_const_buffer);
        m_const_buffer = nullptr;
        m_buffer_value = nullptr;
        m_offset = 0;
    }

    #define CBUFFERPTR (((char*)m_buffer_value)+m_offset)
    #define COMPUTEOFFEST(type) (sizeof(type) % 16 > 0 ? 16 * (sizeof(type) / 16 + 1) : sizeof(type))
    
    void MapConstBuffer::add(int i)
    {
        *((int*)CBUFFERPTR) = i;
        m_offset += COMPUTEOFFEST(int);
    }
    void MapConstBuffer::add(float f)
    {
        *((float*)CBUFFERPTR) = f;
        m_offset += COMPUTEOFFEST(float);
    }
    void MapConstBuffer::add(double d)
    {
        *((double*)CBUFFERPTR) = d;
        m_offset += COMPUTEOFFEST(double);
    }
        
    void MapConstBuffer::add(const IVec2& v2)
    {
        *((IVec2*)CBUFFERPTR) = v2;
        m_offset += COMPUTEOFFEST(IVec2);
    }
    void MapConstBuffer::add(const IVec3& v3)
    {
        *((IVec3*)CBUFFERPTR) = v3;
        m_offset += COMPUTEOFFEST(IVec3);
    }
    void MapConstBuffer::add(const IVec4& v4)
    {
        *((IVec4*)CBUFFERPTR) = v4;
        m_offset += COMPUTEOFFEST(IVec4);
    }
        
    void MapConstBuffer::add(const Vec2& v2)
    {
        *((Vec2*)CBUFFERPTR) = v2;
        m_offset += COMPUTEOFFEST(IVec2);
    }
    void MapConstBuffer::add(const Vec3& v3)
    {
        *((Vec3*)CBUFFERPTR) = v3;
        m_offset += COMPUTEOFFEST(Vec3);
    }
    void MapConstBuffer::add(const Vec4& v4)
    {
        *((Vec4*)CBUFFERPTR) = v4;
        m_offset += COMPUTEOFFEST(Vec4);
    }
    void MapConstBuffer::add(const Mat3& m3)
    {
        *((Mat3*)CBUFFERPTR) = m3;
        m_offset += COMPUTEOFFEST(Mat3);
    }
    void MapConstBuffer::add(const Mat4& m4)
    {
        *((Mat4*)CBUFFERPTR) = m4;
        m_offset += COMPUTEOFFEST(Mat4);
    }
        
    void MapConstBuffer::add(const DVec2& v2)
    {
        *((DVec2*)CBUFFERPTR) = v2;
        m_offset += COMPUTEOFFEST(DVec2);
    }
    void MapConstBuffer::add(const DVec3& v3)
    {
        *((DVec3*)CBUFFERPTR) = v3;
        m_offset += COMPUTEOFFEST(DVec3);
    }
    void MapConstBuffer::add(const DVec4& v4)
    {
        *((DVec4*)CBUFFERPTR) = v4;
        m_offset += COMPUTEOFFEST(DVec4);
    }
    void MapConstBuffer::add(const DMat3& m3)
    {
        *((DMat3*)CBUFFERPTR) = m3;
        m_offset += COMPUTEOFFEST(DMat3);
    }
    void MapConstBuffer::add(const DMat4& m4)
    {
        *((DMat4*)CBUFFERPTR) = m4;
        m_offset += COMPUTEOFFEST(DMat4);
    }
        
    void MapConstBuffer::add(const int* i, size_t n)
    {
        while(n--){ add(*i); i++; }
    }
    void MapConstBuffer::add(const float* f, size_t n)
    {
        while(n--){ add(*f); f++; }
    }
    void MapConstBuffer::add(const double* d, size_t n)
    {
        while(n--){ add(*d); d++; }
    }
    
    void MapConstBuffer::add(const IVec2* v2, size_t n)
    {
        while(n--){ add(*v2); v2++; }
    }
    void MapConstBuffer::add(const IVec3* v3, size_t n)
    {
        while(n--){ add(*v3); v3++; }
    }
    void MapConstBuffer::add(const IVec4* v4, size_t n)
    {
        while(n--){ add(*v4); v4++; }
    }
    
    void MapConstBuffer::add(const Vec2* v2, size_t n)
    {
        while(n--){ add(*v2); v2++; }
    }
    void MapConstBuffer::add(const Vec3* v3, size_t n)
    {
        while(n--){ add(*v3); v3++; }
    }
    void MapConstBuffer::add(const Vec4* v4, size_t n)
    {
        while(n--){ add(*v4); v4++; }
    }
    void MapConstBuffer::add(const Mat3* m3, size_t n)
    {
        while(n--){ add(*m3); m3++; }
    }
    void MapConstBuffer::add(const Mat4* m4, size_t n)
    {
        while(n--){ add(*m4); m4++; }
    }
    
    void MapConstBuffer::add(const DVec2* v2, size_t n)
    {
        while(n--){ add(*v2); v2++; }
    }
    void MapConstBuffer::add(const DVec3* v3, size_t n)
    {
        while(n--){ add(*v3); v3++; }
    }
    void MapConstBuffer::add(const DVec4* v4, size_t n)
    {
        while(n--){ add(*v4); v4++; }
    }
    void MapConstBuffer::add(const DMat3* m3, size_t n)
    {
        while(n--){ add(*m3); m3++; }
    }
    void MapConstBuffer::add(const DMat4* m4, size_t n)
    {
        while(n--){ add(*m4); m4++; }
    }
    
    void MapConstBuffer::add(const std::vector < int >& i)
    {
        add(i.data(),i.size());
    }
    void MapConstBuffer::add(const std::vector < float >& f)
    {
        add(f.data(),f.size());
    }
    void MapConstBuffer::add(const std::vector < double >& d)
    {
        add(d.data(),d.size());
    }
    
    void MapConstBuffer::add(const std::vector < IVec2 >& v2)
    {
        add(v2.data(),v2.size());
    }
    void MapConstBuffer::add(const std::vector < IVec3 >& v3)
    {
        add(v3.data(),v3.size());
    }
    void MapConstBuffer::add(const std::vector < IVec4 >& v4)
    {
        add(v4.data(),v4.size());
    }
    
    void MapConstBuffer::add(const std::vector < Vec2 >& v2)
    {
        add(v2.data(),v2.size());
    }
    void MapConstBuffer::add(const std::vector < Vec3 >& v3)
    {
        add(v3.data(),v3.size());
    }
    void MapConstBuffer::add(const std::vector < Vec4 >& v4)
    {
        add(v4.data(),v4.size());
    }
    void MapConstBuffer::add(const std::vector < Mat3 >& m3)
    {
        add(m3.data(),m3.size());
    }
    void MapConstBuffer::add(const std::vector < Mat4 >& m4)
    {
        add(m4.data(),m4.size());
    }
    
    void MapConstBuffer::add(const std::vector < DVec2 >& v2)
    {
        add(v2.data(),v2.size());
    }
    void MapConstBuffer::add(const std::vector < DVec3 >& v3)
    {
        add(v3.data(),v3.size());
    }
    void MapConstBuffer::add(const std::vector < DVec4 >& v4)
    {
        add(v4.data(),v4.size());
    }
    void MapConstBuffer::add(const std::vector < DMat3 >& m3)
    {
        add(m3.data(),m3.size());
    }
    void MapConstBuffer::add(const std::vector < DMat4 >& m4)
    {
        add(m4.data(),m4.size());
    }
        
    MapConstBuffer::~MapConstBuffer()
    {
        if(m_const_buffer) end();
    }
    #undef CBUFFERPTR
    #undef COMPUTEOFFEST
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
