#pragma once
#include "../D3D12DescriptorHeap.h"
#include <memory>

namespace RHI
{
    class DescriptorHeapManager
    {
    public:
        static DescriptorHeapManager& GetInstance()
        {
            static DescriptorHeapManager instance;
            return instance;
        }

        DescriptorHeapManager(const DescriptorHeapManager& arg) = delete;
        DescriptorHeapManager& operator=(const DescriptorHeapManager& arg) = delete;

        void InitHeaps();

    private:
        DescriptorHeapManager() {}

        std::shared_ptr<D3D12DescriptorHeap> m_uav_srv_cbv_heaps;
        std::shared_ptr<D3D12DescriptorHeap> m_rtv_heaps;
        std::shared_ptr<D3D12DescriptorHeap> m_dsv_heaps;
        std::shared_ptr<D3D12DescriptorHeap> m_sampler_heaps;
    };
}