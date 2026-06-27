//
//  ParserStaticMesh.cpp
//  Square
//
//  Created by Gabriele Di Bari on 07/01/24.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#include <optional>
#include <vector>
#include <limits>
#include <cstring>
#include <cstddef>
#include "Square/Config.h"
#include "Square/Core/ByteOrder.h"
#include "Square/Data/ParserUtils.h"
#include "Square/Data/ParserStaticMesh.h"
#include "Square/Driver/Render.h"
#include "Square/Render/VertexLayout.h"

namespace Square
{
	namespace Parser
	{
		#pragma region Definition
		// Header
		// 4 byte: Magic
		// 1 byte: Version
		// 1 byte: Index or not
		// 1 byte: Submesh or not
		// 1 byte: Layout
		//			Position2D						 (1)
		// 			Position3D					     (2)
		// 			Position2DUV                     (3)
		//			Position3DUV                     (4)
		// 			Position3DNormalTangetBinomialUV (5)
		// 8 bytes: body offset
		// 
		// Body
		// 8 bytes: Vertex array size
		// 8 bytes: Vertex element size
		// N bytes:	Vertex data (N = Vertex array size * Vertex element size)
		// -- Optional
		// 8 bytes: Index array size
		// 8 bytes: Index element size
		// N bytes:	index data (N = Index array size * Index element size)
		// -- Optional
		// 8 bytes: Submesh array size
		// 8 bytes: Submesh element size
		// N bytes:	Submesh data (N = Submesh array size * Submesh element size)
		enum class StaticMeshLayoutType : char
		{
			SMLTYPE_POSITON2D = 0,
			SMLTYPE_POSITON3D = 1,
			SMLTYPE_POSITON2DUV = 2,
			SMLTYPE_POSITON3DUV = 3,
			SMLTYPE_POSITON3DTANGENTBINOMIALUV = 4,
			SMLTYPE_UNKNOWN = ~0
		};

		PACKED(struct StaticMeshHeader
		{
			char magic[4];
			bool index;
			bool submesh;
			StaticMeshLayoutType type;
			unsigned long long offset;
		});

		// Submesh element:
		// 1 byte: draw type:
		//			POINTS         (0)
		//			LINES          (1)
		//			LINE_LOOP      (2)
		//			TRIANGLES      (3)
		//			TRIANGLE_STRIP (4)
		// 8 bytes: index count
		// 8 bytes: index offset
		enum class StaticMeshDrawType : unsigned char
		{
			SMDTYPE_POINTS = 0,
			SMDTYPE_LINES = 1,
			SMDTYPE_LINE_LOOP = 2,
			SMDTYPE_TRIANGLES = 3,
			SMDTYPE_TRIANGLE_STRIP = 4
		};

		PACKED(struct StaticSubMesh
		{
			StaticMeshDrawType type;
			unsigned long long index_count;
			unsigned long long index_offset;
		});

		#pragma endregion

		#pragma region Parser
		Render::DrawType as_render_type(StaticMeshDrawType type)
		{
			switch (type)
			{
			default:
			case StaticMeshDrawType::SMDTYPE_POINTS:		  return Render::DrawType::DRAW_POINTS;
			case StaticMeshDrawType::SMDTYPE_LINES:			  return Render::DrawType::DRAW_LINES;
			case StaticMeshDrawType::SMDTYPE_LINE_LOOP:		  return Render::DrawType::DRAW_LINE_LOOP;
			case StaticMeshDrawType::SMDTYPE_TRIANGLES:		  return Render::DrawType::DRAW_TRIANGLES;
			case StaticMeshDrawType::SMDTYPE_TRIANGLE_STRIP:  return Render::DrawType::DRAW_TRIANGLE_STRIP;
			}
		}

		// A cursor over the binary blob being parsed: base pointer + size + the offset
		// we have read up to.
		struct BinaryReader
		{
			const unsigned char* data   = nullptr;
			size_t               size   = 0;
			size_t               offset = 0;
		};

		// A located, bounds-checked array block: pointer to the first element + count.
		struct ArrayBlock
		{
			const unsigned char* data  = nullptr;
			unsigned long long   count = 0;
		};

