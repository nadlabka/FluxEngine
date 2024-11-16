#pragma once
#include "../D3D12DescriptorHeap.h"
#include <memory>

namespace RHI
{
    class DescriptorsHeapsManager
    {
    public:
        static DescriptorsHeapsManager& GetInstance()
        {
            static DescriptorsHeapsManager instance;
            return instance;
        }

        DescriptorsHeapsManager(const DescriptorsHeapsManager& arg) = delete;
        DescriptorsHeapsManager& operator=(const DescriptorsHeapsManager& arg) = delete;

        void InitHeaps();
        void Destroy();

        std::shared_ptr<D3D12DescriptorHeap> GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
    private:
        DescriptorsHeapsManager() {}

        std::shared_ptr<D3D12DescriptorHeap> m_uav_srv_cbv_heap;
        std::shared_ptr<D3D12DescriptorHeap> m_rtv_heap;
        std::shared_ptr<D3D12DescriptorHeap> m_dsv_heap;
        std::shared_ptr<D3D12DescriptorHeap> m_sampler_heap;
    };
}