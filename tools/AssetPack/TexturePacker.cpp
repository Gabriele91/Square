//
//  TexturePacker.cpp
//  AssetPack
//
//  The textures of an archive for a platform: each one in the compression of its GPUs (desktop:
//  BC in DDS; mobile: ASTC in KTX), from the best image it has: one already in that compression
//  (kept as it is), else its image not compressed (PNG, TGA, BMP, JPEG: a file or the bytes in its
//  .sqtex), else the one of the other compression (decoded, compressed again). The same image
//  (the same bytes) is compressed once: its textures share one file. The images no texture uses
//  any more are removed.
//
#include "TexturePacker.h"
#include <atomic>
#include <cmath>
#include <thread>

namespace AuxTexturePacker
{
    //the extensions of the images of a texture next to its url ("x_img.dds": "x_img.ktx"...)
    static const char* s_image_extensions[] = { ".dds", ".ktx", ".png", ".tga", ".bmp", ".jpg" };

    //the channels of the pixels of a BC format (Data::Image::decompress_bc)
    struct BCChannels
    {
        Square::Render::TextureFormat m_format;
        unsigned int                  m_channels;
    };
    static const BCChannels s_bc_channels[] =
    {
        { Square::Render::TF_BC1, 4 },
        { Square::Render::TF_BC3, 4 },
        { Square::Render::TF_BC4, 1 },
        { Square::Render::TF_BC5, 2 },
    };

    //the magic numbers of the images not compressed, their extension (else ".tga": it has none)
    struct RawMagic
    {
        unsigned char m_first;
        unsigned char m_second;
        const char*   m_extension;
    };
    static const RawMagic s_raw_magics[] =
    {
        { 0x89, 'P',  ".png" },
        { 0xFF, 0xD8, ".jpg" },
        { 'B',  'M',  ".bmp" },
    };

    static unsigned int bc_channels(Square::Render::TextureFormat format)
    {
        for (const BCChannels& bc : s_bc_channels)
        {
            if (bc.m_format == format)
            {
                return bc.m_channels;
            }
        }
        return 4;
    }

    static Square::String raw_extension(const Square::Vector<unsigned char>& bytes)
    {
        if (bytes.size() < 2)
        {
            return ".tga";
        }
        for (const RawMagic& magic : s_raw_magics)
        {
            if (bytes[0] == magic.m_first && bytes[1] == magic.m_second)
            {
                return magic.m_extension;
            }
        }
        return ".tga";
    }

    //the format of a compressed image (TF_INVALID: not one)
    static Square::Render::TextureFormat format_of(const Square::Vector<unsigned char>& bytes)
    {
        Square::Vector<unsigned char> levels;
        unsigned long width = 0;
        unsigned long height = 0;
        unsigned int  count = 0;
        Square::Render::TextureFormat format = Square::Render::TF_INVALID;
        if (!Square::Data::Image::is_compressed(bytes))
        {
            return Square::Render::TF_INVALID;
        }
        if (!Square::Data::Image::load_compressed(bytes, levels, width, height, count, format))
        {
            return Square::Render::TF_INVALID;
        }
        return format;
    }

    //the z of a normal of its x, y (a unit vector, z towards the viewer: BC5 has x, y only)
    static unsigned char normal_z(unsigned char x, unsigned char y)
    {
        const float nx = float(x) / 255.0f * 2.0f - 1.0f;
        const float ny = float(y) / 255.0f * 2.0f - 1.0f;
        const float nz = std::sqrt(std::max(0.0f, 1.0f - nx * nx - ny * ny));
        return (unsigned char)std::lround((nz * 0.5f + 0.5f) * 255.0f);
    }

