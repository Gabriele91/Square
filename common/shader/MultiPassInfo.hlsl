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

struct MultiPassStruct
{
	uint m_id; // current multi-pass index (PassID)
};

cbuffer MultiPass
{
	MultiPassStruct multi_pass;
}
