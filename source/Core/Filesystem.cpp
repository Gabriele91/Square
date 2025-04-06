#include "Square/Core/Filesystem.h"
#include "Square/Core/StringUtilities.h"
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <assert.h>
#include <zlib.h>
#include <fstream>
#include <memory>

#ifdef _WIN32
	#include <windows.h>
	#include <direct.h>
	#include <io.h>
	#include <Shlwapi.h>
    #include <intrin.h>
    #define bswap32 _byteswap_ulong
	#define getcwd _getcwd
	#define access _access
    #define mkdir(x,y) _mkdir(x)
	#define F_OK 00
	#define R_OK 04
	#define W_OK 02
	#define SEPARETOR '\\'
    #define OS_MAX_PATH MAX_PATH
#else
	#include <sys/types.h>
    #include <sys/stat.h>
    #include <dirent.h>
	#include <unistd.h>
    #include <stdint.h>
    #if defined(__linux)
    #include <sys/sendfile.h> // sendfile()
    #endif
	#if defined(__APPLE__)
    #include <CoreFoundation/CoreFoundation.h>
	#include <mach-o/dyld.h>
	#include <libgen.h>
    #include <copyfile.h>  // macOS-specific
	#endif
    #define bswap32 __builtin_bswap32
	#define SEPARETOR '/'
    #define OS_MAX_PATH _POSIX_PATH_MAX
#endif

namespace Square
{
namespace Filesystem
{

	std::string program_dir()
	{
		#ifdef _WIN32
			char buffer[MAX_PATH];
			int bytes = GetModuleFileName(NULL, buffer, MAX_PATH);
			if (bytes) return get_directory(buffer);
		#elif defined(__linux)
			//path
			char buffer[PATH_MAX]{ '\0' };
			//print id
			char link[PATH_MAX];
			sprintf(link, "/proc/%d/exe", getpid());
			//get
			int bytes = std::min<int>(readlink(link, buffer, PATH_MAX), PATH_MAX - 1);
			//test
			if (bytes >= 0)
			{
				buffer[bytes] = '\0';
				return get_directory(buffer);
			}
		#elif defined(__APPLE__)
			char path[1024];
			uint32_t size = sizeof(path);
			if (_NSGetExecutablePath(path, &size) == 0)
				return get_directory(path);
		#else
			#error "os not supported"
		#endif
		//return
		return "";
	}
	
    std::string working_dir()
    {
        char buffer[255];
        char *answer = getcwd(buffer, sizeof(buffer));
        if (answer) return answer;
        return "";
    }

    std::string home_dir()
    {
        const char *home_dir = std::getenv("HOME");
        if (home_dir) return home_dir;
        else         return "";
    }
    
    std::string resource_dir()
    {
        #if defined(__APPLE__)
            char path[PATH_MAX];
            std::string bundle_path;
            std::string resources_path;
            //bundle
            CFBundleRef bundle = CFBundleGetMainBundle();
            //dir bundle
            CFURLRef bundleURL = CFBundleCopyBundleURL(bundle);
            CFStringRef bundleURLSTRING = CFURLCopyFileSystemPath( bundleURL, kCFURLPOSIXPathStyle );
            CFStringGetCString( bundleURLSTRING, path, FILENAME_MAX, kCFStringEncodingASCII );
            bundle_path = path;
            //dir rsourcea
            CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);
            CFStringRef resourcesURLSTRING = CFURLCopyFileSystemPath( resourcesURL, kCFURLPOSIXPathStyle );
            CFStringGetCString( resourcesURLSTRING, path, FILENAME_MAX, kCFStringEncodingASCII );
            resources_path = path;
            //Dealloc
            CFRelease(resourcesURLSTRING);
            CFRelease(bundleURLSTRING);
            CFRelease(bundleURL);
            CFRelease(resourcesURL);
            // Packaged into app bundle
            if (bundle_path != resources_path && bundle_path.size() && resources_path.size())
            {
                return bundle_path + SEPARETOR + resources_path + SEPARETOR;
            }
            //or return executable path
            return  program_dir();
        #else
            return working_dir();
        #endif
    }

    bool is_file(const std::string& filepath)
    {
        return  access(filepath.c_str(), F_OK) != -1 && !is_directory(filepath);
    }

    bool is_readable(const std::string& filepath)
    {
        return  access(filepath.c_str(), R_OK) != -1;
    }

