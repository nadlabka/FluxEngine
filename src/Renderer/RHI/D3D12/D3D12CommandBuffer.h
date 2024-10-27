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

		void BindRenderPipeline(std::shared_ptr<IRenderPipeline> renderPipeline);

		void SubmitToQueue(std::shared_ptr<ICommandQueue> commandQueue);

		//includes implicict commands such as dynamic states binds and resource states transitions
		void BeginRecording();
		void EndRecording();

		void SetViewport(const ViewportInfo& viewportInfo);
		void SetScissors(const ScissorsRect& rect);
		void SetBlendConstants(const std::array<float, 4> constantsValues);

		void DrawInstanced(const InstancedDrawInfo& instancedDrawInfo);
		void DrawIndexedInstanced(const IndexedInstancedDrawInfo& indexedInstancedDrawInfo);

		std::shared_ptr<D3D12RenderPipeline> m_currentRenderPipeline;

		RscPtr<ID3D12CommandAllocator> m_commandAllocator;
		RscPtr<ID3D12GraphicsCommandList> m_commandList;

		uint64_t m_fenceValue;
		HANDLE m_fenceEvent;
	};
}