#pragma once

#pragma once
#include <Square/Square.h>

namespace Square
{
namespace Data 
{
namespace GLB
{
	static constexpr const char extension[4]{ '.','g','l','b' };

	enum class ChunkType : uint32
	{
		CT_JSON = 0x4E4F534A,
		CT_BIN  = 0x004E4942
	};

	PACKED(struct Chunk
	{
		uint32 lenght;
		ChunkType type;
	});

	PACKED(struct Header
	{
		char   magic[4];
		uint32 version;
		uint32 length;
	});
	
	struct GLB
	{
		std::string json;
		std::vector<unsigned char> bin;
		GLB() = default;
		GLB(GLB&& glb) = default;
		GLB(const GLB& glb) = default;
	};

	std::optional<GLB> decode_glb(const std::string& path)
	{
		std::vector<unsigned char> buffer = std::move(Filesystem::binary_file_read_all(path));
		// Test
		if (buffer.size() < sizeof(Header))
			return {};
		// Read header
		const Header* header = reinterpret_cast<const Header*>(buffer.data());
		// Test 
		if (header->version != 2
		||  strncmp(header->magic,"glTF",4) != 0
		||  header->length < (sizeof(Header) + sizeof(Chunk)))
		{
			return {};
		}
		// End
		const unsigned char* file_end = buffer.data() + header->length;
		const unsigned char* ptr = buffer.data() + sizeof(Header);
		// Output GLB
		GLB glb_output;
		// Each Chunk
		while(ptr < file_end && ptr < &buffer.back())
		{ 
			const Chunk* chunk = reinterpret_cast<const Chunk*>(ptr);

			switch (chunk->type)
			{
			case ChunkType::CT_JSON:
			{
				const char* start_string = reinterpret_cast<const char*>(ptr + sizeof(Chunk));
				const char* end_string = reinterpret_cast<const char*>(ptr + sizeof(Chunk) + chunk->lenght);
				glb_output.json = std::string(start_string, end_string);
			}
			break;
			case ChunkType::CT_BIN:
			{
				const unsigned char* start_bin = ptr + sizeof(Chunk);
				const unsigned char* end_bin = ptr + sizeof(Chunk) + chunk->lenght;
				glb_output.bin = std::vector<unsigned char>(start_bin, end_bin);
			}
			break;
			default: break;
			}

			ptr += sizeof(Chunk) + chunk->lenght;
		}
		return glb_output;
	}
}
}
}