    bool is_writable(const std::string& filepath)
    {
        return  access(filepath.c_str(), W_OK) != -1;
    }

    bool exists(const std::string& filepath)
    {
        #if _WIN32
            return PathFileExistsA(get_fullpath(filepath).m_path.c_str()) == TRUE;
        #else
            struct stat buf;
            return (stat(filepath.c_str(), &buf) == 0);
        #endif
    }


    //utilities files
    std::string get_directory(const std::string& filepath)
    {
        //do it?
        if (is_directory(filepath)) return filepath;
        //get separetor
        auto separetor = filepath.find_last_of("\\/");
        //test
        if (separetor == std::string::npos)
            return "";
        //return sub path
        return filepath.substr(0, separetor);
    }

    std::string get_filename(const std::string& filepath)
    {
        //copy path
        std::string filename = filepath;
        //get separetor
        auto separetor = filename.find_last_of("\\/");
        //remove rest of path
        if (separetor != std::string::npos) filename.erase(0, separetor + 1);
        //return
        return filename;
    }

    std::string get_basename(const std::string& filepath)
    {
        //copy path
        std::string filename = get_filename(filepath);
        //get dot
        auto dot = filename.rfind(".");
        //extract
        if (dot != std::string::npos) filename.erase(dot);
        //return name
        return filename;
    }

    std::string get_extension(const std::string& filepath, bool to_lower)
    {
        //copy path
        std::string filename = get_filename(filepath);
        //get dot
        auto dot = filename.rfind(".");
        //extract
        if (dot != std::string::npos)
        {
            // get ext
            auto ext = filename.substr(dot);
            // to lower?
            if (to_lower)
            {
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            }
            // return
            return ext;
        }
        //return 0
        return "";
    }

    std::vector<char> file_read_all(const std::string& filepath)
    {
        std::vector<char> out;
        /////////////////////////////////////////////////////////////////////
        FILE* file = fopen(filepath.c_str(), "rb");
        //bad case
        if (!file) return out;
        /////////////////////////////////////////////////////////////////////
        std::fseek(file, 0, SEEK_END);
        size_t size = std::ftell(file);
        std::fseek(file, 0, SEEK_SET);
        //no size:
        if(!size) std::fclose(file);
        /////////////////////////////////////////////////////////////////////
        out.resize(size, 0);
		square_assert_or_release(std::fread(&out[0], size, 1, file));
        /////////////////////////////////////////////////////////////////////
        std::fclose(file);
        //return
        return out;
    }

    std::string text_file_read_all(const std::string& filepath)
    {
        std::string out;
        /////////////////////////////////////////////////////////////////////
        FILE* file = fopen(filepath.c_str(), "r");
        //bad case
        if (!file) return "";
        /////////////////////////////////////////////////////////////////////
        std::fseek(file, 0, SEEK_END);
        size_t size = std::ftell(file);
        std::fseek(file, 0, SEEK_SET);
        //no size:
        if (!size) std::fclose(file);
        /////////////////////////////////////////////////////////////////////
        out.resize(size);
        square_assert_or_release(std::fread(&out[0], 1, size, file));
        /////////////////////////////////////////////////////////////////////
        std::fclose(file);
        //return
        return out;
    }

    std::vector<unsigned char> binary_file_read_all(const std::string& filepath)
    {
        std::vector<unsigned char> out;
        /////////////////////////////////////////////////////////////////////
        FILE* file = fopen(filepath.c_str(), "rb");
        //bad case
        if (!file) return out;
        /////////////////////////////////////////////////////////////////////
        std::fseek(file, 0, SEEK_END);
        size_t size = std::ftell(file);
        std::fseek(file, 0, SEEK_SET);
        //no size:
        if (!size) std::fclose(file);
        /////////////////////////////////////////////////////////////////////
        out.resize(size, 0);
        square_assert_or_release(std::fread(&out[0], size, 1, file));
        /////////////////////////////////////////////////////////////////////
        std::fclose(file);
        //return
        return out;
    }

    static inline int is_big_endian()
    {
        int i = 1;
        return !*((char*)&i);
    }

