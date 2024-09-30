#include <stdafx.h>
#include "D3D12Device.h"
#include "D3D12CommandQueue.h"
#include "D3D12Texture.h"
#include "D3D12CommandBuffer.h"

RHI::D3D12Device::D3D12Device()
{

}

RHI::D3D12Device::~D3D12Device()
{

}

std::shared_ptr<RHI::ICommandQueue> RHI::D3D12Device::CreateCommandQueue(QueueType queueType) const
{
    RscPtr<ID3D12CommandQueue> commandQueue;
    RscPtr<ID3D12Fence> fence;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = ConverQueueTypeToCommandListType(queueType);

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
    ThrowIfFailed(m_device->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    return std::make_shared<D3D12CommandQueue>(commandQueue, fence, 0u);
}

std::shared_ptr<RHI::ICommandBuffer> RHI::D3D12Device::CreateCommandBuffer(QueueType bufferSubmitQueueType) const
{
    RscPtr<ID3D12CommandAllocator> commandAllocator;
    RscPtr<ID3D12GraphicsCommandList> commandList;

    auto commandListType = ConverQueueTypeToCommandListType(bufferSubmitQueueType);

    ThrowIfFailed(m_device->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&commandAllocator)));
    ThrowIfFailed(m_device->CreateCommandList(0, commandListType, commandAllocator.ptr(), nullptr, IID_PPV_ARGS(&commandList)));

    return std::make_shared<D3D12CommandBuffer>(commandAllocator, commandList);
}
