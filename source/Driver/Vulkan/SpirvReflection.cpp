//
//  SpirvReflection.cpp
//  Square, the driver of Vulkan
//
#include "SpirvReflection.h"
#include <cstring>
#include <algorithm>
#include <unordered_map>

namespace Square
{
namespace Render
{
namespace SpirvReflection
{
	namespace AuxSpirvReflection
	{
		//the SPIR-V words used here (the specification: opcodes, decorations, enumerants)
		static constexpr uint32_t s_magic = 0x07230203;
		static constexpr uint32_t s_header = 5;
		enum Op : uint32_t
		{
			OP_NAME = 5,
			OP_MEMBER_NAME = 6,
			OP_ENTRY_POINT = 15,
			OP_CAPABILITY = 17,
			OP_TYPE_BOOL = 20,
			OP_TYPE_INT = 21,
			OP_TYPE_FLOAT = 22,
			OP_TYPE_VECTOR = 23,
			OP_TYPE_MATRIX = 24,
			OP_TYPE_IMAGE = 25,
			OP_TYPE_SAMPLER = 26,
			OP_TYPE_SAMPLED_IMAGE = 27,
			OP_TYPE_ARRAY = 28,
			OP_TYPE_RUNTIME_ARRAY = 29,
			OP_TYPE_STRUCT = 30,
			OP_TYPE_POINTER = 32,
			OP_CONSTANT = 43,
			OP_VARIABLE = 59,
			OP_DECORATE = 71,
			OP_MEMBER_DECORATE = 72
		};
		enum Decoration : uint32_t
		{
			DEC_BLOCK = 2,
			DEC_BUFFER_BLOCK = 3,
			DEC_ROW_MAJOR = 4,
			DEC_ARRAY_STRIDE = 6,
			DEC_MATRIX_STRIDE = 7,
			DEC_BUILT_IN = 11,
			DEC_LOCATION = 30,
			DEC_BINDING = 33,
			DEC_DESCRIPTOR_SET = 34,
			DEC_OFFSET = 35
		};
		enum Storage : uint32_t
		{
			SC_UNIFORM_CONSTANT = 0,
			SC_INPUT = 1,
			SC_UNIFORM = 2,
			SC_STORAGE_BUFFER = 12
		};
		static constexpr uint32_t s_capability_geometry = 2;
		static constexpr uint32_t s_capability_shader_layer = 69;
		static constexpr uint32_t s_capability_viewport_index_layer = 5254;
		static constexpr uint32_t s_execution_model_vertex = 0;

		//a type of the module
		struct Type
		{
			uint32_t m_op{ 0 };
			std::vector<uint32_t> m_operands; //the words after its result id
		};

		//a member of a struct: its decorations
		struct MemberDecorations
		{
			uint32_t m_offset{ 0 };
			uint32_t m_matrix_stride{ 0 };
			bool     m_row_major{ false };
		};

		//the module read
		struct Module
		{
			std::unordered_map<uint32_t, std::string> m_names;
			std::unordered_map<uint64_t, std::string> m_member_names;          //(struct, member)
			std::unordered_map<uint32_t, Type>        m_types;
			std::unordered_map<uint32_t, uint32_t>    m_constants;              //id -> value (32 bit)
			std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>> m_decorations; //id -> decoration -> value
			std::unordered_map<uint64_t, MemberDecorations> m_member_decorations; //(struct, member)
			std::vector<std::pair<uint32_t, uint32_t>> m_variables;           //id, its pointer type
			std::vector<uint32_t> m_capabilities;
			bool m_vertex{ false };
		};

		static uint64_t key(uint32_t a, uint32_t b)
		{
			return (uint64_t(a) << 32) | uint64_t(b);
		}

		//a literal string from a word (its bytes up to the first zero)
		static std::string literal(const uint32_t* words, size_t count)
		{
			std::string out;
			for (size_t i = 0; i != count; ++i)
			{
				for (uint32_t shift = 0; shift != 32; shift += 8)
				{
					const char c = char((words[i] >> shift) & 0xFF);
					if (c == 0)
					{
						return out;
					}
					out += c;
				}
			}
			return out;
		}

