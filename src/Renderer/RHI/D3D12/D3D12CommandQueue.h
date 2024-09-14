#pragma once
#include <stdafx.h>
#include "../CommandQueue.h"

namespace RHI
{
	struct D3D12CommandQueue : public ICommandQueue
	{
		D3D12CommandQueue();
		D3D12CommandQueue(RscPtr<ID3D12CommandQueue> commandQueue) : m_commandQueue(commandQueue) {}
		~D3D12CommandQueue();

		RscPtr<ID3D12CommandQueue> m_commandQueue;
	};
}