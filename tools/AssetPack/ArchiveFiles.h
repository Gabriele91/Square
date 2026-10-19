//
//  ArchiveFiles.h
//  AssetPack
//
//  The files of an archive of the resources (.sqz): read all of them, changed, written again as
//  a new archive (its folder made, packed, removed).
//
#pragma once
#include <Square/Square.h>

class ArchiveFiles
{
public:
    //the files of an archive (its root: the archives of the games have no folders); false: none
    bool read(const Square::String& archive);

    //the archive of the files (a folder next to it, packed, removed)
    bool write(const Square::String& archive) const;

    //a file: its bytes (null: none)
    const Square::Vector<unsigned char>* find(const Square::String& name) const;
    bool contains(const Square::String& name) const;
    void set(const Square::String& name, const Square::Vector<unsigned char>& bytes);
    void remove(const Square::String& name);

    //the names of the files, of an extension (".sqtex"; empty: all)
    Square::Vector<Square::String> names(const Square::String& extension = "") const;

    //the bytes of all the files
    size_t size() const;

    //the same bytes: the same hash (FNV-1a, 64 bit)
    static Square::uint64 hash_of(const Square::Vector<unsigned char>& bytes);

private:
    Square::Map<Square::String, Square::Vector<unsigned char>> m_files;
};
