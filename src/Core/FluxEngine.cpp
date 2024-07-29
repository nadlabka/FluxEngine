#include <stdafx.h>
#include "FluxEngine.h"
#include "../Utils/DXSampleHelper.h"
#include "../Application/WinAPI/WinApplication.h"
#include "../Application/WinAPI/WinWindow.h"
#include "../Renderer/Renderer.h"

FluxEngine::FluxEngine()
{

}

FluxEngine::~FluxEngine()
{

}

void FluxEngine::Init()
{
    auto& renderer = Renderer::GetInstance();
    renderer.Init();
    renderer.LoadPipeline();
    renderer.LoadAssets();
}

void FluxEngine::Update()
{
}

void FluxEngine::Render()
{
    auto& renderer = Renderer::GetInstance();
    renderer.Render();
}

void FluxEngine::Destroy()
{
    auto& renderer = Renderer::GetInstance();
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    renderer.WaitForGpu();

    CloseHandle(renderer.m_fenceEvent);
}

void FluxEngine::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    auto& renderer = Renderer::GetInstance();
    for (int i = 1; i < argc; ++i)
    {
        if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
            _wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
        {
            renderer.m_useWarpDevice = true;
            WinApplication::SetTitle(WinApplication::GetTitle() + L" (WARP)");
        }
    }
}
