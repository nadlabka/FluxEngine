#pragma once
#include "../Buffer.h"
#include "D3D12StatefulResource.h"
#include "D3D12DescriptorHeap.h"
#include <cstdint>
#include <DebugMacros.h>

namespace RHI
{
    D3D12_HEAP_TYPE ConvertBufferVisibilityToD3D12HeapType(BufferVisibility memoryVisibility)
    {
        switch (memoryVisibility)
        {
        case BufferVisibility::DefaultPrivate:
            return D3D12_HEAP_TYPE_DEFAULT;
        case BufferVisibility::Upload:
            return D3D12_HEAP_TYPE_UPLOAD;
        case BufferVisibility::Readback:
            return D3D12_HEAP_TYPE_READBACK;
        default:
            return D3D12_HEAP_TYPE_CUSTOM;
        }
    }

    D3D12_RESOURCE_STATES GetD3D12ResourceStateFromDescription(const BufferDescription& desc)
    {
        D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;

        switch (desc.visibility)
        {
        case BufferVisibility::Upload:
            return D3D12_RESOURCE_STATE_GENERIC_READ;
        case BufferVisibility::Readback:
            return D3D12_RESOURCE_STATE_COPY_DEST;
        }

        if (desc.usage & BufferUsage::UniformBuffer)
        {
            state |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        }
        if (desc.usage & BufferUsage::StorageBuffer)
        {
            state |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        }
        if (desc.usage & BufferUsage::IndexBuffer)
        {
            state |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        }
        if (desc.usage & BufferUsage::VertexBuffer)
        {
            state |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        }
        if (desc.usage & BufferUsage::IndirectBuffer)
        {
            state |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        }
        if (desc.flags.isCopySrc)
        {
            state |= D3D12_RESOURCE_STATE_COPY_SOURCE;
        }
        if (desc.flags.isCopyDst)
        {
            ASSERT(!desc.flags.isCopySrc, "Buffer can't be copy SRC and DEST at the same time");
            state |= D3D12_RESOURCE_STATE_COPY_DEST;
        }

        return state;
    }

	struct D3D12Buffer : public IBuffer, public D3D12StatefulResource
	{
        D3D12Buffer(uint32_t elementsNum, uint32_t elementStride, RscPtr<D3D12MA::Allocation> allocation, D3D12_RESOURCE_STATES resourceState);

        void AllocateDescriptorsInHeaps(const BufferDescription& desc);

        uint32_t m_UAVDescriptorsIndex = D3D12DescriptorHeap::INDEX_INVALID;
        uint32_t m_SRVDescriptorsIndex = D3D12DescriptorHeap::INDEX_INVALID;
        uint32_t m_CBVDescriptorsIndex = D3D12DescriptorHeap::INDEX_INVALID;

        uint32_t m_elementsNum;
        uint32_t m_elementStride;

		RscPtr<ID3D12Resource> m_buffer;
		RscPtr<D3D12MA::Allocation> m_allocation;
	};
}