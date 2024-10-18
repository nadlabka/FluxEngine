#pragma once
#include "stdafx.h"
#include "../CommandBuffer.h"
#include "D3D12RenderPass.h"
#include "D3D12RenderPipeline.h"

namespace RHI
{
	struct D3D12CommandBuffer : ICommandBuffer
	{
		D3D12CommandBuffer() = default;
		D3D12CommandBuffer(RscPtr<ID3D12CommandAllocator> commandAllocator, RscPtr<ID3D12GraphicsCommandList> commandList);
		~D3D12CommandBuffer();

		void SubmitToQueue(std::shared_ptr<ICommandQueue> commandQueue);

		void BeginRecording();
		void EndRecording();

		void SetViewport(const ViewportInfo& viewportInfo);
		void SetScissors(const ScissorsRect& rect);

		void DrawInstanced(const InstancedDrawInfo& instancedDrawInfo);
		void DrawIndexedInstanced(const IndexedInstancedDrawInfo& indexedInstancedDrawInfo);

		std::shared_ptr<D3D12RenderPass> currentRenderPass;
		std::shared_ptr<D3D12RenderPipeline> currentRenderPipeline;

		RscPtr<ID3D12CommandAllocator> m_commandAllocator;
		RscPtr<ID3D12GraphicsCommandList> m_commandList;

		uint64_t m_fenceValue;
		HANDLE m_fenceEvent;
	};
}