		// Locate the array block { count, element_size, data } at the reader's offset.
		// Validates the header, the element size and the (overflow-safe) content size
		// against the buffer, then advances the reader past the block.
		static std::optional<ArrayBlock> read_array_block(BinaryReader& reader, size_t element_size)
		{
			// the block starts with two unsigned long long: count + element_size
			constexpr size_t header_size = sizeof(unsigned long long) * 2;
			// the header must fit in the buffer
			if (reader.offset > reader.size || reader.size - reader.offset < header_size)
				return std::nullopt;
			// read the two sizes endian-safe (no unaligned load)
			const unsigned long long count           = read_le<unsigned long long>(reader.data + reader.offset);
			const unsigned long long stored_elem_size = read_le<unsigned long long>(reader.data + reader.offset + sizeof(unsigned long long));
			// the element must match the expected size (never zero)
			if (stored_elem_size != element_size)
				return std::nullopt;
			// content size = count * element_size, guarded against overflow
			if (count > std::numeric_limits<unsigned long long>::max() / element_size)
				return std::nullopt;
			const unsigned long long content_size = count * element_size;
			// the whole block (header + content) must fit in the buffer
			if (reader.size - reader.offset - header_size < content_size)
				return std::nullopt;
			// ok: point at the data, advance the cursor
			const ArrayBlock block{ reader.data + reader.offset + header_size, count };
			reader.offset += header_size + (size_t)content_size;
			return block;
		}

		// Copy a homogeneous array of trivially-copyable T at the reader's offset. The
		// bulk memcpy lands the payload in the vector's aligned storage; on a
		// little-endian host the bytes are already in the right order.
		template<typename T>
		static bool read_array(BinaryReader& reader, std::vector<T>& out)
		{
			const std::optional<ArrayBlock> block = read_array_block(reader, sizeof(T));
			if (!block) return false;
			out.resize((size_t)block->count);
			if (block->count) std::memcpy(out.data(), block->data, (size_t)block->count * sizeof(T));
			return true;
		}

		// Decode one packed StaticSubMesh element into a render submesh, field-by-field
		// (read_le) so the unaligned 8-byte counts stay aligned & endian-safe.
		static Render::Mesh::SubMeshList::value_type decode_submesh(const unsigned char* element)
		{
			return
			{
				as_render_type((StaticMeshDrawType)element[offsetof(StaticSubMesh, type)]),
				(unsigned int)read_le<unsigned long long>(element + offsetof(StaticSubMesh, index_count)),
				(unsigned int)read_le<unsigned long long>(element + offsetof(StaticSubMesh, index_offset)),
			};
		}

		// Read the submesh array at the reader's offset and return it already decoded
		// into render submeshes, ready to hand to the context.
		static std::optional<Render::Mesh::SubMeshList> read_submeshes(BinaryReader& reader)
		{
			const std::optional<ArrayBlock> block = read_array_block(reader, sizeof(StaticSubMesh));
			if (!block) return std::nullopt;
			Render::Mesh::SubMeshList submeshes;
			submeshes.reserve((size_t)block->count);
			for (unsigned long long i = 0; i < block->count; ++i)
				submeshes.push_back(decode_submesh(block->data + i * sizeof(StaticSubMesh)));
			return submeshes;
		}

		// The header decoded into aligned fields (the on-disk StaticMeshHeader is packed).
		struct ParsedHeader
		{
			bool                 has_index;
			bool                 has_submesh;
			StaticMeshLayoutType layout_type;
			unsigned long long   body_offset;
		};

		// Validate and decode the header at the start of the buffer. Single-byte fields
		// are read in place; the 8-byte offset is read endian-safe to avoid an unaligned
		// load on the packed field. Returns nullopt on a missing/invalid header.
		static std::optional<ParsedHeader> read_header(const unsigned char* ptr, size_t size)
		{
			static const char magic[4] = { 's', 'm', '3', 'd' };
			// the header must fit
			if (size < sizeof(StaticMeshHeader)) return std::nullopt;
			// magic
			if (std::memcmp(ptr + offsetof(StaticMeshHeader, magic), magic, 4) != 0) return std::nullopt;
			// fields
			ParsedHeader header;
			header.has_index   = ptr[offsetof(StaticMeshHeader, index)]   != 0;
			header.has_submesh = ptr[offsetof(StaticMeshHeader, submesh)] != 0;
			header.layout_type = (StaticMeshLayoutType)ptr[offsetof(StaticMeshHeader, type)];
			header.body_offset = read_le<unsigned long long>(ptr + offsetof(StaticMeshHeader, offset));
			// the body must start inside the buffer
			if (header.body_offset >= size) return std::nullopt;
			return header;
		}

		bool StaticMesh::parse(Context& default_context, const std::vector<unsigned char>& binary)
		{
			const unsigned char* binary_ptr = binary.data();
			return parse(default_context, binary_ptr, binary.size());
		}

