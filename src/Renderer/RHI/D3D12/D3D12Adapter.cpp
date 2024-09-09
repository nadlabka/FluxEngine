#include <stdafx.h>
#include "D3D12Adapter.h"
#include "D3D12Device.h"

RHI::D3D12Adapter::D3D12Adapter()
{
}

RHI::D3D12Adapter::~D3D12Adapter()
{
}

std::shared_ptr<RHI::IDevice> RHI::D3D12Adapter::CreateDevice(RHI::DeviceCreateDesc deviceDesc) const
{
    RscPtr<ID3D12Device> device;
    ThrowIfFailed(D3D12CreateDevice(
        m_adapter.ptr(),
        D3D_FEATURE_LEVEL_12_0,
        IID_PPV_ARGS(&device)
    ));

    return std::make_shared<RHI::D3D12Device>(device);
}
