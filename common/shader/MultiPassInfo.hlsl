//
//  MultiPassInfo.hlsl
//  Square
//
//  Shared state for multi-pass techniques (e.g. cube/CSM shadows on backends
//  without geometry shaders). The host renders the same draw N times and updates
//  only this buffer between steps: "m_id" is the current pass index (cube face /
//  cascade / ...). It is the single thing that changes from one step to the next.
//
#pragma once

// the mask of the layers of the draw (CSM: the cascades of a caster), bit i: layer i
#define MULTI_PASS_HAS_LAYER(mask, id) ((((mask) >> (id)) & 1u) != 0u) // a layer in a mask

struct MultiPassStruct
{
	uint m_id;   // current multi-pass index (PassID)
	uint m_mask; // the layers (CSM cascades) of the draw, bit i: layer i
};

cbuffer MultiPass
{
	MultiPassStruct multi_pass;
}
