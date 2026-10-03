//
//  FilesystemArchive.h
//  Square
//
//  The archives (.sqz, .zip) of Filesystem: an archive on the disk is a directory, its entries
//  are its files and sub directories ("models/hovercraft.sqz/scene.acgz"). Opened at the first
//  path inside it (its index read once), read by minizip (zlib/contrib/minizip), open until
//  the end of the application. Only for Filesystem.cpp.
//
#pragma once
#include <string>
#include <vector>

namespace Square
{
namespace Filesystem
{
namespace Archive
{
	//a path inside an archive (or the archive itself): its directory or one of its entries
	bool is_directory(const std::string& path);
	bool is_file(const std::string& path);
	//the bytes of a file of an archive (false: not in an archive)
	bool read(const std::string& path, std::vector<unsigned char>& output);
	//the files and the sub directories of a directory of an archive (false: not in an archive)
	bool list(const std::string& path, std::vector<std::string>& files, std::vector<std::string>& directories);
	//the files of a directory, its sub directories too, in an archive (deflate level 6; images
	//and gzip files stored as they are)
	bool write(const std::string& directory, const std::string& archive_path);
}
}
}
