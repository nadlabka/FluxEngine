#include <stdafx.h>
#include "FluxEngine.h"
#include <DXSampleHelper.h>
#include "../Application/WinAPI/WinApplication.h"
#include "../Application/WinAPI/WinWindow.h"
#include "../Renderer/Renderer1.h"
#include "../Renderer/RHI/RHIContext.h"

Core::FluxEngine::FluxEngine()
{

}

Core::FluxEngine::~FluxEngine()
{

}

void Core::FluxEngine::Init()
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

void Core::FluxEngine::Update()
{
}

void Core::FluxEngine::Render()
{
    auto& renderer = Renderer1::GetInstance();
    renderer.Render();
}

void Core::FluxEngine::Destroy()
{
    auto& renderer = Renderer1::GetInstance();
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    renderer.WaitForGpu();
    
    renderer.Destroy();

    auto& rhiContext = RHIContext::GetInstance();
    rhiContext.Destroy();
}

void Core::FluxEngine::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    for (int i = 1; i < argc; ++i)
    {
        if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
            _wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
        {
            Application::WinApplication::SetTitle(Application::WinApplication::GetTitle() + L" (WARP)");
        }
    }
}
