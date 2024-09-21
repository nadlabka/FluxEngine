#pragma once
#include <DescriptorHeap.h>
#include <DataStructures/IndicesRangeSolidVector.h>

namespace RHI
{
	struct D3D12DescriptorHeap : public IndicesRangeSolidVector, public DirectX::DescriptorHeap
	{
		D3D12DescriptorHeap() = default;
		D3D12DescriptorHeap(RscPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_DESC desc);
	};
}