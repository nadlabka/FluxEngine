#include "stdafx.h"
#include "DescriptorsHeapsManager.h"
#include "../../RHIContext.h"
#include "../D3D12Device.h"

void RHI::DescriptorsHeapsManager::InitHeaps()
{
    auto& currentAPI = RHIContext::GetInstance();
    auto device = currentAPI.GetDevice();
    
    auto d3d12device = std::static_pointer_cast<D3D12Device>(device);

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 4096;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;

        m_uav_srv_cbv_heap = std::make_shared<D3D12DescriptorHeap>(d3d12device->m_device, desc);
        m_uav_srv_cbv_heap->SetMaxIndexLimit(desc.NumDescriptors);
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = 128;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;

        m_rtv_heap = std::make_shared<D3D12DescriptorHeap>(d3d12device->m_device, desc);
        m_rtv_heap->SetMaxIndexLimit(desc.NumDescriptors);
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        desc.NumDescriptors = 128;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;

        m_dsv_heap = std::make_shared<D3D12DescriptorHeap>(d3d12device->m_device, desc);
        m_dsv_heap->SetMaxIndexLimit(desc.NumDescriptors);
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        desc.NumDescriptors = 256;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;

        m_sampler_heap = std::make_shared<D3D12DescriptorHeap>(d3d12device->m_device, desc);
        m_sampler_heap->SetMaxIndexLimit(desc.NumDescriptors);
    }
}

void RHI::DescriptorsHeapsManager::Destroy()
{
    m_uav_srv_cbv_heap.reset();
    m_rtv_heap.reset();
    m_dsv_heap.reset();
    m_sampler_heap.reset();
}

std::shared_ptr<RHI::D3D12DescriptorHeap> RHI::DescriptorsHeapsManager::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
    switch (heapType)
    {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        return m_uav_srv_cbv_heap;

    case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
        return m_rtv_heap;

    case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
        return m_dsv_heap;

    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
        return m_sampler_heap;

    default:
        throw std::invalid_argument("Unsupported heap type requested.");
    }

}
