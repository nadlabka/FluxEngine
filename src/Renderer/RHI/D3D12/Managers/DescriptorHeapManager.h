#pragma once
#include "../D3D12DescriptorHeap.h"
#include <memory>

namespace RHI
{
    struct ResourceDescriptorsIndices
    {
        constexpr static size_t invalid = ULLONG_MAX;

        size_t uav_index;
        size_t srv_index;
        size_t m_rtv_index;
        size_t m_dsv_index;
    };

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
        std::shared_ptr<D3D12DescriptorHeap> GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

    private:
        DescriptorHeapManager() {}

        std::shared_ptr<D3D12DescriptorHeap> m_uav_srv_cbv_heap;
        std::shared_ptr<D3D12DescriptorHeap> m_rtv_heap;
        std::shared_ptr<D3D12DescriptorHeap> m_dsv_heap;
        std::shared_ptr<D3D12DescriptorHeap> m_sampler_heap;
    };
}