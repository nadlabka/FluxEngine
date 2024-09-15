#pragma once
#include <DescriptorHeap.h>
#include <DataStructures/IndicesRangeSolidVector.h>

namespace RHI
{
	struct D3D12DescriptorHeap : public IndicesRangeSolidVector, DirectX::DescriptorHeap
	{
		D3D12DescriptorHeap() = default;
	};
}