//
//  Square
//
//  Created by Gabriele on 29/06/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
//Engine include
#include "Square/Config.h"
//Std include
#include <sstream>
//OS Include
#ifdef _WIN32
#include <windows.h>
#include <wincodec.h>
#include <d3d11_3.h>
#include <DXGI1_3.h>
#include <d3dcompiler.h>
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"Dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"d3dcompiler.lib")

#else 
#error OS not supported
#endif

namespace Square
{
namespace Debug
{
	//none
}
}
