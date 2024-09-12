#include "stdafx.h"
#include "D3D12Allocator.h"
#include "D3D12Device.h"
#include "D3D12Adapter.h"

RHI::D3D12Allocator::D3D12Allocator(std::shared_ptr<IDevice> device, std::shared_ptr<IAdapter> adapter)
{
	auto d3d12Device = std::dynamic_pointer_cast<D3D12Device>(device);
	auto d3d12Adapter = std::dynamic_pointer_cast<D3D12Adapter>(adapter);

	D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
	allocatorDesc.pDevice = d3d12Device->m_device.ptr();
	allocatorDesc.pAdapter = d3d12Adapter->m_adapter.ptr();
	// These flags are optional but recommended.
	allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED |
		D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;

	ThrowIfFailed(D3D12MA::CreateAllocator(&allocatorDesc, &m_allocator));
}
