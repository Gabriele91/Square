//
//  AssetReport.h
//  AssetPack
//
//  The report of a release of the archives of a platform: each archive (its size before and after,
//  its textures: kept, compressed, shared, removed; what was done to each one), then the files the
//  archives have the same (the same bytes in more archives: a map and another one), the bytes they
//  repeat.
//
#pragma once
#include <Square/Square.h>
#include "ArchiveFiles.h"
#include "TexturePacker.h"

class AssetReport
{
public:
    //an archive done: its sizes, its textures, its files (the same ones of the other archives)
    void add(const Square::String& archive,
             size_t size_before,
             const ArchiveFiles& files,
             const TexturePacker::Stats& stats,
             const Square::Vector<Square::String>& log);

    //the report, its text
    Square::String text(const Square::String& platform) const;

    //the bytes before and after (all the archives)
    size_t size_before() const;
    size_t size_after() const;

private:
    //the files of the same bytes in more archives: from this size (small ones: not worth it)
    static constexpr size_t s_shared_size = 16 * 1024;

    struct Archive
    {
        Square::String               m_name;
        size_t                       m_before{ 0 };
        size_t                       m_after{ 0 };
        TexturePacker::Stats         m_stats;
        Square::Vector<Square::String> m_log;
    };
    //a file of an archive (its name, its size)
    struct File
    {
        Square::String m_archive;
        Square::String m_name;
        size_t         m_size{ 0 };
    };

    //the lines of an archive
    static Square::String archive_text(const Archive& archive);
    //the lines of the files more archives have
    Square::String shared_text() const;

    Square::Vector<Archive>                         m_archives;
    Square::Map<Square::uint64, Square::Vector<File>> m_files;
};