    //the pixels of a BC format as RGBA: R as grey, RG as a normal (its z), RGBA as they are
    static Square::Vector<unsigned char> to_rgba(const Square::Vector<unsigned char>& pixels, unsigned int channels, bool normal_map)
    {
        if (channels == 4)
        {
            return pixels;
        }
        const size_t count = pixels.size() / channels;
        Square::Vector<unsigned char> rgba(count * 4);
        for (size_t i = 0; i != count; ++i)
        {
            const unsigned char* in = &pixels[i * channels];
            unsigned char* out = &rgba[i * 4];
            out[0] = in[0];
            out[1] = in[0];
            out[2] = in[0];
            out[3] = 255;
            if (channels == 2)
            {
                out[1] = in[1];
                out[2] = 0;
                if (normal_map)
                {
                    out[2] = normal_z(in[0], in[1]);
                }
            }
        }
        return rgba;
    }

    //the name of a texture used by a material, in a line "normal_map texture("name")"
    static bool texture_of_line(const Square::String& line, const char* key, Square::String& name)
    {
        const size_t at = line.find(key);
        if (at == Square::String::npos)
        {
            return false;
        }
        const Square::String call = "texture(\"";
        const size_t first = line.find(call, at);
        if (first == Square::String::npos)
        {
            return false;
        }
        const size_t start = first + call.size();
        const size_t last = line.find('"', start);
        if (last == Square::String::npos)
        {
            return false;
        }
        name = line.substr(start, last - start);
        return true;
    }
}

TexturePacker::TexturePacker(ImageConverter::Compression compression, bool keep_fallback, const Square::String& cache)
: m_compression(compression)
, m_keep_fallback(keep_fallback)
, m_cache(cache)
{
}

Square::String TexturePacker::cache_file(const Square::String& key) const
{
    if (m_cache.empty())
    {
        return Square::String();
    }
    //(its key: the hash of its bytes and " normal"; a file name of it)
    Square::String name = key;
    for (char& letter : name)
    {
        if (letter == ' ')
        {
            letter = '_';
        }
    }
    return Square::Filesystem::join(m_cache, name + ImageConverter::extension(m_compression));
}

bool TexturePacker::from_cache(const Square::String& key, Converted& converted) const
{
    const Square::String file = cache_file(key);
    if (file.empty() || !Square::Filesystem::exists(file))
    {
        return false;
    }
    converted.m_bytes = Square::Filesystem::binary_file_read_all(file);
    converted.m_description = "from the cache";
    return !converted.m_bytes.empty();
}

void TexturePacker::to_cache(const Square::String& key, const Converted& converted) const
{
    const Square::String file = cache_file(key);
    if (file.empty() || converted.m_bytes.empty())
    {
        return;
    }
    Square::Filesystem::binary_file_write_all(file, converted.m_bytes);
}

Square::Set<Square::String> TexturePacker::normal_maps(const ArchiveFiles& files)
{
    Square::Set<Square::String> out;
    for (const Square::String& name : files.names(".mat"))
    {
        const Square::Vector<unsigned char>* file = files.find(name);
        Square::StringStream text(Square::String(file->begin(), file->end()));
        Square::String line;
        while (std::getline(text, line))
        {
            Square::String texture;
            if (AuxTexturePacker::texture_of_line(line, "normal_map", texture))
            {
                out.insert(texture);
            }
        }
    }
    return out;
}

ImageConverter::Compression TexturePacker::compression_of(const Square::Vector<unsigned char>& bytes)
{
    const Square::Render::TextureFormat format = AuxTexturePacker::format_of(bytes);
    if (format == Square::Render::TF_INVALID)
    {
        return ImageConverter::Compression::NONE;
    }
    if (format == Square::Render::TF_ASTC_4x4)
    {
        return ImageConverter::Compression::ASTC;
    }
    return ImageConverter::Compression::BC;
}

