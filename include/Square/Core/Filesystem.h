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
    SQUARE_API std::string working_dir();
    SQUARE_API std::string home_dir();

    SQUARE_API bool is_directory(const std::string& directorypath);
    SQUARE_API bool is_file(const std::string& filepath);
    SQUARE_API bool is_readable(const std::string& filepath);
    SQUARE_API bool is_writable(const std::string& filepath);
    SQUARE_API bool exists(const std::string& filepath);
    //utilities files
    SQUARE_API std::string get_directory(const std::string& filepath);
    SQUARE_API std::string get_filename(const std::string& filepath);
    SQUARE_API std::string get_basename(const std::string& filepath);
    SQUARE_API std::string get_extension(const std::string& filepath);
    SQUARE_API std::vector<char> file_read_all(const std::string& filepath);
    SQUARE_API std::string text_file_read_all(const std::string& filepath);

    //utilities path
    struct PathOperation
    {
        bool m_success;
        std::string m_path;
    };
    SQUARE_API PathOperation get_fullpath(const std::string& relative);
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
}
}
