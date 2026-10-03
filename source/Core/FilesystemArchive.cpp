//
//  FilesystemArchive.cpp
//  Square
//
//  See FilesystemArchive.h.
//
#include <algorithm>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <memory>
#include <mutex>
#include <set>
#include <unordered_map>
#include <unzip.h>
#include <zip.h>
#include "Square/Core/Filesystem.h"
#include "FilesystemArchive.h"

#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/stat.h>
#endif

namespace Square
{
namespace Filesystem
{
namespace Archive
{
	namespace AuxArchive
	{
		//////////////////////////////////////////////////////////////////////////////////////
		//paths
		bool disk_is_file(const std::string& path)
		{
		#ifdef _WIN32
			const DWORD attributes = GetFileAttributesA(path.c_str());
			return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
		#else
			struct stat st;
			return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
		#endif
		}

		bool has_archive_extension(const std::string& name)
		{
			const std::string extension = get_extension(name);
			return extension == ".sqz" || extension == ".zip";
		}

		//compressed already (images, gzip): stored, deflate gains nothing
		bool is_compressed_file(const std::string& name)
		{
			static const std::set<std::string> extensions{ ".png", ".jpg", ".jpeg", ".gz", ".sm3dgz", ".acgz", ".acjgz" };
			return extensions.count(get_extension(name)) != 0;
		}

		std::string to_slash(std::string path)
		{
			std::replace(path.begin(), path.end(), '\\', '/');
			return path;
		}

		std::vector<std::string> split_parts(const std::string& path)
		{
			std::vector<std::string> parts;
			parts.reserve(std::count(path.begin(), path.end(), '/') + 1);
			size_t start = 0;
			for (size_t end = path.find('/'); end != std::string::npos; end = path.find('/', start))
			{
				parts.push_back(path.substr(start, end - start));
				start = end + 1;
			}
			parts.push_back(path.substr(start));
			return parts;
		}

		//an entry: without ".", ".." and empty parts (false: out of the archive)
		bool normalize_entry(const std::string& path, std::string& entry)
		{
			std::vector<std::string> parts;
			for (const auto& part : split_parts(path))
			{
				if (part.empty() || part == ".") continue;
				if (part != "..")         { parts.push_back(part); continue; }
				if (parts.empty())        return false;
				parts.pop_back();
			}
			entry.clear();
			for (const auto& part : parts)
			{
				if (!entry.empty()) entry += "/";
				entry += part;
			}
			return true;
		}

		std::string parent_of(const std::string& entry)
		{
			const size_t separator = entry.rfind('/');
			return separator == std::string::npos ? std::string() : entry.substr(0, separator);
		}

		std::string name_of(const std::string& entry)
		{
			const size_t separator = entry.rfind('/');
			return separator == std::string::npos ? entry : entry.substr(separator + 1);
		}

		//////////////////////////////////////////////////////////////////////////////////////
		//an open archive: its index (the files, their place, the directories)
		struct ArchiveData
		{
			struct Entry
			{
				unz64_file_pos m_position;
				ZPOS64_T       m_size;
			};
			unzFile                                m_file{ nullptr };
			std::unordered_map<std::string, Entry> m_files;
			std::set<std::string>                  m_directories{ "" }; //"" the root
			std::mutex                             m_mutex;             //one read at a time

			~ArchiveData()
			{
				if (m_file) unzClose(m_file);
			}

			bool open(const std::string& path)
			{
				m_file = unzOpen64(path.c_str());
				if (!m_file) return false;
				for (int status = unzGoToFirstFile(m_file); status == UNZ_OK; status = unzGoToNextFile(m_file))
				{
					add_current();
				}
				return true;
			}