		static bool has(const Module& module, uint32_t id, uint32_t decoration)
		{
			auto it = module.m_decorations.find(id);
			return it != module.m_decorations.end() && it->second.count(decoration);
		}

		static uint32_t decoration(const Module& module, uint32_t id, uint32_t decoration, uint32_t otherwise)
		{
			auto it = module.m_decorations.find(id);
			if (it != module.m_decorations.end())
			{
				auto value = it->second.find(decoration);
				if (value != it->second.end())
				{
					return value->second;
				}
			}
			return otherwise;
		}

		static const Type* type_of(const Module& module, uint32_t id)
		{
			auto it = module.m_types.find(id);
			return it != module.m_types.end() ? &it->second : nullptr;
		}

		//the size of a type in a block (its decorations: the strides, the offsets)
		static uint32_t size_of(const Module& module, uint32_t id, const MemberDecorations& member);

		static uint32_t size_of_struct(const Module& module, uint32_t id, const Type& type)
		{
			uint32_t size = 0;
			for (uint32_t i = 0; i != uint32_t(type.m_operands.size()); ++i)
			{
				MemberDecorations member;
				auto it = module.m_member_decorations.find(key(id, i));
				if (it != module.m_member_decorations.end())
				{
					member = it->second;
				}
				size = std::max(size, member.m_offset + size_of(module, type.m_operands[i], member));
			}
			return size;
		}

		static uint32_t size_of(const Module& module, uint32_t id, const MemberDecorations& member)
		{
			const Type* type = type_of(module, id);
			uint32_t size = 0;
			if (type)
			{
				switch (type->m_op)
				{
				case OP_TYPE_BOOL:
					size = 4;
					break;
				case OP_TYPE_INT:
				case OP_TYPE_FLOAT:
					size = type->m_operands[0] / 8;
					break;
				case OP_TYPE_VECTOR:
					size = type->m_operands[1] * size_of(module, type->m_operands[0], member);
					break;
				case OP_TYPE_MATRIX:
				{
					const uint32_t columns = type->m_operands[1];
					const Type* column = type_of(module, type->m_operands[0]);
					const uint32_t rows = column && column->m_op == OP_TYPE_VECTOR ? column->m_operands[1] : 4;
					if (member.m_matrix_stride)
					{
						size = (member.m_row_major ? rows : columns) * member.m_matrix_stride;
					}
					else
					{
						size = columns * size_of(module, type->m_operands[0], member);
					}
				}
				break;
				case OP_TYPE_ARRAY:
				{
					auto length = module.m_constants.find(type->m_operands[1]);
					const uint32_t count = length != module.m_constants.end() ? length->second : 1;
					const uint32_t stride = decoration(module, id, DEC_ARRAY_STRIDE, 0);
					size = count * (stride ? stride : size_of(module, type->m_operands[0], member));
				}
				break;
				case OP_TYPE_STRUCT:
					size = size_of_struct(module, id, *type);
					break;
				default:
					break;
				}
			}
			return size;
		}

