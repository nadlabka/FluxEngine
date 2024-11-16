#include <stdafx.h>
#include <stdexcept>
#include "RHIContext.h"
#include "D3D12/D3D12Factory.h"
#include "D3D12/D3D12Allocator.h"
#include "D3D12/Managers/DescriptorsHeapsManager.h"
#include "D3D12/D3D12Device.h"
#include "D3D12/D3D12Adapter.h"

void RHI::RHIContext::Init(ERHIRenderingAPI api, const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc)
{
	if(api == ERHIRenderingAPI::D3D12)
	{
		InitD3D12(adapterDesc, deviceDesc);
		currentAPI = ERHIRenderingAPI::D3D12;
	}
	else if(api == ERHIRenderingAPI::Vulkan)
	{
		throw std::runtime_error("Vulkan API support is not implemented");
		InitVulkan(adapterDesc, deviceDesc);
		currentAPI = ERHIRenderingAPI::Vulkan;
	}
}

void RHI::RHIContext::Destroy()
{
	DescriptorsHeapsManager::GetInstance().Destroy();

	m_allocator.reset();

	if (currentAPI == ERHIRenderingAPI::D3D12)
	{
#ifdef _DEBUG
		auto d3d12Device = std::static_pointer_cast<D3D12Device>(m_device);
		RscPtr<ID3D12DebugDevice> debugDevice;
		ThrowIfFailed(d3d12Device->m_device->QueryInterface(IID_PPV_ARGS(&debugDevice))); // adds 1 to device ref counter
		debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL); // should be 2 device ref and nothing else here
#endif
	}
	else if (currentAPI == ERHIRenderingAPI::Vulkan)
	{

	}

	m_device.reset();
	m_adapter.reset();
	m_factory.reset();
}

void RHI::RHIContext::InitD3D12(const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc)
{
	m_factory = std::make_shared<D3D12Factory>();

	m_adapter = m_factory->CreateAdapter(adapterDesc);

	m_device = m_adapter->CreateDevice(deviceDesc);

	m_allocator = std::make_shared<D3D12Allocator>(m_device, m_adapter);

	DescriptorsHeapsManager::GetInstance().InitHeaps();
}

void RHI::RHIContext::InitVulkan(const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc)
{

}