			//the entry at the cursor of minizip
			void add_current()
			{
				unz_file_info64 info;
				char raw_name[1024];
				if (unzGetCurrentFileInfo64(m_file, &info, raw_name, sizeof(raw_name), nullptr, 0, nullptr, 0) != UNZ_OK) return;
				const std::string name = to_slash(raw_name);
				if (name.empty()) return;
				const bool is_directory = name.back() == '/';
				std::string entry;
				if (!normalize_entry(name, entry) || entry.empty()) return;
				//its directories
				for (std::string parent = is_directory ? entry : parent_of(entry); !parent.empty(); parent = parent_of(parent))
				{
					m_directories.insert(parent);
				}
				if (is_directory) return;
				Entry file;
				if (unzGetFilePos64(m_file, &file.m_position) != UNZ_OK) return;
				file.m_size = info.uncompressed_size;
				m_files[entry] = file;
			}

			bool read(const Entry& file, std::vector<unsigned char>& output)
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				unz64_file_pos position = file.m_position;
				if (unzGoToFilePos64(m_file, &position) != UNZ_OK) return false;
				if (unzOpenCurrentFile(m_file) != UNZ_OK) return false;
				output.resize(size_t(file.m_size));
				bool success = read_current(output);
				//the CRC is checked closing it
				success = unzCloseCurrentFile(m_file) == UNZ_OK && success;
				if (!success) output.clear();
				return success;
			}

			bool read_current(std::vector<unsigned char>& output)
			{
				for (size_t done = 0; done < output.size();)
				{
					const unsigned chunk = unsigned(std::min<size_t>(output.size() - done, 1u << 30));
					const int count = unzReadCurrentFile(m_file, output.data() + done, chunk);
					if (count <= 0) return false;
					done += size_t(count);
				}
				return true;
			}
		};

		//////////////////////////////////////////////////////////////////////////////////////
		//the archives open (null: not an archive), by their full path
		std::mutex                                                    s_mutex;
		std::unordered_map<std::string, std::unique_ptr<ArchiveData>> s_archives;

		std::string key_of(const std::string& archive_path)
		{
			std::string key = get_fullpath(archive_path).m_path;
			if (key.empty()) key = archive_path;
		#ifdef _WIN32
			key = to_slash(key);
			std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return char(std::tolower(c)); });
		#endif
			return key;
		}

		ArchiveData* open(const std::string& archive_path)
		{
			const std::string key = key_of(archive_path);
			std::lock_guard<std::mutex> lock(s_mutex);
			auto it = s_archives.find(key);
			if (it != s_archives.end()) return it->second.get();
			auto data = std::make_unique<ArchiveData>();
			if (!data->open(archive_path)) data.reset();
			return s_archives.emplace(key, std::move(data)).first->second.get();
		}

		void close(const std::string& archive_path)
		{
			std::lock_guard<std::mutex> lock(s_mutex);
			s_archives.erase(key_of(archive_path));
		}

		//the archive of a path (its first part that is an archive file) and the entry in it
		bool split(const std::string& path, ArchiveData*& data, std::string& entry)
		{
			const std::string normalized = to_slash(path);
			for (size_t end = normalized.find('/'), start = 0; start < normalized.size(); end = normalized.find('/', start))
			{
				const size_t part_end = end == std::string::npos ? normalized.size() : end;
				const std::string archive_path = normalized.substr(0, part_end);
				if (has_archive_extension(normalized.substr(start, part_end - start)) && disk_is_file(archive_path))
				{
					data = open(archive_path);
					const std::string rest = end == std::string::npos ? std::string() : normalized.substr(end + 1);
					return data && normalize_entry(rest, entry);
				}
				if (end == std::string::npos) break;
				start = end + 1;
			}
			return false;
		}

		//////////////////////////////////////////////////////////////////////////////////////
		//writing
		zip_fileinfo now_file_info()
		{
			zip_fileinfo info{};
			const std::time_t now = std::time(nullptr);
			if (const std::tm* local = std::localtime(&now))
			{
				info.tmz_date.tm_sec  = local->tm_sec;
				info.tmz_date.tm_min  = local->tm_min;
				info.tmz_date.tm_hour = local->tm_hour;
				info.tmz_date.tm_mday = local->tm_mday;
				info.tmz_date.tm_mon  = local->tm_mon;
				info.tmz_date.tm_year = local->tm_year + 1900;
			}
			return info;
		}

