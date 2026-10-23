//
//  Settings.cpp
//  Builder
//
//  See Settings.h.
//
#include "Settings.h"
#include <filesystem>

namespace Builder
{
    namespace AuxSettings
    {
        //a field of the file: a text, a flag (its name, the field of the settings)
        struct Text
        {
            const char*                    m_name;
            Square::String Settings::*     m_field;
        };
        struct Flag
        {
            const char*      m_name;
            bool Settings::* m_field;
        };

        static const Text s_texts[] =
        {
            { "cmake",       &Settings::m_cmake },
            { "ninja",       &Settings::m_ninja },
            { "vcvars",      &Settings::m_vcvars },
            { "clang_cl",    &Settings::m_clang_cl },
            { "clang",       &Settings::m_clang },
            { "gcc",         &Settings::m_gcc },
            { "android_sdk", &Settings::m_android_sdk },
            { "java_home",   &Settings::m_java_home },
            { "game",        &Settings::m_game },
            { "target",      &Settings::m_target },
            { "engine",      &Settings::m_engine },
            { "build_root",  &Settings::m_build_root },
            { "platform",    &Settings::m_platform },
            { "config",      &Settings::m_config },
            { "compiler",    &Settings::m_compiler },
        };

        static const Flag s_flags[] =
        {
            { "assets", &Settings::m_assets },
            { "deploy", &Settings::m_deploy },
            { "run",    &Settings::m_run },
        };
    }

    Square::String Settings::path()
    {
        using namespace Square::Filesystem;
        return join(join(app_data_dir(), "square"), "builder.json");
    }

    bool Settings::load()
    {
        using namespace AuxSettings;
        const Square::String file = path();
        if (!Square::Filesystem::exists(file))
        {
            return false;
        }
        Square::Data::Json json;
        if (!json.parser(Square::Filesystem::text_file_read_all(file)) || !json.document().is_object())
        {
            return false;
        }
        const Square::Data::JsonValue& root = json.document();
        for (const Text& text : s_texts)
        {
            if (root.contains(text.m_name) && root[text.m_name].is_string())
            {
                this->*text.m_field = root[text.m_name].string();
            }
        }
        for (const Flag& flag : s_flags)
        {
            if (root.contains(flag.m_name) && root[flag.m_name].is_boolean())
            {
                this->*flag.m_field = root[flag.m_name].boolean();
            }
        }
        return true;
    }

    bool Settings::save() const
    {
        using namespace AuxSettings;
        Square::Data::JsonObject root;
        for (const Text& text : s_texts)
        {
            root[text.m_name] = Square::Data::JsonValue(this->*text.m_field);
        }
        for (const Flag& flag : s_flags)
        {
            root[flag.m_name] = Square::Data::JsonValue(this->*flag.m_field);
        }
        Square::OStringStream out;
        out << Square::Data::JsonValue(root);
        const Square::String file = path();
        std::error_code error;
        std::filesystem::create_directories(std::filesystem::u8path(Square::Filesystem::get_directory(file)), error);
        return Square::Filesystem::text_file_write_all(file, out.str());
    }
}