Square::Vector<TexturePacker::Source> TexturePacker::sources_of(const ArchiveFiles& files, const TextureDescriptor& texture) const
{
    Square::Vector<Source> out;
    Square::Set<Square::String> names;
    //the files: its url, its fallback, the images of its url with the other extensions
    if (!texture.m_url.empty())
    {
        names.insert(texture.m_url);
        const Square::String stem = Square::Filesystem::get_basename(texture.m_url);
        for (const char* extension : AuxTexturePacker::s_image_extensions)
        {
            names.insert(stem + extension);
        }
    }
    if (!texture.m_fallback.empty())
    {
        names.insert(texture.m_fallback);
    }
    for (const Square::String& name : names)
    {
        //(not an image this packer wrote for another texture: the source is the one it had)
        const Square::Vector<unsigned char>* bytes = files.find(name);
        if (bytes && !m_written.count(name))
        {
            out.push_back({ compression_of(*bytes), name, bytes });
        }
    }
    //its bytes in the .sqtex
    if (!texture.m_embedded.empty())
    {
        out.push_back({ compression_of(texture.m_embedded), "", &texture.m_embedded });
    }
    return out;
}

TexturePacker::Source TexturePacker::best_source(const Square::Vector<Source>& sources) const
{
    //the compression of the platform, else not compressed, else the other one
    const ImageConverter::Compression order[] =
    {
        m_compression,
        ImageConverter::Compression::NONE,
        ImageConverter::Compression::BC,
        ImageConverter::Compression::ASTC
    };
    for (ImageConverter::Compression compression : order)
    {
        for (const Source& source : sources)
        {
            if (source.m_compression == compression)
            {
                return source;
            }
        }
    }
    return Source();
}

Square::Vector<unsigned char> TexturePacker::decode(const Square::Vector<unsigned char>& bytes, bool normal_map, unsigned long& width, unsigned long& height)
{
    Square::Vector<unsigned char> levels;
    unsigned int count = 0;
    Square::Render::TextureFormat format = Square::Render::TF_INVALID;
    if (!Square::Data::Image::load_compressed(bytes, levels, width, height, count, format))
    {
        return Square::Vector<unsigned char>();
    }
    //the first level (the next ones made again from it)
    if (format == Square::Render::TF_ASTC_4x4)
    {
        return Square::Data::Image::decompress_astc(levels.data(), width, height);
    }
    const Square::Vector<unsigned char> pixels = Square::Data::Image::decompress_bc(levels.data(), width, height, format);
    if (pixels.empty())
    {
        return pixels;
    }
    return AuxTexturePacker::to_rgba(pixels, AuxTexturePacker::bc_channels(format), normal_map);
}

Square::Vector<unsigned char> TexturePacker::convert(const Source& source, bool normal_map, Square::String& description, unsigned int threads) const
{
    if (source.m_compression == ImageConverter::Compression::NONE)
    {
        ImageConverter::Result result = ImageConverter::compress(*source.m_bytes, normal_map, m_compression, threads);
        description = result.m_description;
        return result.m_data;
    }
    unsigned long width = 0;
    unsigned long height = 0;
    const Square::Vector<unsigned char> rgba = decode(*source.m_bytes, normal_map, width, height);
    if (rgba.empty())
    {
        description = "not decoded";
        return rgba;
    }
    ImageConverter::Result result = ImageConverter::compress_rgba(rgba, width, height, normal_map, m_compression, threads);
    description = result.m_description;
    return result.m_data;
}

Square::String TexturePacker::output_name(const ArchiveFiles& files, const Square::String& texture_name, const TextureDescriptor& texture) const
{
    Square::String stem = texture_name + "_img";
    if (!texture.m_url.empty())
    {
        stem = Square::Filesystem::get_basename(texture.m_url);
    }
    const Square::String extension = ImageConverter::extension(m_compression);
    Square::String name = stem + extension;
    for (size_t index = 2; files.contains(name); ++index)
    {
        name = stem + "_" + Square::to_string(index) + extension;
    }
    return name;
}

