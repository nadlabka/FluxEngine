#pragma once
#include <DescriptorHeap.h>
#include <DataStructures/IndicesRangeSolidVector.h>

namespace RHI
{
	struct D3D12DescriptorHeap : DirectX::DescriptorHeap
	{

		IndicesRangeSolidVector descriptorsIndicesRange;
	};
}