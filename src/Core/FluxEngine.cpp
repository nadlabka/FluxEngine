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

void FluxEngine::OnInit()
{
    auto& renderer = Renderer::GetInstance();
    renderer.Init();
    renderer.LoadPipeline();
    renderer.LoadAssets();
}

void FluxEngine::OnUpdate()
{
}

void FluxEngine::OnRender()
{
    auto& renderer = Renderer::GetInstance();
    renderer.Render();
}

void FluxEngine::OnDestroy()
{
    auto& renderer = Renderer::GetInstance();
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    renderer.WaitForPreviousFrame();

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
