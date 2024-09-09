#include <stdafx.h>
#include "D3D12Device.h"
#include "D3D12CommandQueue.h"

RHI::D3D12Device::D3D12Device()
{

}

RHI::D3D12Device::~D3D12Device()
{

}

std::shared_ptr<RHI::ICommandQueue> RHI::D3D12Device::CreateCommandQueue(QueueType queueType) const
{
    RscPtr<ID3D12CommandQueue> commandQueue;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    switch (queueType)
    {
    case QueueType::eQueueType_AllCommands:
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        break;
    case QueueType::eQueueType_CopyOnly:
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        break;
    case QueueType::eQueueType_ComputeOnly:
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        break;
    }

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

    return std::make_shared<D3D12CommandQueue>(commandQueue);
}
