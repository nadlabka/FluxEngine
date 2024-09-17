#include <stdafx.h>
#include <stdexcept>
#include "RHIContext.h"
#include "D3D12/D3D12Factory.h"
#include "D3D12/D3D12Allocator.h"
#include "D3D12/Managers/DescriptorHeapManager.h"

void RHI::RHIContext::Init(ERHIRenderingAPI api, const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc)
{
	if(api == ERHIRenderingAPI::eAPI_D3D12)
	{
		InitD3D12(adapterDesc, deviceDesc);
		currentRHI = ERHIRenderingAPI::eAPI_D3D12;
	}
	else if(api == ERHIRenderingAPI::eAPI_Vulkan)
	{
		throw std::runtime_error("Vulkan API support is not implemented");
		InitVulkan(adapterDesc, deviceDesc);
		currentRHI = ERHIRenderingAPI::eAPI_Vulkan;
	}
}

void RHI::RHIContext::InitD3D12(const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc)
{
	m_factory = std::make_shared<D3D12Factory>();

	m_adapter = m_factory->CreateAdapter(adapterDesc);

	m_device = m_adapter->CreateDevice(deviceDesc);

	m_allocator = std::make_shared<D3D12Allocator>(m_device, m_adapter);

	DescriptorHeapManager::GetInstance().InitHeaps();
}

void RHI::RHIContext::InitVulkan(const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc)
{

}
