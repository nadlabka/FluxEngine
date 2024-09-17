#include "stdafx.h"
#include "D3D12DescriptorHeap.h"
#include "../RHIContext.h"

RHI::D3D12DescriptorHeap::D3D12DescriptorHeap(RscPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_DESC desc) 
	: DirectX::DescriptorHeap(device.ptr(), &desc)
{
}