size_t TexturePacker::remove_unused(ArchiveFiles& files, const Square::Set<Square::String>& had, const Square::Set<Square::String>& used) const
{
    //the texture names the materials use (an image used as a texture by its name: kept)
    Square::Set<Square::String> named;
    for (const Square::String& material : files.names(".mat"))
    {
        const Square::Vector<unsigned char>* file = files.find(material);
        Square::StringStream text(Square::String(file->begin(), file->end()));
        Square::String line;
        while (std::getline(text, line))
        {
            Square::String texture;
            if (AuxTexturePacker::texture_of_line(line, "", texture))
            {
                named.insert(texture);
            }
        }
    }
    size_t removed = 0;
    for (const Square::String& name : had)
    {
        const bool in_use = used.count(name) || named.count(Square::Filesystem::get_basename(name));
        if (!in_use && files.contains(name))
        {
            files.remove(name);
            ++removed;
        }
    }
    return removed;
}

Square::String TexturePacker::stored(ArchiveFiles& files, const Square::String& texture_name, const Source& source)
{
    if (!source.m_name.empty())
    {
        return source.m_name;
    }
    Square::String extension = AuxTexturePacker::raw_extension(*source.m_bytes);
    if (source.m_compression != ImageConverter::Compression::NONE)
    {
        extension = ImageConverter::extension(source.m_compression);
    }
    const Square::String stem = texture_name + "_img";
    Square::String name = stem + extension;
    for (size_t index = 2; files.contains(name); ++index)
    {
        name = stem + "_" + Square::to_string(index) + extension;
    }
    files.set(name, *source.m_bytes);
    m_written.insert(name);
    return name;
}

Square::String TexturePacker::key_of(const Source& source, bool normal_map)
{
    Square::String key = Square::to_string(ArchiveFiles::hash_of(*source.m_bytes));
    if (normal_map)
    {
        key += " normal";
    }
    return key;
}

Square::String TexturePacker::pack_texture
(
    ArchiveFiles& files,
    const Square::String& texture_name,
    const TextureDescriptor& texture,
    const Source& source,
    bool normal_map,
    Square::Map<Square::String, Square::String>& compressed,
    Stats& stats,
    Square::Vector<Square::String>& log
)
{
    //the same image of another texture: its file
    const Square::String key = key_of(source, normal_map);
    auto done = compressed.find(key);
    if (done != compressed.end())
    {
        ++stats.m_shared;
        log.push_back(texture_name + ": the image of " + done->second);
        return done->second;
    }
    //already in the compression of the platform: as it is
    if (source.m_compression == m_compression)
    {
        ++stats.m_kept;
        const Square::String file = stored(files, texture_name, source);
        compressed[key] = file;
        log.push_back(texture_name + ": " + file + " kept");
        return file;
    }
    //compressed (from the image not compressed, else from the other compression: convert_all)
    const Converted& converted = m_converted[key];
    const Square::Vector<unsigned char>& bytes = converted.m_bytes;
    const Square::String& description = converted.m_description;
    if (bytes.empty())
    {
        ++stats.m_uncompressed;
        const Square::String file = stored(files, texture_name, source);
        compressed[key] = file;
        log.push_back(texture_name + ": " + file + " left as it is, " + description);
        return file;
    }
    const Square::String file = output_name(files, texture_name, texture);
    files.set(file, bytes);
    m_written.insert(file);
    compressed[key] = file;
    Square::String from = "from its image";
    if (source.m_compression == ImageConverter::Compression::NONE)
    {
        ++stats.m_from_raw;
    }
    else
    {
        ++stats.m_from_other;
        from = "from the other compression";
    }
    log.push_back(texture_name + ": " + file + " " + from + ", " + description);
    return file;
}

Square::String TexturePacker::fallback_of(const Square::Vector<Source>& sources, const Square::String& file) const
{
    if (!m_keep_fallback)
    {
        return Square::String();
    }
    for (const Source& source : sources)
    {
        const bool raw = source.m_compression == ImageConverter::Compression::NONE;
        if (raw && !source.m_name.empty() && source.m_name != file)
        {
            return source.m_name;
        }
    }
    return Square::String();
}

