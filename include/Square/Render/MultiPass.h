//
//  MultiPass.h
//  Square
//
//  CPU-side mirror of common/shader/MultiPassInfo.hlsl. Carries the single piece
//  of state that changes between the steps of a multi-pass technique (the current
//  cube face / CSM cascade / ... index).
//
#pragma once
#include "Square/Config.h"
#include "Square/Render/ConstantBuffer.h"

// the mask of the layers of a multi-pass draw (CSM: the cascades of a caster), bit i: layer i
#define MULTI_PASS_ALL_LAYERS 0xFFFFFFFFu                          // every layer
#define MULTI_PASS_LAYER_BIT(id) (1u << (id))                      // the bit of a layer
#define MULTI_PASS_HAS_LAYER(mask, id) ((((mask) >> (id)) & 1u) != 0u) // a layer in a mask

namespace Square
{
namespace Render
{
	CBStruct UniformMultiPass
	{
		uint32 m_id { 0 };                       // current multi-pass index (PassID)
		uint32 m_mask { MULTI_PASS_ALL_LAYERS }; // the layers (CSM cascades) of the draw
	};
}
}
