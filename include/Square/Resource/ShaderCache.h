//
//  ShaderCache.h
//  Square
//
//  The intermediate cache of the shaders: a shader translated (HLSL -> SPIR-V -> the language of
//  the backend: GLSL, GLSL ES, MSL, HLSL) is its stages, the input of the driver; they are kept by
//  the hash of what made them (the source preprocessed, its includes and defines in it, the
//  target, its options, the version of the converter). A shader changed: a new hash, translated
//  again. Two places, read in order:
//  - the pack (read only, made offline by tools/ShaderPack for the targets of a platform: the
//    shaders of a Retail build, none translated at runtime): <pack>/<language>/<hash>.sqsh, a
//    folder or an archive (shaders.sqz);
//  - the folder of the runtime (written when a shader is translated: the next run reads it), the
//    files read touched (kept by the pruning).
//  The final cache (the binaries of the driver from the stages) is the driver's
//  (Render::Context::shader_cache). prune: the files of the caches not used for some days gone.
//
#pragma once
#include <string>
#include <vector>
#include "Square/Config.h"
#include "Square/Driver/Render.h"

namespace Square
{
namespace Resource
{
	//a stage of a shader translated: its type, its entry point, its header, its source
	struct SQUARE_API ShaderStage
	{
		Render::ShaderType m_type{ Render::ST_VERTEX_SHADER };
		std::string        m_entry{ "main" };
		std::string        m_header;
		std::string        m_source;
	};
	using ShaderStages = std::vector< ShaderStage >;

	class SQUARE_API ShaderCache
	{
	public:

		//the version of the files (a new one: the old files never read again)
		//(3: the SPIR-V of Vulkan in its layout, HLSL2ALL m_vulkan_layout)
		//(4: the SPIR-V of Vulkan legalized by SPIRV-Tools)
		static constexpr uint32 version = 4;

		//the key of a shader: what made it (its target, its options, its source), its hashes
		struct Key
		{
			uint64 m_hash{ 0 };  //its name
			uint64 m_check{ 0 }; //another hash of the same, in the file (a collision: not taken)
		};
		static Key key(const std::string& what);

		//the folder of the runtime (written), the pack (read only; none: "")
		void folder(const std::string& folder) { m_folder = folder; }
		const std::string& folder() const { return m_folder; }
		void pack(const std::string& pack) { m_pack = pack; }
		const std::string& pack() const { return m_pack; }

		//the stages of a shader translated for a language (the pack, then the folder); false: not
		//there (or not whole)
		bool find(const std::string& language, const Key& key, ShaderStages& stages) const;
		//the stages of a shader translated: into the folder of the runtime
		bool store(const std::string& language, const Key& key, const ShaderStages& stages) const;

		//the file of a shader in a root (the pack, a folder)
		static std::string file_of(const std::string& root, const std::string& language, const Key& key);
		//the stages to bytes and back (the files)
		static std::vector<unsigned char> encode(const Key& key, const ShaderStages& stages);
		static bool decode(const std::vector<unsigned char>& bytes, const Key& key, ShaderStages& stages);

		//the files under a folder not written or read for days gone (the leftovers of a write too)
		static size_t prune(const std::string& folder, int days);

	private:

		std::string m_folder;
		std::string m_pack;
	};
}
}