Square::Vector<TexturePacker::Plan> TexturePacker::plan(const ArchiveFiles& files, Stats& stats, Square::Vector<Square::String>& log) const
{
    //the .sqtex (all of them first: their sources point to their bytes)
    Square::Vector<Plan> plans;
    for (const Square::String& file : files.names(".sqtex"))
    {
        Plan plan;
        if (plan.m_texture.parse(*files.find(file)))
        {
            plan.m_file = file;
            plan.m_name = Square::Filesystem::get_basename(file);
            plans.push_back(plan);
        }
    }
    //their images, the best one, normal map or not
    const Square::Set<Square::String> normals = normal_maps(files);
    for (Plan& plan : plans)
    {
        ++stats.m_textures;
        plan.m_sources = sources_of(files, plan.m_texture);
        plan.m_source = best_source(plan.m_sources);
        if (!plan.m_source.m_bytes)
        {
            log.push_back(plan.m_name + ": no image");
            continue;
        }
        const bool bc5 = AuxTexturePacker::format_of(*plan.m_source.m_bytes) == Square::Render::TF_BC5;
        plan.m_normal_map = normals.count(plan.m_name) || bc5;
        plan.m_key = key_of(plan.m_source, plan.m_normal_map);
    }
    return plans;
}

void TexturePacker::convert_all(const Square::Vector<Plan>& plans)
{
    //the images to compress: one a key, not already in the compression of the platform
    Square::Vector<const Plan*> jobs;
    Square::Set<Square::String> keys;
    for (const Plan& plan : plans)
    {
        const bool compress = plan.m_source.m_bytes && plan.m_source.m_compression != m_compression;
        if (compress && keys.insert(plan.m_key).second)
        {
            jobs.push_back(&plan);
        }
    }
    //at once: the threads of the CPU, an image each (its compression on its thread only), the ones
    //of the cache read from it, the new ones written in it
    Square::Vector<Converted> results(jobs.size());
    std::atomic<size_t> next{ 0 };
    auto worker = [&]()
    {
        for (size_t job = next++; job < jobs.size(); job = next++)
        {
            const Plan& plan = *jobs[job];
            Converted& result = results[job];
            if (from_cache(plan.m_key, result))
            {
                continue;
            }
            result.m_bytes = convert(plan.m_source, plan.m_normal_map, result.m_description, 1);
            to_cache(plan.m_key, result);
        }
    };
    const size_t threads = std::min<size_t>(std::max(1u, std::thread::hardware_concurrency()), jobs.size());
    Square::Vector<std::thread> workers;
    for (size_t thread = 0; thread < threads; ++thread)
    {
        workers.emplace_back(worker);
    }
    for (std::thread& thread : workers)
    {
        thread.join();
    }
    for (size_t job = 0; job != jobs.size(); ++job)
    {
        m_converted[jobs[job]->m_key] = results[job];
    }
}

TexturePacker::Stats TexturePacker::pack(ArchiveFiles& files, Square::Vector<Square::String>& log)
{
    Stats stats;
    m_written.clear();
    m_converted.clear();
    Square::Vector<Plan> plans = plan(files, stats, log);
    convert_all(plans);
    //the textures in order (the same report, the same names of every run)
    Square::Set<Square::String> had;
    Square::Set<Square::String> used;
    Square::Map<Square::String, Square::String> compressed;
    for (Plan& plan : plans)
    {
        for (const Source& source : plan.m_sources)
        {
            if (!source.m_name.empty())
            {
                had.insert(source.m_name);
            }
        }
        if (!plan.m_source.m_bytes)
        {
            continue;
        }
        TextureDescriptor& texture = plan.m_texture;
        const Square::String file = pack_texture(files, plan.m_name, texture, plan.m_source, plan.m_normal_map, compressed, stats, log);
        texture.m_url = file;
        texture.m_embedded.clear();
        texture.m_fallback = fallback_of(plan.m_sources, file);
        used.insert(texture.m_url);
        if (!texture.m_fallback.empty())
        {
            used.insert(texture.m_fallback);
        }
        files.set(plan.m_file, texture.serialize());
    }
    stats.m_removed = remove_unused(files, had, used);
    return stats;
}
