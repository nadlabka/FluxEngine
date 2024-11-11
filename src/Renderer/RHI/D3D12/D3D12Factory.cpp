#include <stdafx.h>
#include "D3D12Factory.h"
#include "D3D12Adapter.h"
#include <DXSampleHelper.h>
#include "D3D12Swapchain.h"
#include "D3D12Surface.h"
#include "D3D12CommandQueue.h"

RHI::D3D12Factory::D3D12Factory()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        RscPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }

        RscPtr<ID3D12InfoQueue> infoQueue;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&infoQueue)))) {
            // Configure the warning severity levels
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

            D3D12_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.pIDList = nullptr;
            filter.DenyList.NumIDs = 0;
            infoQueue->AddStorageFilterEntries(&filter);

            std::cout << "D3D12 Debug Info Queue Configured." << std::endl;
        }
    }
#endif

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
}

RHI::D3D12Factory::~D3D12Factory()
{
}

std::shared_ptr<RHI::IAdapter> RHI::D3D12Factory::CreateAdapter(RHI::AdapterCreateDesc adapterDesc) const
{
    RscPtr<IDXGIAdapter1> adapter;
    if(adapterDesc.useWarpDevice)
    {
        ThrowIfFailed(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)));
    }
    else
    {
        RscPtr<IDXGIFactory6> factory6;
        if (SUCCEEDED(m_factory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    adapterDesc.useHighPerformanceAdapter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                    ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.ptr(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        if (adapter.ptr() == nullptr)
        {
            for (UINT adapterIndex = 0; SUCCEEDED(m_factory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.ptr(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }
    }

    return std::make_shared<D3D12Adapter>(adapter);
}

std::shared_ptr<RHI::ISwapchain> RHI::D3D12Factory::CreateSwapchain(std::shared_ptr<ISurface> surface, std::shared_ptr<ICommandQueue> commandQueue, uint32_t framesCount) const
{
    RscPtr<IDXGISwapChain1> swapchain1;
    RscPtr<IDXGISwapChain3> swapchain3;

    auto d3d12Surface = static_pointer_cast<D3D12Surface>(surface);
    auto d3d12CommandQueue = static_pointer_cast<D3D12CommandQueue>(commandQueue);

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = framesCount;
    swapChainDesc.Width = d3d12Surface->m_width;
    swapChainDesc.Height = d3d12Surface->m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ThrowIfFailed(m_factory->CreateSwapChainForHwnd(
        d3d12CommandQueue->m_commandQueue.ptr(),        // Swap chain needs the queue so that it can force a flush on it.
        (HWND)d3d12Surface->m_windowHandle,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapchain1
    ));

    ThrowIfFailed(swapchain1->QueryInterface(IID_PPV_ARGS(&swapchain3)));

    auto resultSwapchain = std::make_shared<D3D12Swapchain>(swapchain3, framesCount);
    resultSwapchain->UpdateDescriptors();
    return resultSwapchain;
}