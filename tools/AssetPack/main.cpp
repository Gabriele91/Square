//
//  AssetPack
//  The archives of the resources of a game (.sqz) for the release of a platform: each texture in
//  the compression of its GPUs (desktop: BC in DDS; mobile: ASTC in KTX) from the best image it
//  has (one already in that compression, else its image not compressed, else the other
//  compression), the same images in one file, the images no texture uses removed; a report of what
//  was done (and of the files more archives have the same):
//
//      AssetPack -t android -i assets -o release/android/assets
//      AssetPack -t windows -i assets/arena.sqz -o release/windows/assets -f
//
//  The images of the archives of the models are best made by Model with all of them (--images
//  all: PNG, DDS and KTX): each release keeps the ones of its platform, nothing compressed again.
//
#define SQUARE_MAIN
#include <Square/Square.h>
#include <filesystem>
#include "ArchiveFiles.h"
#include "AssetReport.h"
#include "TexturePacker.h"

static Square::Shell::ParserCommands s_ShellCommands
{
      Square::Shell::Command{ "platform", "t", "the platform of the release [windows, linux, macos, android, ios; desktop, mobile]", Square::Shell::ValueType::value_string, true,  Square::Shell::Value_t() }
    , Square::Shell::Command{ "input",    "i", "archives (.sqz) or folders of archives, by ','"                                   , Square::Shell::ValueType::value_string, true,  Square::Shell::Value_t() }
    , Square::Shell::Command{ "output",   "o", "the folder of the archives of the release (not the one of the input)"             , Square::Shell::ValueType::value_string, true,  Square::Shell::Value_t() }
    , Square::Shell::Command{ "report",   "r", "the report (default: <output>/assetpack_<platform>.txt)"                          , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "fallback", "f", "keep the images not compressed as the fallbacks of the textures"                  , Square::Shell::ValueType::value_none,   false, Square::Shell::Value_t(false) }
    , Square::Shell::Command{ "cache",    "c", "the folder of the images compressed (the next releases do not compress them again)", Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "help",     "h", "show help"                                                                        , Square::Shell::ValueType::value_none,   false, Square::Shell::Value_t(false) }
};

namespace AuxAssetPack
{
    //a platform and the compression of its GPUs
    struct PlatformCompression
    {
        const char*                 m_platform;
        ImageConverter::Compression m_compression;
    };
    static const PlatformCompression s_platforms[] =
    {
        { "windows", ImageConverter::Compression::BC   },
        { "linux",   ImageConverter::Compression::BC   },
        { "macos",   ImageConverter::Compression::BC   },
        { "desktop", ImageConverter::Compression::BC   },
        { "android", ImageConverter::Compression::ASTC },
        { "ios",     ImageConverter::Compression::ASTC },
        { "mobile",  ImageConverter::Compression::ASTC },
    };

    //the compression of a platform (NONE: not one)
    static ImageConverter::Compression compression_of(const Square::String& platform)
    {
        for (const PlatformCompression& entry : s_platforms)
        {
            if (Square::case_insensitive_equal(platform, entry.m_platform))
            {
                return entry.m_compression;
            }
        }
        return ImageConverter::Compression::NONE;
    }

    //the archives of a path: itself (an archive), else the archives of the folder
    static void archives_of_path(const Square::String& path, Square::Vector<Square::String>& out)
    {
        if (Square::Filesystem::get_extension(path) == ".sqz")
        {
            out.push_back(path);
            return;
        }
        for (const auto& entry : std::filesystem::directory_iterator(path.c_str()))
        {
            const Square::String file = entry.path().generic_string().c_str();
            if (entry.is_regular_file() && Square::Filesystem::get_extension(file) == ".sqz")
            {
                out.push_back(file);
            }
        }
    }

    //the archives of the input (paths by ',')
    static Square::Vector<Square::String> archives_of(const Square::String& input)
    {
        Square::Vector<Square::String> out;
        Square::StringStream paths(input);
        for (Square::String path; std::getline(paths, path, ',');)
        {
            if (!path.empty())
            {
                archives_of_path(path, out);
            }
        }
        return out;
    }

    //an archive for the platform: read, its textures, written in the output
    static bool pack_archive(Square::Context& context,
                             const Square::String& archive,
                             const Square::String& output,
                             TexturePacker& packer,
                             AssetReport& report)
    {
        ArchiveFiles files;
        if (!files.read(archive))
        {
            context.logger()->warning("AssetPack: unable to read " + archive);
            return false;
        }
        const size_t size_before = files.size();
        Square::Vector<Square::String> log;
        const TexturePacker::Stats stats = packer.pack(files, log);
        const Square::String name = Square::Filesystem::get_filename(archive);
        if (!files.write(Square::Filesystem::join(output, name)))
        {
            context.logger()->warning("AssetPack: unable to write " + name);
            return false;
        }
        report.add(name, size_before, files, stats, log);
        context.logger()->info("AssetPack: " + name + ", " + Square::to_string(stats.m_textures) + " textures");
        return true;
    }
}

square_main(s_ShellCommands)(Square::Application& app, Square::Shell::ParserValue& args, Square::Shell::Error& errors)
{
    using namespace Square;
    if (args.find("help") != args.end() && std::get<bool>(args["help"]))
    {
        app.context()->logger()->info(Shell::filename(args) + ":\n");
        app.context()->logger()->info(Shell::help(s_ShellCommands));
        return 0;
    }
    if (errors.type != Shell::ErrorType::none)
    {
        app.context()->logger()->error("Error to parse input [" + Square::to_string(errors.id_argument) + "]: " + errors.what);
        return -1;
    }
    const String platform = std::get<String>(args.at("platform"));
    const String input = std::get<String>(args.at("input"));
    const String output = std::get<String>(args.at("output"));
    const ImageConverter::Compression compression = AuxAssetPack::compression_of(platform);
    if (compression == ImageConverter::Compression::NONE)
    {
        app.context()->logger()->error("AssetPack: unknown platform " + platform);
        return -1;
    }
    std::filesystem::create_directories(output.c_str());
    const String cache = std::get<String>(args.at("cache"));
    if (!cache.empty())
    {
        std::filesystem::create_directories(cache.c_str());
    }
    //the archives, then the report
    TexturePacker packer(compression, std::get<bool>(args.at("fallback")), cache);
    AssetReport report;
    bool done = true;
    for (const String& archive : AuxAssetPack::archives_of(input))
    {
        done = AuxAssetPack::pack_archive(*app.context(), archive, output, packer, report) && done;
    }
    String report_file = std::get<String>(args.at("report"));
    if (report_file.empty())
    {
        report_file = Filesystem::join(output, "assetpack_" + platform + ".txt");
    }
    Filesystem::text_file_write_all(report_file, report.text(platform));
    app.context()->logger()->info("AssetPack: " + report_file);
    if (!done)
    {
        return 1;
    }
    return 0;
}
