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

namespace Square
{
namespace Render
{
	CBStruct UniformMultiPass
	{
		uint32 m_id { 0 }; // current multi-pass index (PassID)
	};
}
}
