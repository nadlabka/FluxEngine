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
		std::shared_ptr<IRenderPipeline> GetCurrentRenderPipeline();

		void SubmitToQueue(std::shared_ptr<ICommandQueue> commandQueue);

		//includes implicict commands such as pipeline dynamic states binds and resource states transitions
		void BeginRecording(std::shared_ptr<ICommandQueue> commandQueue);
		void EndRecording();

		void BindPipelineResources();
		void BindRenderTargets();
		void FinishRenderTargets();

		void ForceWaitUntilFinished(std::shared_ptr<ICommandQueue> commandQueue);

		void SetViewport(const ViewportInfo& viewportInfo);
		void SetScissors(const ScissorsRect& scissorsRect);
		void SetBlendConstants(const std::array<float, 4> constantsValues);
		void SetPrimitiveTopology(PrimitiveTopology primitiveTopology);

		void SetVertexBuffer(std::shared_ptr<IBuffer> buffer, uint32_t slot, const BufferRegionDescription& bufferBindDesc);
		void SetIndexBuffer(std::shared_ptr<IBuffer> buffer, const BufferRegionDescription& bufferBindDesc);

		void DrawInstanced(const InstancedDrawInfo& instancedDrawInfo);
		void DrawIndexedInstanced(const IndexedInstancedDrawInfo& indexedInstancedDrawInfo);

		void CopyDataBetweenBuffers(std::shared_ptr<IBuffer> fromBuffer, std::shared_ptr<IBuffer> toBuffer, const BufferRegionCopyDescription& regionCopyDesc);

		std::shared_ptr<D3D12RenderPipeline> m_currentRenderPipeline = {};

		RscPtr<ID3D12CommandAllocator> m_commandAllocator;
		RscPtr<ID3D12GraphicsCommandList> m_commandList;

		uint64_t m_fenceValue;
		HANDLE m_fenceEvent;
	};
}