    static uint32_t get_uncompressed_size(const std::string& filepath)
    {
        /////////////////////////////////////////////////////////////////////
        FILE* file = fopen(filepath.c_str(), "rb");
        //bad case
        if (!file) return 0;
        /////////////////////////////////////////////////////////////////////
        if(fseek(file, -4, SEEK_END) != 0) { fclose(file);  return 0; }
        //read the last 4 bytes
        uint32_t uncompressed_size = 0;
        if (fread(&uncompressed_size, sizeof(uncompressed_size), 1, file) != 1) { fclose(file);  return 0; }
        //gzip stores the size in little-endian format, so convert if necessary
        if (is_big_endian()) uncompressed_size = bswap32(uncompressed_size);
        //ok
        fclose(file);
        return uncompressed_size;
    }

    std::vector<unsigned char> binary_compress_file_read_all(const std::string& filepath)
    {
        std::vector<unsigned char> out;
        //get size
        long uncompressed_size = get_uncompressed_size(filepath);
        //no size:
        if (!uncompressed_size) { return out; }
        /////////////////////////////////////////////////////////////////////
        gzFile file = gzopen(filepath.c_str(), "rb");
        //bad case
        if (!file) return out;
        /////////////////////////////////////////////////////////////////////
        out.resize(uncompressed_size, 0);
        square_assert_or_release(gzread(file, &out[0], uncompressed_size));
        /////////////////////////////////////////////////////////////////////
        gzclose(file);
        //return
        return out;
    }

    bool file_write_all(const std::string& filepath, const std::vector<char>& buffer)
    {
        /////////////////////////////////////////////////////////////////////
        FILE* file = fopen(filepath.c_str(), "wb");
        //bad case
        if (!file) return false;
        /////////////////////////////////////////////////////////////////////
        bool has_been_write = !!std::fwrite(buffer.data(), buffer.size(), 1, file);
        /////////////////////////////////////////////////////////////////////
        std::fclose(file);
        //return
        return has_been_write;
    }

    bool text_file_write_all(const std::string& filepath, const std::string& buffer)
    {
        /////////////////////////////////////////////////////////////////////
        FILE* file = std::fopen(filepath.c_str(), "w");
        //bad case
        if (!file) return false;
        /////////////////////////////////////////////////////////////////////
        bool has_been_write = !!std::fwrite(buffer.data(), buffer.size(), 1, file);
        /////////////////////////////////////////////////////////////////////
        std::fclose(file);
        //return
        return has_been_write;
    }
    
    bool binary_file_write_all(const std::string& filepath, const std::vector<unsigned char>& buffer)
    {
        /////////////////////////////////////////////////////////////////////
        FILE* file = std::fopen(filepath.c_str(), "wb");
        //bad case
        if (!file) return false;
        /////////////////////////////////////////////////////////////////////
        bool has_been_write = !!std::fwrite(buffer.data(), buffer.size(), 1, file);
        /////////////////////////////////////////////////////////////////////
        std::fclose(file);
        //return
        return has_been_write;
    }

    bool binary_compress_file_write_all(const std::string& filepath, const std::vector<unsigned char>& buffer)
    {
        /////////////////////////////////////////////////////////////////////
        gzFile file = gzopen(filepath.c_str(), "wb");
        //bad case
        if (!file) return false;
        /////////////////////////////////////////////////////////////////////
        bool has_been_write = !!gzwrite(file, buffer.data(), static_cast<unsigned int>(buffer.size()));
        /////////////////////////////////////////////////////////////////////
        gzclose(file);
        //return
        return has_been_write;
    }

    PathOperation get_fullpath(const std::string& relative)
    {
#ifdef _WIN32
        char fullpath[MAX_PATH];
        if (GetFullPathNameA(relative.c_str(), MAX_PATH, fullpath, 0) == 0) return PathOperation{ false, "" };
        return PathOperation{ true, std::string(fullpath) };
#else
        char realname[_POSIX_PATH_MAX];
        if (realpath(relative.c_str(), realname) == 0) return PathOperation{ false, "" };
        return PathOperation{ true, std::string(realname) };
#endif
    }

