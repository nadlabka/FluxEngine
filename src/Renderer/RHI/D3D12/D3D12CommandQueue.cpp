#include "stdafx.h"
#include "D3D12CommandQueue.h"
#include "D3D12CommandBuffer.h"


RHI::D3D12CommandQueue::D3D12CommandQueue(RscPtr<ID3D12CommandQueue> commandQueue, RscPtr<ID3D12Fence> fence, uint64_t fenceValue)
    : m_commandQueue(commandQueue), m_fence(fence), m_fenceValue(fenceValue)
{
    m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

RHI::D3D12CommandQueue::~D3D12CommandQueue()
{
    CloseHandle(m_fenceEvent);
}

void RHI::D3D12CommandQueue::WaitUntilCompleted()
{
	WaitForFenceValue(SignalFence(), m_fenceEvent);
}

uint64_t RHI::D3D12CommandQueue::SignalFence()
{
    uint64_t fenceValue = ++m_fenceValue;
    m_commandQueue->Signal(m_fence.ptr(), fenceValue);
    return fenceValue;
}

bool RHI::D3D12CommandQueue::IsFenceValueCompleted(uint64_t value)
{
    return m_fence->GetCompletedValue() >= value;
}

void RHI::D3D12CommandQueue::WaitForFenceValue(uint64_t value, HANDLE fenceEvent)
{
    if (!IsFenceValueCompleted(value))
    {
        m_fence->SetEventOnCompletion(value, fenceEvent);
        ::WaitForSingleObject(fenceEvent, DWORD_MAX);
    }
}

D3D12_COMMAND_LIST_TYPE RHI::ConvertQueueTypeToCommandListType(QueueType queueType)
{
    switch (queueType)
    {
    case QueueType::AllCommands: return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case QueueType::CopyOnly: return D3D12_COMMAND_LIST_TYPE_COPY;
    case QueueType::ComputeOnly: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    default: throw std::runtime_error("No such command queue is available for conversion ToCommandListType");
    }
}