		//the module from its words (false: not SPIR-V)
		static bool read_module(const uint32_t* words, size_t count, Module& module)
		{
			if (count < s_header || words[0] != s_magic)
			{
				return false;
			}
			size_t at = s_header;
			while (at < count)
			{
				const uint32_t op = words[at] & 0xFFFF;
				const uint32_t length = words[at] >> 16;
				if (length == 0 || at + length > count)
				{
					return false;
				}
				const uint32_t* operand = words + at + 1;
				const size_t operands = length - 1;
				switch (op)
				{
				case OP_NAME:
					module.m_names[operand[0]] = literal(operand + 1, operands - 1);
					break;
				case OP_MEMBER_NAME:
					module.m_member_names[key(operand[0], operand[1])] = literal(operand + 2, operands - 2);
					break;
				case OP_ENTRY_POINT:
					module.m_vertex = module.m_vertex || operand[0] == s_execution_model_vertex;
					break;
				case OP_CAPABILITY:
					module.m_capabilities.push_back(operand[0]);
					break;
				case OP_TYPE_BOOL:
				case OP_TYPE_INT:
				case OP_TYPE_FLOAT:
				case OP_TYPE_VECTOR:
				case OP_TYPE_MATRIX:
				case OP_TYPE_IMAGE:
				case OP_TYPE_SAMPLER:
				case OP_TYPE_SAMPLED_IMAGE:
				case OP_TYPE_ARRAY:
				case OP_TYPE_RUNTIME_ARRAY:
				case OP_TYPE_STRUCT:
				case OP_TYPE_POINTER:
				{
					Type type;
					type.m_op = op;
					type.m_operands.assign(operand + 1, operand + operands);
					module.m_types[operand[0]] = type;
				}
				break;
				case OP_CONSTANT:
					if (operands >= 3)
					{
						module.m_constants[operand[1]] = operand[2];
					}
					break;
				case OP_VARIABLE:
					module.m_variables.push_back({ operand[1], operand[0] });
					break;
				case OP_DECORATE:
					module.m_decorations[operand[0]][operand[1]] = operands >= 3 ? operand[2] : 1;
					break;
				case OP_MEMBER_DECORATE:
				{
					MemberDecorations& member = module.m_member_decorations[key(operand[0], operand[1])];
					switch (operand[2])
					{
					case DEC_OFFSET:        member.m_offset = operands >= 4 ? operand[3] : 0;        break;
					case DEC_MATRIX_STRIDE: member.m_matrix_stride = operands >= 4 ? operand[3] : 0; break;
					case DEC_ROW_MAJOR:     member.m_row_major = true;                               break;
					default: break;
					}
				}
				break;
				default:
					break;
				}
				at += length;
			}
			return true;
		}

		//the name of a resource: its block (a buffer), else its variable
		static std::string resource_name(const Module& module, uint32_t variable, uint32_t type)
		{
			auto block = module.m_names.find(type);
			const bool named_block = block != module.m_names.end() && !block->second.empty();
			const Type* t = type_of(module, type);
			if (t && t->m_op == OP_TYPE_STRUCT && named_block)
			{
				return block->second;
			}
			auto name = module.m_names.find(variable);
			return name != module.m_names.end() ? name->second : std::string();
		}

		static ImageDim image_dim(uint32_t dim)
		{
			switch (dim)
			{
			case 0:  return ImageDim::DIM_1D;
			case 1:  return ImageDim::DIM_2D;
			case 2:  return ImageDim::DIM_3D;
			case 3:  return ImageDim::DIM_CUBE;
			default: return ImageDim::DIM_OTHER;
			}
		}
	}

