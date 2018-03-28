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
			if (auto lib = open(location + "/" + file))
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
			if (auto lib = open(location + "/" + file))
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
}
}