    SQUARE_API PathOperation get_canonical(const std::string& fullpath)
    {
        // Path output
        char path[OS_MAX_PATH]{ 0 };

#if defined( _WIN32 )
        std::string fullpath_win32 = replace_all(fullpath, "/", "\\");
        strncpy(path, fullpath_win32.c_str(), (std::min<size_t>)(fullpath_win32.size(), OS_MAX_PATH));
#else 
        strncpy(path, fullpath.c_str(), (std::min<size_t>)(fullpath.size(), OS_MAX_PATH));
#endif

        char* src { 0 };
        char* dst { 0 };
        char c;
        int  slash = 0;

        /* Convert multiple adjacent slashes to single slash */
        src = dst = path;
        while ((c = *dst++ = *src++) != '\0')
        {
            if (c == SEPARETOR)
            {
                slash = 1;
                while (*src == SEPARETOR)
                    src++;
            }
        }

        if (slash == 0)
            return PathOperation{ false, "" };

        /* Remove "./" from "./xxx" but leave "./" alone. */
        /* Remove "/." from "xxx/." but reduce "/." to "/". */
        /* Reduce "xxx/./yyy" to "xxx/yyy" */
        src = dst = (*path == SEPARETOR) ? path + 1 : path;
        while (src[0] == '.' && src[1] == SEPARETOR && src[2] != '\0')
            src += 2;
        while ((c = *dst++ = *src++) != '\0')
        {
            if (c == SEPARETOR && src[0] == '.' && (src[1] == '\0' || src[1] == SEPARETOR))
            {
                src++;
                dst--;
            }
        }
        if (path[0] == SEPARETOR && path[1] == '.' &&
            (path[2] == '\0' || (path[2] == SEPARETOR && path[3] == '\0')))
            path[1] = '\0';

        /* Remove trailing slash, if any.  There is at most one! */
        /* dst is pointing one beyond terminating null */
        if ((dst -= 2) > path && *dst == SEPARETOR)
            *dst++ = '\0';
        
        // Ok
        return PathOperation{ true, std::string(path) };
    }

    std::vector<std::string> split(std::string str, const std::string& delimiter)
    {
        //temps
        size_t pos = 0;
        std::vector<std::string> tokens;
        //search
        while ((pos = str.find(delimiter)) != std::string::npos)
        {
            tokens.push_back(str.substr(0, pos));
            str.erase(0, pos + delimiter.length());
        }
        //add last
        if (str.size()) tokens.push_back(str);
        //return
        return tokens;
    }

    PathOperation get_relative_to(const std::string& base, const std::string& path)
    {
        //get abs paths
        PathOperation absolute_base = get_fullpath(base); if (!absolute_base.m_success) return PathOperation{ false, "" };
        PathOperation absolute_path = get_fullpath(path); if (!absolute_path.m_success) return PathOperation{ false, "" };
#ifdef _WIN32
        char output_path[MAX_PATH];

        if (PathRelativePathToA(output_path,
            absolute_base.m_path.c_str(),
            FILE_ATTRIBUTE_DIRECTORY,
            absolute_path.m_path.c_str(),
            FILE_ATTRIBUTE_NORMAL) == 0) return PathOperation{ false, "" };

        // Remove .\"" from relative path .\\"<disk>:"
        std::string relative(output_path);
        std::string::size_type dquote1 = relative.find('\"');
        std::string::size_type dquote2 = relative.rfind('\"');
        if (dquote1 != std::string::npos && 
            dquote2 != std::string::npos && 
            dquote1 != dquote2)
        {
            relative = relative.substr(dquote1 + 1, dquote2 - dquote1 -1);
        }
        // Return
        return PathOperation{ true, std::move(relative) };
#else
        std::string output_path;

        //split
        std::string separetor(SEPARETOR);
        std::vector< std::string > base_directories = split(absolute_base.m_path, separetor);
        std::vector< std::string > path_directories = split(absolute_path.m_path, separetor);

        //max loop
        size_t max_size = std::min(base_directories.size(), path_directories.size());

        //start index
        size_t i = 0;

        //search dif
        for (i = 0; i != max_size; ++i)
        {
            if (base_directories[i] != path_directories[i]) break;
        }

        //back
        for (size_t j = 0; j < (max_size - i); ++i) output_path += "../";

        //next
        for (i = max_size; i < path_directories.size(); ++i)
        {
            output_path += path_directories[i] + SEPARETOR;
        }

        //ok
        return PathOperation{ true, output_path };
#endif
    }

	std::string join(const std::string& path1, const std::string& path2)
	{
		if (!path1.size()) return path1;
		if (!path2.size()) return path2;
		return path1 + SEPARETOR + path2;
	}

    bool makedir(const std::string& path)
    {
        return mkdir(path.c_str(), 0777) == 0;
    }
    
