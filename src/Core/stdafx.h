#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif					

#include <windows.h>


#pragma comment(lib, "dxguid.lib") 
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>

#include <D3D12MemoryAllocator/include/D3D12MemAlloc.h>

#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <shellapi.h>
#include <wrl.h>

#include <RscPtr.h>
#include <D3D12Math.h>
#include <DXSampleHelper.h>