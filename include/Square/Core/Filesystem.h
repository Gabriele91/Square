//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"

namespace Square
{
namespace Filesystem
{
	SQUARE_API std::string program_dir();
    SQUARE_API std::string working_dir();
    SQUARE_API std::string home_dir();
    SQUARE_API std::string resource_dir();

    SQUARE_API bool is_directory(const std::string& directorypath);
    SQUARE_API bool is_file(const std::string& filepath);
    SQUARE_API bool is_readable(const std::string& filepath);
    SQUARE_API bool is_writable(const std::string& filepath);
    SQUARE_API bool exists(const std::string& filepath);
    //utilities files
    SQUARE_API std::string get_directory(const std::string& filepath);
    SQUARE_API std::string get_filename(const std::string& filepath);
    SQUARE_API std::string get_basename(const std::string& filepath);
    SQUARE_API std::string get_extension(const std::string& filepath, bool to_lower = true);
    SQUARE_API std::string join(const std::string& path1, const std::string& path2);
    //read
    SQUARE_API std::vector<char> file_read_all(const std::string& filepath);    
    SQUARE_API std::string text_file_read_all(const std::string& filepath);
    SQUARE_API std::vector<unsigned char> binary_file_read_all(const std::string& filepath);
    SQUARE_API std::vector<unsigned char> binary_compress_file_read_all(const std::string& filepath);
    //write
    SQUARE_API bool file_write_all(const std::string& filepath, const std::vector<char>& buffer);
    SQUARE_API bool text_file_write_all(const std::string& filepath, const std::string& buffer);
    SQUARE_API bool binary_file_write_all(const std::string& filepath, const std::vector<unsigned char>& buffer);
    SQUARE_API bool binary_compress_file_write_all(const std::string& filepath, const std::vector<unsigned char>& buffer);

    //shell helps
    SQUARE_API bool makedir(const std::string& path);
    SQUARE_API bool copyfile(const std::string& infilepath, const std::string& ofilepath);


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // template help
    template < class ...Args >
    static inline std::string join(const std::string& path1, const std::string& path2, Args... args) { return join(path1, join(path2, args...)); }
    /////////////////////////////////////////////////////////////////////////////////////////////////

    //utilities path
    struct PathOperation
    {
        bool m_success;
        std::string m_path;
    };
    SQUARE_API PathOperation get_fullpath(const std::string& relative);
    SQUARE_API PathOperation get_canonical(const std::string& fullpath);
    SQUARE_API PathOperation get_relative_to(const std::string& base, const std::string& path);
    //utilities directories
    struct FolderList
    {
        bool m_success;
        std::vector < std::string > m_fields;
        //iterator
        std::vector < std::string >::iterator begin(){ return m_fields.begin(); }
        std::vector < std::string >::iterator end()  { return m_fields.end(); }
        std::vector < std::string >::const_iterator begin() const { return m_fields.begin(); }
        std::vector < std::string >::const_iterator end()   const { return m_fields.end(); }
    };
    using FilesList       = FolderList;
    using DirectoriesList = FolderList;

    SQUARE_API FilesList       get_files(const std::string& directorypath);
    SQUARE_API DirectoriesList get_sub_directories(const std::string& directorypath);
    
    namespace Stream
    {
        class SQUARE_API GZStreambuf : public std::streambuf
        {
            using gzFile = void*;
            static const size_t buffer_size = 8192;
        public:
            GZStreambuf();
            ~GZStreambuf() override;

            bool open(const std::string& filename, const char* mode);
            void close();

        protected:
            int overflow(int ch) override;
            std::streamsize xsputn(const char* s, std::streamsize count) override;
            int underflow() override;

        private:
            gzFile file_;
            char buffer_[buffer_size];
        };

        class SQUARE_API GZOStream : public std::ostream
        {
        public:
            GZOStream();
            explicit GZOStream(const std::string& filename);
            void open(const std::string& filename);
            void close();

        private:
            GZStreambuf buf_;
        };

        class SQUARE_API GZIStream : public std::istream
        {
        public:
            GZIStream();
            explicit GZIStream(const std::string& filename);
            void open(const std::string& filename);
            void close();

        private:
            GZStreambuf buf_;
        };
    }
}
}
