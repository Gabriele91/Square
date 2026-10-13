//
//  ShaderCache.cpp
//  Square
//
//  See ShaderCache.h for the high level description.
//
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include "Square/Config.h"
#include "Square/Core/Filesystem.h"
#include "Square/Resource/ShaderCache.h"

namespace Square
{
namespace Resource
{
	namespace AuxShaderCache
	{
		//the magic of a file
		static const char s_magic[4]{ 'S', 'Q', 'S', 'H' };

		//FNV-1a (the same on every run, every compiler), from a basis
		static uint64 fnv1a(const std::string& text, uint64 value)
		{
			for (const char c : text)
			{
				value ^= uint64(static_cast<unsigned char>(c));
				value *= 1099511628211ull;
			}
			return value;
		}

		static void put32(std::vector<unsigned char>& bytes, uint32 value)
		{
			for (int i = 0; i != 4; ++i)
			{
				bytes.push_back(static_cast<unsigned char>((value >> (8 * i)) & 0xFF));
			}
		}

		static void put64(std::vector<unsigned char>& bytes, uint64 value)
		{
			put32(bytes, uint32(value & 0xFFFFFFFFull));
			put32(bytes, uint32(value >> 32));
		}

		static void put_text(std::vector<unsigned char>& bytes, const std::string& text)
		{
			put32(bytes, uint32(text.size()));
			bytes.insert(bytes.end(), text.begin(), text.end());
		}

		//a reader of a file: false at the end (nothing more read)
		struct Reader
		{
			const std::vector<unsigned char>& m_bytes;
			size_t m_at{ 0 };
			bool   m_good{ true };

			uint32 get32()
			{
				uint32 value = 0;
				m_good = m_good && m_at + 4 <= m_bytes.size();
				for (int i = 0; i != 4 && m_good; ++i)
				{
					value |= uint32(m_bytes[m_at++]) << (8 * i);
				}
				return value;
			}

			uint64 get64()
			{
				const uint64 low = get32();
				const uint64 high = get32();
				return low | (high << 32);
			}

			std::string get_text()
			{
				const uint32 size = get32();
				std::string text;
				m_good = m_good && m_at + size <= m_bytes.size();
				if (m_good)
				{
					text.assign(reinterpret_cast<const char*>(m_bytes.data() + m_at), size);
					m_at += size;
				}
				return text;
			}
		};
	}

	ShaderCache::Key ShaderCache::key(const std::string& what)
	{
		Key out;
		out.m_hash = AuxShaderCache::fnv1a(what, 14695981039346656037ull);
		//(another basis: another hash of the same)
		out.m_check = AuxShaderCache::fnv1a(what, 0x9E3779B97F4A7C15ull);
		return out;
	}

	std::string ShaderCache::file_of(const std::string& root, const std::string& language, const Key& key)
	{
		char name[32];
		std::snprintf(name, sizeof(name), "%016llx.sqsh", (unsigned long long)key.m_hash);
		return Filesystem::join(Filesystem::join(root, language), name);
	}

	std::vector<unsigned char> ShaderCache::encode(const Key& key, const ShaderStages& stages)
	{
		using namespace AuxShaderCache;
		std::vector<unsigned char> bytes(s_magic, s_magic + 4);
		put32(bytes, version);
		put64(bytes, key.m_check);
		put32(bytes, uint32(stages.size()));
		for (const ShaderStage& stage : stages)
		{
			put32(bytes, uint32(stage.m_type));
			put_text(bytes, stage.m_entry);
			put_text(bytes, stage.m_header);
			put_text(bytes, stage.m_source);
		}
		return bytes;
	}

	bool ShaderCache::decode(const std::vector<unsigned char>& bytes, const Key& key, ShaderStages& stages)
	{
		using namespace AuxShaderCache;
		bool good = bytes.size() >= 4 && std::memcmp(bytes.data(), s_magic, 4) == 0;
		Reader reader{ bytes, 4 };
		good = good && reader.get32() == version;
		good = good && reader.get64() == key.m_check;
		const uint32 count = good ? reader.get32() : 0;
		ShaderStages out;
		for (uint32 i = 0; i != count && good; ++i)
		{
			ShaderStage stage;
			const uint32 type = reader.get32();
			stage.m_entry = reader.get_text();
			stage.m_header = reader.get_text();
			stage.m_source = reader.get_text();
			good = reader.m_good && type < uint32(Render::ST_N_SHADER);
			stage.m_type = Render::ShaderType(type);
			out.push_back(std::move(stage));
		}
		good = good && reader.m_good && !out.empty();
		if (good)
		{
			stages = std::move(out);
		}
		return good;
	}

	bool ShaderCache::find(const std::string& language, const Key& key, ShaderStages& stages) const
	{
		bool found = false;
		//the pack (an archive or a folder: read only)
		if (!m_pack.empty())
		{
			const std::string file = file_of(m_pack, language, key);
			found = Filesystem::exists(file) && decode(Filesystem::binary_file_read_all(file), key, stages);
		}
		//the folder of the runtime (the file touched: used)
		if (!found && !m_folder.empty())
		{
			const std::string file = file_of(m_folder, language, key);
			found = Filesystem::exists(file) && decode(Filesystem::binary_file_read_all(file), key, stages);
			if (found)
			{
				std::error_code error;
				std::filesystem::last_write_time(std::filesystem::u8path(file), std::filesystem::file_time_type::clock::now(), error);
			}
		}
		return found;
	}

	bool ShaderCache::store(const std::string& language, const Key& key, const ShaderStages& stages) const
	{
		bool stored = false;
		if (!m_folder.empty())
		{
			const std::filesystem::path file = std::filesystem::u8path(file_of(m_folder, language, key));
			std::error_code error;
			std::filesystem::create_directories(file.parent_path(), error);
			//a file aside, then renamed: never read half written
			const std::filesystem::path temporary = file.string() + ".tmp";
			const std::vector<unsigned char> bytes = encode(key, stages);
			{
				std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
				out.write(reinterpret_cast<const char*>(bytes.data()), std::streamsize(bytes.size()));
				stored = bool(out);
			}
			std::filesystem::rename(temporary, file, error);
			stored = stored && !error;
			if (error)
			{
				std::filesystem::remove(temporary, error);
			}
		}
		return stored;
	}

	size_t ShaderCache::prune(const std::string& folder, int days)
	{
		namespace fs = std::filesystem;
		size_t removed = 0;
		std::error_code error;
		const fs::path root = fs::u8path(folder);
		if (fs::is_directory(root, error))
		{
			const auto now = fs::file_time_type::clock::now();
			const auto old = std::chrono::hours(24 * days);
			const auto leftover = std::chrono::hours(1);
			std::vector<fs::path> gone;
			for (fs::recursive_directory_iterator it(root, error), end; it != end && !error; it.increment(error))
			{
				if (!it->is_regular_file(error)) continue;
				const auto age = now - it->last_write_time(error);
				const bool temporary = it->path().extension() == ".tmp";
				if (age > old || (temporary && age > leftover))
				{
					gone.push_back(it->path());
				}
			}
			for (const fs::path& file : gone)
			{
				removed += fs::remove(file, error) ? 1 : 0;
			}
		}
		return removed;
	}
}
}