    bool copyfile(const std::string& infilepath, const std::string& ofilepath)
    {
        #if defined(_WIN32)
            return !!CopyFile(infilepath.c_str(), ofilepath.c_str(), false);
        #elif defined(__APPLE__)
            return copyfile(infilepath.c_str(), ofilepath.c_str(), NULL, COPYFILE_ALL) != 0;
        #elif defined(__linux)
            struct stat src_stat;
            // Open source file
            fd_src = open(infilepath.c_str(), O_RDONLY);
            if (fd_src == -1) 
            {
                return false;
            }

            // Get source file stats
            if (fstat(fd_src, &src_stat) == -1) 
            {
                close(fd_src);
                return false;
            }

            // Open destination file
            fd_dest = open(ofilepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode);
            if (fd_dest == -1)
            {
                close(fd_src);
                return false;
            }

            // Use sendfile to copy
            off_t offset = 0;
            if (sendfile(fd_dest, fd_src, &offset, src_stat.st_size) == -1)
            {
                close(fd_src);
                close(fd_dest);
                return false;
            }

            // Close files
            close(fd_src);
            close(fd_dest);
            return true;
        #else
            std::ifstream ifilestream(infilepath, std::ios::in | std::ios::binary);
            std::ofstream ofilestream(ofilepath, std::ios::out | std::ios::binary);

            if (ifilestream.good() && ofilestream.good())
            {
                std::copy(std::istream_iterator<char>(ifilestream),
                          std::istream_iterator<char>(),
                          std::ostream_iterator<char>(ofilestream));
                return ofilestream.good();
            }
            return false;
        #endif
    }


#ifdef _WIN32
    bool is_directory(const std::string& directory)
    {
        DWORD ftyp = GetFileAttributesA(directory.c_str());
        if (ftyp == INVALID_FILE_ATTRIBUTES) return false;
        if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;
        return false;
    }

    FilesList get_files(const std::string& directorypath)
    {
        //test directory
        if (!is_directory(directorypath)) return FilesList{ false, std::vector<std::string>{} };
        //alloc output
        FilesList output{ true, std::vector<std::string>{} };
        //redefine path
        std::string std_directorypath = directorypath + SEPARETOR;
        std::string path_all = std_directorypath + "*";
        //attributes
        WIN32_FIND_DATAA ffd;
        //struct stat st;
        HANDLE hFind = FindFirstFileA(path_all.c_str(), &ffd);
        //read all
        do
        {
            if (!std::strcmp(ffd.cFileName, ".")) continue;
            if (!std::strcmp(ffd.cFileName, "..")) continue;
            //put into vector only files
            if (is_file(std_directorypath + ffd.cFileName))
            {
                output.m_fields.push_back(ffd.cFileName);
            }

        } while (FindNextFileA(hFind, &ffd) != 0);
        //return output
        return output;
    }

    DirectoriesList get_sub_directories(const std::string& directorypath)
    {
        //test directory
        if (!is_directory(directorypath)) return DirectoriesList{ false, std::vector<std::string>{} };
        //alloc output
        DirectoriesList output{ true, std::vector<std::string>{} };
        //redefine path
        std::string std_directorypath = directorypath + SEPARETOR;
        std::string path_all = std_directorypath + "*";
        //attributes
        WIN32_FIND_DATAA ffd;
        //struct stat st;
        HANDLE hFind = FindFirstFileA(path_all.c_str(), &ffd);
        //read all
        do
        {
            if (!std::strcmp(ffd.cFileName, ".")) continue;
            if (!std::strcmp(ffd.cFileName, "..")) continue;
            //put into vector only dirs
            if (is_directory(std_directorypath + ffd.cFileName))
            {
                output.m_fields.push_back(ffd.cFileName);
            }

        } while (FindNextFileA(hFind, &ffd) != 0);
        //return output
        return output;
    }

#else
    bool is_directory(const std::string& directory)
    {
        struct stat st;
        if (stat(directory.c_str(), &st) == 0) return ((st.st_mode & S_IFDIR) != 0);
        return false;
    }

