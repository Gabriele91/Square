//
//  ArchiveFiles.cpp
//  AssetPack
//
//  The files of an archive of the resources (.sqz): read all of them, changed, written again as
//  a new archive (its folder made, packed, removed).
//
#include "ArchiveFiles.h"
#include <filesystem>

bool ArchiveFiles::read(const Square::String& archive)
{
    using namespace Square;
    m_files.clear();
    const Filesystem::FilesList files = Filesystem::get_files(archive);
    if (!files.m_success)
    {
        return false;
    }
    for (const String& name : files.m_fields)
    {
        m_files[name] = Filesystem::binary_file_read_all(Filesystem::join(archive, name));
    }
    return true;
}

bool ArchiveFiles::write(const Square::String& archive) const
{
    using namespace Square;
    //the files in a folder next to the archive, then packed
    const String folder = archive + ".files";
    std::filesystem::remove_all(folder.c_str());
    std::filesystem::create_directories(folder.c_str());
    for (const auto& file : m_files)
    {
        if (!Filesystem::binary_file_write_all(Filesystem::join(folder, file.first), file.second))
        {
            return false;
        }
    }
    std::filesystem::remove(archive.c_str());
    const bool written = Filesystem::archive_write(folder, archive);
    std::filesystem::remove_all(folder.c_str());
    return written;
}

const Square::Vector<unsigned char>* ArchiveFiles::find(const Square::String& name) const
{
    auto file = m_files.find(name);
    if (file == m_files.end())
    {
        return nullptr;
    }
    return &file->second;
}

bool ArchiveFiles::contains(const Square::String& name) const
{
    return m_files.find(name) != m_files.end();
}

void ArchiveFiles::set(const Square::String& name, const Square::Vector<unsigned char>& bytes)
{
    m_files[name] = bytes;
}

void ArchiveFiles::remove(const Square::String& name)
{
    m_files.erase(name);
}

Square::Vector<Square::String> ArchiveFiles::names(const Square::String& extension) const
{
    Square::Vector<Square::String> out;
    for (const auto& file : m_files)
    {
        if (extension.empty() || Square::Filesystem::get_extension(file.first) == extension)
        {
            out.push_back(file.first);
        }
    }
    return out;
}

Square::uint64 ArchiveFiles::hash_of(const Square::Vector<unsigned char>& bytes)
{
    Square::uint64 hash = 14695981039346656037ULL;
    for (unsigned char byte : bytes)
    {
        hash ^= byte;
        hash *= 1099511628211ULL;
    }
    return hash;
}

size_t ArchiveFiles::size() const
{
    size_t bytes = 0;
    for (const auto& file : m_files)
    {
        bytes += file.second.size();
    }
    return bytes;
}
