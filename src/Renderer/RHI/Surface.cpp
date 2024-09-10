#include "stdafx.h"
#include "Surface.h"
#include "RHIContext.h"
#include "D3D12/D3D12Surface.h"

std::shared_ptr<RHI::ISurface> RHI::ISurface::CreateSurfaceFromWindow(const WinWindow& window)
{
    auto currentAPI = RHIContext::GetInstance().GetCurrentAPI();
    switch (currentAPI)
    {
    case ERHIRenderingAPI::eAPI_D3D12:
        return std::make_shared<D3D12Surface>(window.GetHwnd(), window.GetWidth(), window.GetHeight());
        break;
    case ERHIRenderingAPI::eAPI_Vulkan:
        throw std::runtime_error("Vulkan API support is not implemented");
        break;
    }
}