		//the files of a directory, its sub directories too (the same order every time)
		std::vector<std::filesystem::path> files_of(const std::string& directory)
		{
			namespace fs = std::filesystem;
			std::vector<fs::path> paths;
			std::error_code error;
			for (fs::recursive_directory_iterator it(directory, error), end; !error && it != end; it.increment(error))
			{
				if (it->is_regular_file(error)) paths.push_back(it->path());
			}
			std::sort(paths.begin(), paths.end());
			return paths;
		}

		//deflate level 6 (the default of zlib), the files compressed already stored
		bool write_entry(zipFile archive, const std::string& name, const std::vector<unsigned char>& bytes, const zip_fileinfo& info)
		{
			const bool store = is_compressed_file(name);
			const int method = store ? 0 : Z_DEFLATED;
			const int level = store ? 0 : 6;
			const int zip64 = bytes.size() >= 0xffffffffu;
			if (zipOpenNewFileInZip64(archive, name.c_str(), &info, nullptr, 0, nullptr, 0, nullptr, method, level, zip64) != ZIP_OK) return false;
			const bool written = bytes.empty() || zipWriteInFileInZip(archive, bytes.data(), unsigned(bytes.size())) == ZIP_OK;
			return zipCloseFileInZip(archive) == ZIP_OK && written;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//Archive
	bool is_directory(const std::string& path)
	{
		AuxArchive::ArchiveData* data = nullptr;
		std::string entry;
		return AuxArchive::split(path, data, entry) && data->m_directories.count(entry) != 0;
	}

	bool is_file(const std::string& path)
	{
		AuxArchive::ArchiveData* data = nullptr;
		std::string entry;
		return AuxArchive::split(path, data, entry) && data->m_files.count(entry) != 0;
	}

	bool read(const std::string& path, std::vector<unsigned char>& output)
	{
		AuxArchive::ArchiveData* data = nullptr;
		std::string entry;
		if (!AuxArchive::split(path, data, entry)) return false;
		auto it = data->m_files.find(entry);
		if (it == data->m_files.end()) return false;
		return data->read(it->second, output);
	}

	bool list(const std::string& path, std::vector<std::string>& files, std::vector<std::string>& directories)
	{
		AuxArchive::ArchiveData* data = nullptr;
		std::string entry;
		if (!AuxArchive::split(path, data, entry) || !data->m_directories.count(entry)) return false;
		files.reserve(data->m_files.size());
		directories.reserve(data->m_directories.size());
		for (const auto& file : data->m_files)
		{
			if (AuxArchive::parent_of(file.first) == entry) files.push_back(AuxArchive::name_of(file.first));
		}
		for (const auto& directory : data->m_directories)
		{
			if (!directory.empty() && AuxArchive::parent_of(directory) == entry) directories.push_back(AuxArchive::name_of(directory));
		}
		std::sort(files.begin(), files.end());
		return true;
	}

	bool write(const std::string& directory, const std::string& archive_path)
	{
		namespace fs = std::filesystem;
		std::error_code error;
		if (!fs::is_directory(directory, error)) return false;
		//open here: closed, its index changes
		AuxArchive::close(archive_path);
		zipFile archive = zipOpen64(archive_path.c_str(), APPEND_STATUS_CREATE);
		if (!archive) return false;
		const zip_fileinfo info = AuxArchive::now_file_info();
		bool success = true;
		for (const auto& path : AuxArchive::files_of(directory))
		{
			const std::string name = fs::relative(path, directory, error).generic_string();
			success = !error && AuxArchive::write_entry(archive, name, binary_file_read_all(path.string()), info);
			if (!success) break;
		}
		return zipClose(archive, nullptr) == ZIP_OK && success;
	}
}
}
}
