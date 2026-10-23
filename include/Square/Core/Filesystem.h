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
	SQUARE_API String program_dir();
    SQUARE_API String working_dir();
    SQUARE_API String home_dir();
    //the folder of the data of the applications of the user (settings, saves): Windows
    //%APPDATA%, macOS ~/Library/Application Support, Linux $XDG_DATA_HOME or ~/.local/share
    SQUARE_API String app_data_dir();
    SQUARE_API String resource_dir();

    SQUARE_API bool is_directory(const String& directorypath);
    SQUARE_API bool is_file(const String& filepath);
    SQUARE_API bool is_readable(const String& filepath);
    SQUARE_API bool is_writable(const String& filepath);
    SQUARE_API bool exists(const String& filepath);
    //archives (.sqz, .zip): a directory for all the functions here ("pack.sqz/dir/file"), read only
    SQUARE_API bool is_archive(const String& filepath);
    SQUARE_API bool archive_write(const String& directorypath, const String& archivepath);
    //utilities files
    SQUARE_API String get_directory(const String& filepath);
    SQUARE_API String get_filename(const String& filepath);
    SQUARE_API String get_basename(const String& filepath);
    SQUARE_API String get_extension(const String& filepath, bool to_lower = true);
    SQUARE_API String join(const String& path1, const String& path2);
    //read
    SQUARE_API Vector<char> file_read_all(const String& filepath);    
    SQUARE_API String text_file_read_all(const String& filepath);
    SQUARE_API Vector<unsigned char> binary_file_read_all(const String& filepath);
    SQUARE_API Vector<unsigned char> binary_compress_file_read_all(const String& filepath);
    //write
    SQUARE_API bool file_write_all(const String& filepath, const Vector<char>& buffer);
    SQUARE_API bool text_file_write_all(const String& filepath, const String& buffer);
    SQUARE_API bool binary_file_write_all(const String& filepath, const Vector<unsigned char>& buffer);
    SQUARE_API bool binary_compress_file_write_all(const String& filepath, const Vector<unsigned char>& buffer);

    //shell helps
    SQUARE_API bool makedir(const String& path);
    SQUARE_API bool copyfile(const String& infilepath, const String& ofilepath);


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // template help
    template < class ...Args >
    static inline String join(const String& path1, const String& path2, Args... args) { return join(path1, join(path2, args...)); }
    /////////////////////////////////////////////////////////////////////////////////////////////////

    //utilities path
    struct PathOperation
    {
        bool m_success;
        String m_path;
    };
    SQUARE_API PathOperation get_fullpath(const String& relative);
    SQUARE_API PathOperation get_canonical(const String& fullpath);
    SQUARE_API PathOperation get_relative_to(const String& base, const String& path);
    //utilities directories
    struct FolderList
    {
        bool m_success;
        Vector < String > m_fields;
        //iterator
        Vector < String >::iterator begin(){ return m_fields.begin(); }
        Vector < String >::iterator end()  { return m_fields.end(); }
        Vector < String >::const_iterator begin() const { return m_fields.begin(); }
        Vector < String >::const_iterator end()   const { return m_fields.end(); }
    };
    using FilesList       = FolderList;
    using DirectoriesList = FolderList;

    SQUARE_API FilesList       get_files(const String& directorypath);
    SQUARE_API DirectoriesList get_sub_directories(const String& directorypath);
    
    namespace Stream
    {
        class SQUARE_API GZStreambuf : public std::streambuf
        {
            using gzFile = void*;
            static const size_t buffer_size = 8192;
        public:
            GZStreambuf();
            ~GZStreambuf() override;

            bool open(const String& filename, const char* mode);
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
            explicit GZOStream(const String& filename);
            void open(const String& filename);
            void close();

        private:
            GZStreambuf buf_;
        };

        class SQUARE_API GZIStream : public std::istream
        {
        public:
            GZIStream();
            explicit GZIStream(const String& filename);
            void open(const String& filename);
            void close();

        private:
            GZStreambuf buf_;
        };
    }
}
}
