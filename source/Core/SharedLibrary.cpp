#include "Square/Core/SharedLibrary.h"
// os libs
#ifdef _WIN32
#include <windows.h>
#else       
#include <dlfcn.h>
#endif

namespace Square
{
namespace SharedLibrary
{
#ifdef _WIN32
	void* open(const std::string& path)
	{
		return LoadLibrary(path.c_str());
	}
	void* get(void* shared, const std::string& name)
	{
		return GetProcAddress((HMODULE)shared, name.c_str());
	}
	void  close(void* shared)
	{
		FreeLibrary((HMODULE)shared);
	}
#else
	void* open(const std::string& path)
	{
		return dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
	}
	void* get(void* shared, const std::string& name)
	{
		return dlsym(shared, name.c_str());
	}
	void  close(void* shared)
	{
		dlclose(shared);
	}
#endif
}
}