    FilesList get_files(const std::string& directorypath)
    {
        //test directory
        if (!is_directory(directorypath)) return FilesList{ false, std::vector<std::string>{} };
        //attributes
        DIR *directory;
        struct dirent *dirent;
        //open dir
        if ((directory = opendir(directorypath.c_str())) == NULL) return FilesList{ false, std::vector<std::string>{} };
        //alloc output
        FilesList output{ true, std::vector<std::string>{} };
        //redefine path
        std::string std_directorypath = directorypath + SEPARETOR;
        //read all elements
        while ((dirent = readdir(directory)) != NULL)
        {
            if (!std::strcmp(dirent->d_name, ".")) continue;
            if (!std::strcmp(dirent->d_name, "..")) continue;
            //put into vector only files
            if (is_file(std_directorypath + dirent->d_name))
            {
                output.m_fields.push_back(dirent->d_name);
            }
        }
        //close directory
        closedir(directory);
        //return output
        return output;
    }

    DirectoriesList get_sub_directories(const std::string& directorypath)
    {
        //test directory
        if (!is_directory(directorypath)) return DirectoriesList{ false, std::vector<std::string>{} };
        //attributes
        DIR *directory;
        struct dirent *dirent;
        //open dir
        if ((directory = opendir(directorypath.c_str())) == NULL) return DirectoriesList{ false, std::vector<std::string>{} };
        //alloc output
        DirectoriesList output{ true, std::vector<std::string>{} };
        //redefine path
        std::string std_directorypath = directorypath + SEPARETOR;
        //read all elements
        while ((dirent = readdir(directory)) != NULL)
        {
            if (!std::strcmp(dirent->d_name, ".")) continue;
            if (!std::strcmp(dirent->d_name, "..")) continue;
            //put into vector only dirs
            if (is_directory(std_directorypath + dirent->d_name))
            {
                output.m_fields.push_back(dirent->d_name);
            }
        }
        //close directory
        closedir(directory);
        //return output
        return output;
    }
#endif

namespace Stream
{
#pragma region GZStreambuf
    GZStreambuf::GZStreambuf() : file_(nullptr)
    {
        setg(buffer_, buffer_ + 4, buffer_ + 4);  // Initialize get area
    }

    GZStreambuf::~GZStreambuf()
    {
        close();
    }

    bool GZStreambuf::open(const std::string& filename, const char* mode)
    {
        close();  // Close any previously opened file
        file_ = gzopen(filename.c_str(), mode);
        if (!file_)
        {
            throw std::runtime_error("Failed to open gzip file");
        }
        return true;
    }

    void GZStreambuf::close()
    {
        if (file_)
        {
            gzclose((::gzFile)file_);
            file_ = nullptr;
        }
    }

    int GZStreambuf::overflow(int ch)
    {
        if (ch != EOF)
        {
            char c = static_cast<char>(ch);
            if (gzwrite((::gzFile)file_, &c, 1) != 1)
            {
                return EOF;
            }
        }
        return ch;
    }

    std::streamsize GZStreambuf::xsputn(const char* s, std::streamsize count)
    {
        return gzwrite((::gzFile)file_, s, static_cast<unsigned int>(count));
    }

    int GZStreambuf::underflow()
    {
        if (gptr() < egptr())
        {
            return traits_type::to_int_type(*gptr());
        }

        int num = gzread((::gzFile)file_, buffer_, sizeof(buffer_));
        if (num <= 0)
        {
            return EOF;
        }

        setg(buffer_, buffer_, buffer_ + num);
        return traits_type::to_int_type(*gptr());
    }
#pragma endregion

#pragma region GZOStream
    GZOStream::GZOStream() : std::ostream(&buf_) {}

    GZOStream::GZOStream(const std::string& filename) : std::ostream(&buf_)
    {
        if (!buf_.open(filename, "wb"))
        {
            throw std::runtime_error("Failed to open gzip file");
        }
    }

    void GZOStream::open(const std::string& filename)
    {
        if (!buf_.open(filename, "wb"))
        {
            throw std::runtime_error("Failed to open gzip file");
        }
    }

    void GZOStream::close()
    {
        buf_.close();
    }
#pragma endregion

#pragma region GZOStream
    GZIStream::GZIStream() : std::istream(&buf_) {}

    GZIStream::GZIStream(const std::string& filename) : std::istream(&buf_)
    {
        if (!buf_.open(filename, "rb"))
        {
            throw std::runtime_error("Failed to open gzip file");
        }
    }

    void GZIStream::open(const std::string& filename)
    {
        if (!buf_.open(filename, "rb"))
        {
            throw std::runtime_error("Failed to open gzip file");
        }
    }

    void GZIStream::close()
    {
        buf_.close();
    }
#pragma endregion
}
}
}
