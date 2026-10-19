//
//  AssetReport.cpp
//  AssetPack
//
//  The report of a release of the archives of a platform: each archive (its size before and after,
//  its textures: kept, compressed, shared, removed; what was done to each one), then the files the
//  archives have the same (the same bytes in more archives: a map and another one), the bytes they
//  repeat.
//
#include "AssetReport.h"

namespace AuxAssetReport
{
    static Square::String megabytes(size_t bytes)
    {
        const size_t tenths = (bytes * 10 + 512 * 1024) / (1024 * 1024);
        return Square::to_string(tenths / 10) + "." + Square::to_string(tenths % 10) + " MB";
    }

    //the archives a group of files is in (one each)
    template < class Files >
    static Square::Set<Square::String> archives_of(const Files& files)
    {
        Square::Set<Square::String> out;
        for (const auto& file : files)
        {
            out.insert(file.m_archive);
        }
        return out;
    }
}

void AssetReport::add(const Square::String& archive,
                      size_t size_before,
                      const ArchiveFiles& files,
                      const TexturePacker::Stats& stats,
                      const Square::Vector<Square::String>& log)
{
    Archive line;
    line.m_name = archive;
    line.m_before = size_before;
    line.m_after = files.size();
    line.m_stats = stats;
    line.m_log = log;
    m_archives.push_back(line);
    for (const Square::String& name : files.names())
    {
        const Square::Vector<unsigned char>* bytes = files.find(name);
        if (bytes->size() >= s_shared_size)
        {
            m_files[ArchiveFiles::hash_of(*bytes)].push_back({ archive, name, bytes->size() });
        }
    }
}

size_t AssetReport::size_before() const
{
    size_t bytes = 0;
    for (const Archive& archive : m_archives)
    {
        bytes += archive.m_before;
    }
    return bytes;
}

size_t AssetReport::size_after() const
{
    size_t bytes = 0;
    for (const Archive& archive : m_archives)
    {
        bytes += archive.m_after;
    }
    return bytes;
}

Square::String AssetReport::archive_text(const Archive& archive)
{
    using AuxAssetReport::megabytes;
    const TexturePacker::Stats& stats = archive.m_stats;
    Square::String text;
    text += "== " + archive.m_name + ": " + megabytes(archive.m_before) + " -> " + megabytes(archive.m_after) + "\n";
    text += "   textures " + Square::to_string(stats.m_textures)
          + ": kept " + Square::to_string(stats.m_kept)
          + ", from their image " + Square::to_string(stats.m_from_raw)
          + ", from the other compression " + Square::to_string(stats.m_from_other)
          + ", not compressed " + Square::to_string(stats.m_uncompressed)
          + ", the image of another one " + Square::to_string(stats.m_shared)
          + "; images removed " + Square::to_string(stats.m_removed) + "\n";
    for (const Square::String& line : archive.m_log)
    {
        text += "   " + line + "\n";
    }
    return text;
}

Square::String AssetReport::shared_text() const
{
    Square::String text = "== the same files in more archives (from " + Square::to_string(s_shared_size / 1024) + " KB)\n";
    size_t repeated = 0;
    for (const auto& group : m_files)
    {
        const Square::Vector<File>& files = group.second;
        if (AuxAssetReport::archives_of(files).size() < 2)
        {
            continue;
        }
        repeated += files[0].m_size * (files.size() - 1);
        text += "   " + AuxAssetReport::megabytes(files[0].m_size) + ":";
        for (const File& file : files)
        {
            text += " " + file.m_archive + "/" + file.m_name;
        }
        text += "\n";
    }
    text += "   repeated: " + AuxAssetReport::megabytes(repeated) + "\n";
    return text;
}

Square::String AssetReport::text(const Square::String& platform) const
{
    using AuxAssetReport::megabytes;
    Square::String text = "AssetPack, " + platform + ": " + megabytes(size_before()) + " -> " + megabytes(size_after()) + "\n\n";
    for (const Archive& archive : m_archives)
    {
        text += archive_text(archive) + "\n";
    }
    text += shared_text();
    return text;
}
