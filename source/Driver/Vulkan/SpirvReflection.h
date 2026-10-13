//
//  SpirvReflection.h
//  Square, the driver of Vulkan
//
//  What a stage of SPIR-V has, read from its words: its resources (by their binding: the layout
//  of HLSL2ALL for Vulkan, one binding for each resource of a program, set 0), the sizes of its
//  blocks, the members of its global block ("$Global": the loose uniforms), the inputs of a
//  vertex stage (their location: Render::AttributeType), what it needs of the device.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace Square
{
namespace Render
{
namespace SpirvReflection
{
	//the kind of a resource (its descriptor)
	enum class ResourceKind : unsigned char
	{
		UNIFORM_BUFFER,
		COMBINED_IMAGE_SAMPLER,
		SAMPLED_IMAGE,
		SAMPLER,
		STORAGE_BUFFER,
		UNKNOWN
	};

	//the shape of an image (SPIR-V Dim)
	enum class ImageDim : unsigned char
	{
		DIM_1D,
		DIM_2D,
		DIM_3D,
		DIM_CUBE,
		DIM_OTHER
	};

	//a resource of the stage
	struct Resource
	{
		std::string  m_name;               //its block (a buffer), its variable (an image, a sampler)
		uint32_t     m_binding{ 0 };
		uint32_t     m_set{ 0 };
		uint32_t     m_count{ 1 };         //an array: its length
		ResourceKind m_kind{ ResourceKind::UNKNOWN };
		uint32_t     m_size{ 0 };          //a buffer: the size of its block
		ImageDim     m_dim{ ImageDim::DIM_2D };
		bool         m_arrayed{ false };
		bool         m_global{ false };    //the global block of the loose uniforms ("$Global")
	};

	//a member of the global block (a loose uniform)
	struct Member
	{
		std::string m_name;
		uint32_t    m_offset{ 0 };
		uint32_t    m_size{ 0 };
	};

	//an input of a vertex stage
	struct Input
	{
		uint32_t m_location{ 0 };
		uint32_t m_components{ 4 };
		bool     m_integer{ false };
		bool     m_signed{ false };
	};

	//a stage of SPIR-V read
	struct Stage
	{
		std::vector<Resource> m_resources;
		std::vector<Member>   m_members;         //of the global block
		std::vector<Input>    m_inputs;          //a vertex stage: its vertex attributes
		bool                  m_geometry{ false };      //capability Geometry
		bool                  m_writes_layer{ false };  //a vertex stage writes the layer (ShaderLayer, ShaderViewportIndexLayerEXT)
		std::string           m_error;           //not empty: not read
	};

	//a stage read from its words (as bytes: the source of the driver)
	Stage read(const std::string& bytes);
}
}
}
