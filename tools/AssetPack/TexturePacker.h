//
//  TexturePacker.h
//  AssetPack
//
//  The textures of an archive for a platform: each one in the compression of its GPUs (desktop:
//  BC in DDS; mobile: ASTC in KTX), from the best image it has: one already in that compression
//  (kept as it is), else its image not compressed (PNG, TGA, BMP, JPEG: a file or the bytes in its
//  .sqtex), else the one of the other compression (decoded, compressed again). The same image
//  (the same bytes) is compressed once: its textures share one file. The images no texture uses
//  any more are removed. The images of an archive are compressed at once (one a thread of the CPU),
//  kept in a cache (a folder: the same image of another release, not compressed again).
//
#pragma once
#include <Square/Square.h>
#include "ArchiveFiles.h"
#include "ImageConverter.h"
#include "TextureDescriptor.h"

class TexturePacker
{
public:
    //what an archive became
    struct Stats
    {
        size_t m_textures{ 0 };     //the .sqtex
        size_t m_kept{ 0 };         //already in the compression of the platform
        size_t m_from_raw{ 0 };     //compressed from their image not compressed
        size_t m_from_other{ 0 };   //compressed again from the other compression
        size_t m_uncompressed{ 0 }; //left not compressed (a size BC has not, an image not decoded)
        size_t m_shared{ 0 };       //the same image of another texture: one file
        size_t m_removed{ 0 };      //images no texture uses any more
    };

    //compression: BC (desktop) or ASTC (mobile); keep_fallback: the images not compressed kept as
    //the fallbacks of the textures (else removed: smaller); cache: the folder of the images
    //compressed by the releases before (empty: none)
    TexturePacker(ImageConverter::Compression compression, bool keep_fallback, const Square::String& cache);

    //the textures of an archive (its files changed), what was done to each one in the log
    Stats pack(ArchiveFiles& files, Square::Vector<Square::String>& log);

private:
    //an image of a texture: its compression (NONE: not compressed) and its file
    struct Source
    {
        ImageConverter::Compression    m_compression{ ImageConverter::Compression::NONE };
        Square::String                 m_name;          //its file (empty: in the .sqtex)
        const Square::Vector<unsigned char>* m_bytes{ nullptr };
    };

    //a texture of the archive: its .sqtex, its images, the best one, the key of its image
    struct Plan
    {
        Square::String         m_file;
        Square::String         m_name;
        TextureDescriptor      m_texture;
        Square::Vector<Source> m_sources;
        Source                 m_source;
        bool                   m_normal_map{ false };
        Square::String         m_key;
    };

    //an image compressed (empty: it cannot be; its description: why, or how)
    struct Converted
    {
        Square::Vector<unsigned char> m_bytes;
        Square::String                m_description;
    };

    //the textures of an archive: their images, the best one of each
    Square::Vector<Plan> plan(const ArchiveFiles& files, Stats& stats, Square::Vector<Square::String>& log) const;
    //the images to compress (one a key), compressed at once: the threads of the CPU, one image each
    void convert_all(const Square::Vector<Plan>& plans);
    //the file of an image in the cache (its key, the compression; empty: no cache)
    Square::String cache_file(const Square::String& key) const;
    //an image from the cache (false: not there)
    bool from_cache(const Square::String& key, Converted& converted) const;
    //an image in the cache
    void to_cache(const Square::String& key, const Converted& converted) const;

    //the textures the materials use as their normal maps (normal_map texture("name"))
    static Square::Set<Square::String> normal_maps(const ArchiveFiles& files);
    //the compression of an image (NONE: not compressed)
    static ImageConverter::Compression compression_of(const Square::Vector<unsigned char>& bytes);
    //the images of a texture: its url, its fallback, the ones of its url with the other extensions
    Square::Vector<Source> sources_of(const ArchiveFiles& files, const TextureDescriptor& texture) const;
    //the best one: of the compression of the platform, else not compressed, else the other one
    Source best_source(const Square::Vector<Source>& sources) const;
    //the pixels (RGBA) of the first level of a compressed image (empty: not decoded)
    static Square::Vector<unsigned char> decode(const Square::Vector<unsigned char>& bytes, bool normal_map, unsigned long& width, unsigned long& height);
    //an image in the compression of the platform (empty: it cannot be)
    Square::Vector<unsigned char> convert(const Source& source, bool normal_map, Square::String& description, unsigned int threads) const;
    //the name of the file of a texture in the compression of the platform
    Square::String output_name(const ArchiveFiles& files, const Square::String& texture_name, const TextureDescriptor& texture) const;
    //the file of an image as it is (the bytes in the .sqtex: a file of its own)
    Square::String stored(ArchiveFiles& files, const Square::String& texture_name, const Source& source);
    //the key of an image: its bytes, a normal map or not (the same key: one file)
    static Square::String key_of(const Source& source, bool normal_map);
    //a texture in the compression of the platform: its file
    Square::String pack_texture
    (
        ArchiveFiles& files,
        const Square::String& texture_name,
        const TextureDescriptor& texture,
        const Source& source,
        bool normal_map,
        Square::Map<Square::String, Square::String>& compressed,
        Stats& stats,
        Square::Vector<Square::String>& log
    );
    //the fallback of a texture (its image not compressed in a file; empty: none or not kept)
    Square::String fallback_of(const Square::Vector<Source>& sources, const Square::String& file) const;
    //the images no texture uses (the ones the textures had), not named by a material
    size_t remove_unused(ArchiveFiles& files, const Square::Set<Square::String>& had, const Square::Set<Square::String>& used) const;

    ImageConverter::Compression m_compression;
    bool                        m_keep_fallback;
    Square::String              m_cache;
    //the images written by the pack of an archive (not the sources of its textures)
    Square::Set<Square::String> m_written;
    //the images compressed of an archive, by their key
    Square::Map<Square::String, Converted> m_converted;
};
