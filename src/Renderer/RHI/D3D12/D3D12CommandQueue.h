#pragma once
#include <stdafx.h>
#include "../CommandQueue.h"

namespace RHI
{
	D3D12_COMMAND_LIST_TYPE ConvertQueueTypeToCommandListType(QueueType queueType);

	struct D3D12CommandQueue : public ICommandQueue
	{
		D3D12CommandQueue() = default;
		D3D12CommandQueue(RscPtr<ID3D12CommandQueue> commandQueue, RscPtr<ID3D12Fence> fence, uint64_t fenceValue);
		~D3D12CommandQueue();

		void WaitUntilCompleted();
		
		uint64_t SignalFence();
		bool IsFenceValueCompleted(uint64_t value);
		void WaitForFenceValue(uint64_t value, HANDLE fenceEvent);

		RscPtr<ID3D12Fence> m_fence;
		HANDLE  m_fenceEvent;
		uint64_t m_fenceValue;

		RscPtr<ID3D12CommandQueue> m_commandQueue;
	};
}