	Stage read(const std::string& bytes)
	{
		using namespace AuxSpirvReflection;
		Stage stage;
		std::vector<uint32_t> words(bytes.size() / sizeof(uint32_t));
		std::memcpy(words.data(), bytes.data(), words.size() * sizeof(uint32_t));
		Module module;
		if (!read_module(words.data(), words.size(), module))
		{
			stage.m_error = "not a SPIR-V module";
			return stage;
		}
		for (uint32_t capability : module.m_capabilities)
		{
			stage.m_geometry = stage.m_geometry || capability == s_capability_geometry;
			const bool layer = capability == s_capability_shader_layer || capability == s_capability_viewport_index_layer;
			stage.m_writes_layer = stage.m_writes_layer || (layer && module.m_vertex);
		}
		for (const auto& variable : module.m_variables)
		{
			const Type* pointer = type_of(module, variable.second);
			if (!pointer || pointer->m_op != OP_TYPE_POINTER) continue;
			const uint32_t storage = pointer->m_operands[0];
			uint32_t type_id = pointer->m_operands[1];
			//an input of the vertex stage (not a built-in)
			if (storage == SC_INPUT)
			{
				const bool attribute = module.m_vertex && !has(module, variable.first, DEC_BUILT_IN) && has(module, variable.first, DEC_LOCATION);
				if (attribute)
				{
					Input input;
					input.m_location = decoration(module, variable.first, DEC_LOCATION, 0);
					const Type* type = type_of(module, type_id);
					if (type && type->m_op == OP_TYPE_VECTOR)
					{
						input.m_components = type->m_operands[1];
						type = type_of(module, type->m_operands[0]);
					}
					else
					{
						input.m_components = 1;
					}
					input.m_integer = type && type->m_op == OP_TYPE_INT;
					input.m_signed = input.m_integer && type->m_operands.size() > 1 && type->m_operands[1] != 0;
					stage.m_inputs.push_back(input);
				}
				continue;
			}
			const bool resource = storage == SC_UNIFORM_CONSTANT || storage == SC_UNIFORM || storage == SC_STORAGE_BUFFER;
			if (!resource) continue;
			Resource out;
			//an array of resources: its element, its length
			const Type* type = type_of(module, type_id);
			while (type && (type->m_op == OP_TYPE_ARRAY || type->m_op == OP_TYPE_RUNTIME_ARRAY))
			{
				if (type->m_op == OP_TYPE_ARRAY)
				{
					auto length = module.m_constants.find(type->m_operands[1]);
					out.m_count *= length != module.m_constants.end() ? length->second : 1;
				}
				type_id = type->m_operands[0];
				type = type_of(module, type_id);
			}
			if (!type) continue;
			out.m_name = resource_name(module, variable.first, type_id);
			out.m_binding = decoration(module, variable.first, DEC_BINDING, 0);
			out.m_set = decoration(module, variable.first, DEC_DESCRIPTOR_SET, 0);
			switch (type->m_op)
			{
			case OP_TYPE_STRUCT:
			{
				const bool storage_buffer = storage == SC_STORAGE_BUFFER || has(module, type_id, DEC_BUFFER_BLOCK);
				out.m_kind = storage_buffer ? ResourceKind::STORAGE_BUFFER : ResourceKind::UNIFORM_BUFFER;
				out.m_size = (size_of_struct(module, type_id, *type) + 15u) & ~15u;
				out.m_global = out.m_name.find("Global") != std::string::npos;
				//the loose uniforms: the members of the global block
				if (out.m_global)
				{
					for (uint32_t i = 0; i != uint32_t(type->m_operands.size()); ++i)
					{
						Member member;
						auto name = module.m_member_names.find(key(type_id, i));
						member.m_name = name != module.m_member_names.end() ? name->second : std::string();
						MemberDecorations decorations;
						auto it = module.m_member_decorations.find(key(type_id, i));
						if (it != module.m_member_decorations.end())
						{
							decorations = it->second;
						}
						member.m_offset = decorations.m_offset;
						member.m_size = size_of(module, type->m_operands[i], decorations);
						if (!member.m_name.empty())
						{
							stage.m_members.push_back(member);
						}
					}
				}
			}
			break;
			case OP_TYPE_SAMPLED_IMAGE:
			{
				out.m_kind = ResourceKind::COMBINED_IMAGE_SAMPLER;
				const Type* image = type_of(module, type->m_operands[0]);
				if (image && image->m_op == OP_TYPE_IMAGE)
				{
					out.m_dim = image_dim(image->m_operands[1]);
					out.m_arrayed = image->m_operands[3] != 0;
				}
			}
			break;
			case OP_TYPE_IMAGE:
				out.m_kind = ResourceKind::SAMPLED_IMAGE;
				out.m_dim = image_dim(type->m_operands[1]);
				out.m_arrayed = type->m_operands[3] != 0;
				break;
			case OP_TYPE_SAMPLER:
				out.m_kind = ResourceKind::SAMPLER;
				break;
			default:
				continue;
			}
			stage.m_resources.push_back(out);
		}
		return stage;
	}
}
}
}
