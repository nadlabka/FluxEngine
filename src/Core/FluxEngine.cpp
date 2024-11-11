#include <stdafx.h>
#include "FluxEngine.h"
#include <DXSampleHelper.h>
#include "../Application/WinAPI/WinApplication.h"
#include "../Application/WinAPI/WinWindow.h"
#include "../Renderer/Renderer1.h"
#include "../Renderer/RHI/RHIContext.h"

FluxEngine::FluxEngine()
{

}

FluxEngine::~FluxEngine()
{

}

void FluxEngine::Init()
{
    auto& rhiContext = RHIContext::GetInstance();

    AdapterCreateDesc adapterCreateDesc;
    adapterCreateDesc.useHighPerformanceAdapter = true;
    adapterCreateDesc.useWarpDevice = false;
    DeviceCreateDesc deviceCreateDesc;

    rhiContext.Init(ERHIRenderingAPI::D3D12, adapterCreateDesc, deviceCreateDesc);

    auto& renderer = Renderer1::GetInstance();
    renderer.Init();
    renderer.LoadPipeline();
}

void FluxEngine::Update()
{
}

void FluxEngine::Render()
{
    auto& renderer = Renderer1::GetInstance();
    renderer.Render();
}

void FluxEngine::Destroy()
{
    auto& renderer = Renderer1::GetInstance();
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    renderer.WaitForGpu();

    auto& rhiContext = RHIContext::GetInstance();
    rhiContext.Destroy();
}

void FluxEngine::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    for (int i = 1; i < argc; ++i)
    {
        if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
            _wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
        {
            WinApplication::SetTitle(WinApplication::GetTitle() + L" (WARP)");
        }
    }
}
