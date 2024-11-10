#include "stdafx.h"
#include "D3D12Buffer.h"
#include "../RHIContext.h"
#include "Managers/DescriptorsHeapsManager.h"
#include "D3D12Device.h"

RHI::D3D12Buffer::D3D12Buffer(uint32_t elementsNum, uint32_t elementStride, RscPtr<D3D12MA::Allocation> allocation, D3D12_RESOURCE_STATES targetResourceState)
	: m_elementsNum(elementsNum), m_elementStride(elementStride), m_allocation(allocation), D3D12StatefulResource(targetResourceState)
{
}

RHI::D3D12Buffer::~D3D12Buffer()
{
    auto& descHeapsMgr = DescriptorsHeapsManager::GetInstance();
    auto cbv_srv_uav_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    if (m_SRVDescriptorIndex != D3D12DescriptorHeap::INDEX_INVALID)
    {
        cbv_srv_uav_heap->EraseIndex(m_SRVDescriptorIndex);
    }
    if (m_UAVDescriptorIndex != D3D12DescriptorHeap::INDEX_INVALID)
    {
        cbv_srv_uav_heap->EraseIndex(m_UAVDescriptorIndex);
    }
    if (m_CBVDescriptorIndex != D3D12DescriptorHeap::INDEX_INVALID)
    {
        cbv_srv_uav_heap->EraseIndex(m_CBVDescriptorIndex);
    }
}

void RHI::D3D12Buffer::AllocateDescriptorsInHeaps(const BufferDescription& desc)
{
    auto& rhiContext = RHIContext::GetInstance();
    auto d3d12device = std::static_pointer_cast<D3D12Device>(rhiContext.GetDevice());

    auto& descHeapsMgr = DescriptorsHeapsManager::GetInstance();
    auto cbv_srv_uav_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    ID3D12Resource* resourcePtr = m_buffer.ptr() ? m_buffer.ptr() : m_allocation->GetResource();
    
    if (desc.access == BufferAccess::Readback || desc.access == BufferAccess::Upload)
    {
        return;
    }

    if (desc.usage & BufferUsage::StorageBuffer)
    {
        //create UAV
        uint32_t uavIDX = cbv_srv_uav_heap->AllocateIndex();

        uint32_t uavIndex = cbv_srv_uav_heap->AllocateIndex();
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = 
        {
            .Format = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
            .Buffer =
            {
                .NumElements = desc.elementsNum,
                .StructureByteStride = desc.elementStride,
                .CounterOffsetInBytes = 0,
                .Flags = D3D12_BUFFER_UAV_FLAG_NONE
            }
        };

        auto handle = cbv_srv_uav_heap->GetCpuHandle(uavIndex);
        d3d12device->m_device->CreateUnorderedAccessView(resourcePtr, nullptr, &uavDesc, handle);
        m_UAVDescriptorIndex = uavIndex;
    }
    if (desc.usage & BufferUsage::DataReadBuffer)
    {
        //create SRV
        uint32_t srvIndex = cbv_srv_uav_heap->AllocateIndex();
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
        {
            .Format = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Buffer =
            {
                .FirstElement = 0,
                .NumElements = desc.elementsNum,
                .StructureByteStride = desc.elementStride,
                .Flags = D3D12_BUFFER_SRV_FLAG_NONE
            }
        };

        uint32_t srvIDX = cbv_srv_uav_heap->AllocateIndex();
        auto handle = cbv_srv_uav_heap->GetCpuHandle(srvIDX);
        d3d12device->m_device->CreateShaderResourceView(resourcePtr, &srvDesc, handle);
        m_SRVDescriptorIndex = srvIDX;
    }
    if (desc.usage & BufferUsage::UniformBuffer)
    {
        uint32_t cbvIndex = cbv_srv_uav_heap->AllocateIndex();
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = 
        {
            .BufferLocation = resourcePtr->GetGPUVirtualAddress(),
            .SizeInBytes = (desc.unstructuredSize + 255) & ~255
        };

        auto handle = cbv_srv_uav_heap->GetCpuHandle(cbvIndex);
        d3d12device->m_device->CreateConstantBufferView(&cbvDesc, handle);
        m_CBVDescriptorIndex = cbvIndex;
    }
}

void RHI::D3D12Buffer::UploadData(void* srcData, const BufferRegionCopyDescription& regionCopyDesc)
{
    ID3D12Resource* resourcePtr = m_buffer.ptr() ? m_buffer.ptr() : m_allocation->GetResource();

    void* destData = nullptr;

    D3D12_RANGE readRange = { 0, 0 }; // No read access
    resourcePtr->Map(0, &readRange, &destData);

    memcpy(static_cast<uint8_t*>(destData) + regionCopyDesc.destOffset,
        static_cast<const uint8_t*>(srcData) + regionCopyDesc.srcOffset,
        regionCopyDesc.width);

    D3D12_RANGE writeRange = { regionCopyDesc.destOffset, regionCopyDesc.destOffset + regionCopyDesc.width };
    resourcePtr->Unmap(0, &writeRange);
}

D3D12_HEAP_TYPE RHI::ConvertBufferAccessToD3D12HeapType(BufferAccess memoryVisibility)
{
    switch (memoryVisibility)
    {
    case BufferAccess::DefaultPrivate:
        return D3D12_HEAP_TYPE_DEFAULT;
    case BufferAccess::Upload:
        return D3D12_HEAP_TYPE_UPLOAD;
    case BufferAccess::Readback:
        return D3D12_HEAP_TYPE_READBACK;
    default:
        return D3D12_HEAP_TYPE_CUSTOM;
    }
}