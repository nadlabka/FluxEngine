#pragma once
#include "stdafx.h"
#include "../CommandBuffer.h"
#include "D3D12RenderPass.h"
#include "D3D12RenderPipeline.h"
#include "D3D12ComputePipeline.h"

namespace RHI
{
	struct D3D12CommandBuffer : ICommandBuffer
	{
		D3D12CommandBuffer() = default;
		D3D12CommandBuffer(RscPtr<ID3D12CommandAllocator> commandAllocator, RscPtr<ID3D12GraphicsCommandList> commandList);
		~D3D12CommandBuffer();

		void BindRenderPipeline(std::shared_ptr<IRenderPipeline> renderPipeline);
		std::shared_ptr<IRenderPipeline> GetCurrentRenderPipeline();

		void BindComputePipeline(std::shared_ptr<IComputePipeline> computePipeline);
		std::shared_ptr<IComputePipeline> GetCurrentComputePipeline();

		void SubmitToQueue(std::shared_ptr<ICommandQueue> commandQueue);

		//includes implicict commands such as pipeline dynamic states binds and resource states transitions
		void BeginRecording(std::shared_ptr<ICommandQueue> commandQueue);
		void EndRecording();

		void BindPipelineResources();
		void BindRenderTargets(bool isToClear);
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
		void CopyDataFromBufferToTexture(std::shared_ptr<IBuffer> fromBuffer, std::shared_ptr<ITexture> toTexture, const TextureRegionCopyDescription& regionCopyDesc);

		std::shared_ptr<D3D12RenderPipeline> m_currentRenderPipeline = {};
		std::shared_ptr<D3D12ComputePipeline> m_currentComputePipeline = {};

		RscPtr<ID3D12CommandAllocator> m_commandAllocator;
		RscPtr<ID3D12GraphicsCommandList> m_commandList;

		uint64_t m_fenceValue;
		HANDLE m_fenceEvent;
	};
}