		bool StaticMesh::parse(Context& context, const unsigned char*& ptr, size_t size)
		{
			// Header
			const std::optional<ParsedHeader> header = read_header(ptr, size);
			if (!header) return false;
			// reader positioned at the body
			BinaryReader reader{ ptr, size, (size_t)header->body_offset };
			// Read body (vertex array)
			switch (header->layout_type)
			{
				case StaticMeshLayoutType::SMLTYPE_POSITON2D:
				{
					std::vector<Render::Layout::Position2D> vertex;
					if (!read_array(reader, vertex)) return false;
					context.m_vertex = std::move(vertex);
				}
				break;
				case StaticMeshLayoutType::SMLTYPE_POSITON3D:
				{
					std::vector<Render::Layout::Position3D> vertex;
					if (!read_array(reader, vertex)) return false;
					context.m_vertex = std::move(vertex);
				}
				break;
				case StaticMeshLayoutType::SMLTYPE_POSITON2DUV:
				{
					std::vector<Render::Layout::Position2DUV> vertex;
					if (!read_array(reader, vertex)) return false;
					context.m_vertex = std::move(vertex);
				}
				break;
				case StaticMeshLayoutType::SMLTYPE_POSITON3DUV:
				{
					std::vector<Render::Layout::Position3DUV> vertex;
					if (!read_array(reader, vertex)) return false;
					context.m_vertex = std::move(vertex);
				}
				break;
				case StaticMeshLayoutType::SMLTYPE_POSITON3DTANGENTBINOMIALUV:
				{
					std::vector<Render::Layout::Position3DNormalTangetBinomialUV> vertex;
					if (!read_array(reader, vertex)) return false;
					context.m_vertex = std::move(vertex);
				}
				break;
				default: return false;
			}
			// Index
			if (header->has_index)
			{
				std::vector<unsigned int> index;
				if (!read_array(reader, index)) return false;
				context.m_index = std::move(index);
			}
			// SubMesh
			if (header->has_submesh)
			{
				std::optional<Render::Mesh::SubMeshList> submeshes = read_submeshes(reader);
				if (!submeshes) return false;
				context.m_submesh = std::move(*submeshes);
			}
			return true;
		}
		#pragma endregion

		#pragma region Serializer
		template < typename T >
		static void write_a_vector(const std::vector< T >& vector_t, std::vector<unsigned char>& binary)
		{
			unsigned long long size_array = (unsigned long long)vector_t.size();
			unsigned long long size_element = (unsigned long long)sizeof(T);
			unsigned long long size_bytes = size_array * size_element;
			// Offset in the binary array
			size_t offset = binary.size();
			// Alloc 
			binary.resize(offset
				        + sizeof(size_array)
						+ sizeof(size_element)
						+ size_bytes);
			// Pointer
			unsigned char* bytes = binary.data() + offset;
			// Copy: sizes little-endian (no unaligned store), payload bulk
			write_le<unsigned long long>(bytes, size_array);
			write_le<unsigned long long>(bytes + sizeof(size_array), size_element);
			std::memcpy(bytes + sizeof(size_array) + sizeof(size_element), vector_t.data(), size_bytes);
		}

		static bool write_vertex_buffer(StaticMesh::Context& context, std::vector<unsigned char>& binary)
		{
			bool status = true;
			context.visit(
				[&](auto&) { status = false; },
				[&](Render::Mesh::Vertex2DList& varray) { write_a_vector(varray, binary); },
				[&](Render::Mesh::Vertex3DList& varray) { write_a_vector(varray, binary); },
				[&](Render::Mesh::Vertex2DUVList& varray) { write_a_vector(varray, binary); },
				[&](Render::Mesh::Vertex3DUVList& varray) { write_a_vector(varray, binary); },
				[&](Render::Mesh::Vertex3DNTBUVList& varray) { write_a_vector(varray, binary); }
			);
			return status;
		}

		static StaticMeshLayoutType get_type_of_layout(StaticMesh::Context& context)
		{
			StaticMeshLayoutType type = StaticMeshLayoutType::SMLTYPE_UNKNOWN;
			context.visit(
				[&](auto&) { /* nope */ },
				[&](Render::Mesh::Vertex2DList&) { type = StaticMeshLayoutType::SMLTYPE_POSITON2D; },
				[&](Render::Mesh::Vertex3DList&) { type = StaticMeshLayoutType::SMLTYPE_POSITON3D; },
				[&](Render::Mesh::Vertex2DUVList&) { type = StaticMeshLayoutType::SMLTYPE_POSITON2DUV; },
				[&](Render::Mesh::Vertex3DUVList&) { type = StaticMeshLayoutType::SMLTYPE_POSITON3DUV; },
				[&](Render::Mesh::Vertex3DNTBUVList&) { type = StaticMeshLayoutType::SMLTYPE_POSITON3DTANGENTBINOMIALUV; }
			);
			return type;
		}

