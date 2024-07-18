#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif					

#include <windows.h>

#pragma comment(lib, "DirectXTK.lib")
#pragma comment(lib, "dxguid.lib") 
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <SimpleMath.h>

#include "../Utils/RscPtr.h"

#include <string>
#include <shellapi.h>
#include <wrl.h>
