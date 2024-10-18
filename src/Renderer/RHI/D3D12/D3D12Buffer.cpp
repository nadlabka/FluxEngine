#include "stdafx.h"
#include "D3D12Buffer.h"

RHI::D3D12Buffer::D3D12Buffer(uint32_t elementsNum, uint32_t elementStride, RscPtr<D3D12MA::Allocation> allocation, D3D12_RESOURCE_STATES resourceState)
	: m_elementsNum(elementsNum), m_elementStride(elementStride), m_allocation(allocation), D3D12StatefulResource(resourceState)
{
}

void RHI::D3D12Buffer::AllocateDescriptorsInHeaps(const BufferDescription& desc)
{

}