		static StaticMeshDrawType as_static_mesh_draw_type(Render::DrawType type)
		{
			switch (type)
			{
			default:
			case Render::DrawType::DRAW_POINTS:         return StaticMeshDrawType::SMDTYPE_POINTS;
			case Render::DrawType::DRAW_LINES:          return StaticMeshDrawType::SMDTYPE_LINES;
			case Render::DrawType::DRAW_LINE_LOOP:      return StaticMeshDrawType::SMDTYPE_LINE_LOOP;
			case Render::DrawType::DRAW_TRIANGLES:      return StaticMeshDrawType::SMDTYPE_TRIANGLES;
			case Render::DrawType::DRAW_TRIANGLE_STRIP: return StaticMeshDrawType::SMDTYPE_TRIANGLE_STRIP;
			}
		}

		// Mirror of read_header: write the header at the start of the buffer, single-byte
		// fields in place and the 8-byte body offset little-endian (no unaligned store).
		static void write_header(std::vector<unsigned char>& binary, bool has_index, bool has_submesh, StaticMeshLayoutType layout_type)
		{
			static const char magic[4] = { 's', 'm', '3', 'd' };
			binary.resize(sizeof(StaticMeshHeader));
			unsigned char* bytes = binary.data();
			std::memcpy(bytes + offsetof(StaticMeshHeader, magic), magic, 4);
			bytes[offsetof(StaticMeshHeader, index)]   = has_index   ? 1 : 0;
			bytes[offsetof(StaticMeshHeader, submesh)] = has_submesh ? 1 : 0;
			bytes[offsetof(StaticMeshHeader, type)]    = (unsigned char)layout_type;
			// the body starts right after the header
			write_le<unsigned long long>(bytes + offsetof(StaticMeshHeader, offset), (unsigned long long)sizeof(StaticMeshHeader));
		}

		// Encode one render submesh into a packed StaticSubMesh slot (mirror of
		// decode_submesh): single-byte type in place, 8-byte counts little-endian.
		static void encode_submesh(unsigned char* element, const Render::Mesh::SubMeshList::value_type& sub_mesh)
		{
			element[offsetof(StaticSubMesh, type)] = (unsigned char)as_static_mesh_draw_type(sub_mesh.m_draw_type);
			write_le<unsigned long long>(element + offsetof(StaticSubMesh, index_count),  (unsigned long long)sub_mesh.m_index_count);
			write_le<unsigned long long>(element + offsetof(StaticSubMesh, index_offset), (unsigned long long)sub_mesh.m_index_offset);
		}

		// Write the submesh array { count, element_size, data } endian-safe, encoding each
		// element field-by-field (no packed temporary).
		static void write_submeshes(const Render::Mesh::SubMeshList& submeshes, std::vector<unsigned char>& binary)
		{
			const unsigned long long count        = (unsigned long long)submeshes.size();
			const unsigned long long element_size = (unsigned long long)sizeof(StaticSubMesh);
			constexpr size_t         header_size  = sizeof(unsigned long long) * 2;
			// alloc header + content
			const size_t offset = binary.size();
			binary.resize(offset + header_size + (size_t)(count * element_size));
			unsigned char* bytes = binary.data() + offset;
			// header (count + element_size)
			write_le<unsigned long long>(bytes, count);
			write_le<unsigned long long>(bytes + sizeof(unsigned long long), element_size);
			// elements
			unsigned char* element = bytes + header_size;
			for (const auto& sub_mesh : submeshes)
			{
				encode_submesh(element, sub_mesh);
				element += sizeof(StaticSubMesh);
			}
		}

		bool StaticMesh::serialize(Context& context, std::vector<unsigned char>& binary)
		{
			// Header (the body follows immediately)
			write_header(binary, !!context.m_index.size(), !!context.m_submesh.size(), get_type_of_layout(context));
			// Write vertex array
			if (!write_vertex_buffer(context, binary)) return false;
			// Write index buffer
			if (context.m_index.size()) write_a_vector(context.m_index, binary);
			// Write submeshs
			if (context.m_submesh.size()) write_submeshes(context.m_submesh, binary);
			// Ok
			return true;
		}
		#pragma endregion
	}
}