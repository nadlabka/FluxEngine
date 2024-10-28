#pragma once
#include "../Buffer.h"
#include "D3D12StatefulResource.h"
#include "D3D12DescriptorHeap.h"
#include <cstdint>
#include <DebugMacros.h>

class D3D12MA::Allocation;

namespace RHI
{
    D3D12_HEAP_TYPE ConvertBufferAccessToD3D12HeapType(BufferAccess memoryVisibility);
    D3D12_RESOURCE_STATES GetD3D12ResourceStateFromDescription(const BufferDescription& desc);

	struct D3D12Buffer : public IBuffer, public D3D12StatefulResource
	{
        D3D12Buffer(uint32_t elementsNum, uint32_t elementStride, RscPtr<D3D12MA::Allocation> allocation, bool requiredCopyStateToInit, D3D12_RESOURCE_STATES targetResourceState);
        ~D3D12Buffer();

        void AllocateDescriptorsInHeaps(const BufferDescription& desc);

        void UploadData(void* data, uint64_t size);

        // we need this to avoid unnecessary transition to resource state COPY_DEST immdiately after buffer creation
        bool m_isInitialised = false;

        uint32_t m_UAVDescriptorIndex = D3D12DescriptorHeap::INDEX_INVALID;
        uint32_t m_SRVDescriptorIndex = D3D12DescriptorHeap::INDEX_INVALID;
        uint32_t m_CBVDescriptorIndex = D3D12DescriptorHeap::INDEX_INVALID;

        uint32_t m_elementsNum;
        uint32_t m_elementStride;

        //use recource OR allocation
		RscPtr<ID3D12Resource> m_buffer;
		RscPtr<D3D12MA::Allocation> m_allocation;
	};
}