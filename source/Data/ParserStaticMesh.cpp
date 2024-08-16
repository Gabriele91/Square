//
//  ParserStaticMesh.cpp
//  Square
//
//  Created by Gabriele Di Bari on 07/01/24.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include <optional>
#include <vector>
#include "Square/Config.h"
#include "Square/Data/ParserUtils.h"
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
		enum class StaticMeshLayoutType : unsigned char
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

		template<typename T>
		struct StaticMeshArray
		{
			unsigned long long array_size;
			unsigned long long element_size;
			T data[0];
			// Size in byte
			size_t content_size() const { return size_t(array_size * element_size); }
			size_t full_size() const { return sizeof(array_size) + sizeof(element_size) + content_size(); }
			// As vector
			std::vector< T > as_vector() const 
			{
				// Test
				if (element_size != sizeof(T)) return {};
				// Copy
				return std::vector< T >{ data, data + array_size };
			}
		};

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

		#pragma region Perser
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

		bool StaticMesh::parse(Context& default_context, const std::vector<unsigned char>& binary)
		{
			const unsigned char* binary_ptr = binary.data();
			return parse(default_context, binary_ptr, binary.size());
		}

		bool StaticMesh::parse(Context& context, const unsigned char*& ptr, size_t size)
		{
			// consts
			const char magic[4] = { 's', 'm', '3', 'd' };
			// Test
			if (size < sizeof(StaticMeshHeader)) return false;
			// Header
			const StaticMeshHeader& header = *reinterpret_cast<const StaticMeshHeader*>(ptr);
			// Test magic
			if (memcmp(header.magic, magic, 4) != 0) return false;
			// Test body
			if (header.offset >= size) return false;
			// Offset
			size_t offset = header.offset;
			// Read body
			switch (header.type)
			{
				case StaticMeshLayoutType::SMLTYPE_POSITON2D:
				{
					auto& p2 = *reinterpret_cast<const StaticMeshArray<Render::Layout::Position2D>*>(&ptr[offset]);
					context.m_vertex = p2.as_vector();
					offset += p2.full_size();
				}
				break;
				case StaticMeshLayoutType::SMLTYPE_POSITON3D:
				{
					auto& p3 = *reinterpret_cast<const StaticMeshArray<Render::Layout::Position3D>*>(&ptr[offset]);
					context.m_vertex = p3.as_vector();
					offset += p3.full_size();
				}
				break;
				case StaticMeshLayoutType::SMLTYPE_POSITON2DUV:
				{
					auto& p2uv = *reinterpret_cast<const StaticMeshArray<Render::Layout::Position2DUV>*>(&ptr[offset]);
					context.m_vertex = p2uv.as_vector();
					offset += p2uv.full_size();
				}
				break;
				case StaticMeshLayoutType::SMLTYPE_POSITON3DUV:
				{
					auto& p3uv = *reinterpret_cast<const StaticMeshArray<Render::Layout::Position3DUV>*>(&ptr[offset]);
					context.m_vertex = p3uv.as_vector();
					offset += p3uv.full_size();
				}
				break;
				case StaticMeshLayoutType::SMLTYPE_POSITON3DTANGENTBINOMIALUV:
				{
					auto& p3nbuv = *reinterpret_cast<const StaticMeshArray<Render::Layout::Position3DNormalTangetBinomialUV>*>(&ptr[offset]);
					context.m_vertex = p3nbuv.as_vector();
					offset += p3nbuv.full_size();
				}
				break;
				default: return false;
			}
			// Index
			if (header.index)
			{
				auto& index = *reinterpret_cast<const StaticMeshArray<unsigned int>*>(&ptr[offset]);
				context.m_index = index.as_vector();
			}
			// SubMesh
			if (header.submesh)
			{
				auto& submesh = *reinterpret_cast<const StaticMeshArray<StaticSubMesh>*>(&ptr[offset]);
				auto memory_submesh = std::move(submesh.as_vector());
				for (auto& submesh : memory_submesh)
					context.m_submesh.push_back
					({
						as_render_type(submesh.type),
						(unsigned int)submesh.index_count,
						(unsigned int)submesh.index_offset,
					});
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
			// Copy
			*(reinterpret_cast<unsigned long long*>(bytes)) = size_array;
			*(reinterpret_cast<unsigned long long*>(bytes + sizeof(size_array))) = size_element;
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

		bool StaticMesh::serialize(Context& context, std::vector<unsigned char>& binary)
		{
			// Alloc header
			binary.resize(sizeof(StaticMeshHeader));
			// Write header
			*(reinterpret_cast<StaticMeshHeader*>(binary.data())) = StaticMeshHeader
			{
				{ 's', 'm', '3', 'd' },
				!!context.m_index.size(),
				!!context.m_submesh.size(),
				get_type_of_layout(context),
				sizeof(StaticMeshHeader)
			};
			// Write vertex array
			if (!write_vertex_buffer(context, binary)) return false;
			// Write index buffer
			if (context.m_index.size()) write_a_vector(context.m_index, binary);
			// Write submeshs
			if (context.m_submesh.size())
			{
				// Alloc tmp vector
				std::vector< StaticSubMesh > static_sub_mesh_tmp;
				static_sub_mesh_tmp.reserve(context.m_submesh.size());
				// Copy fill tmp vector
				for (auto& sub_mesh : context.m_submesh)
				{
					static_sub_mesh_tmp.emplace_back(
						StaticSubMesh
						{
							as_static_mesh_draw_type(sub_mesh.m_draw_type),
							(unsigned long long)sub_mesh.m_index_count,
							(unsigned long long)sub_mesh.m_index_offset
						}
					);
				}
				// Write tmp vector
				write_a_vector(static_sub_mesh_tmp, binary);
			}
			// Ok
			return true;
		}
		#pragma endregion
	}
}