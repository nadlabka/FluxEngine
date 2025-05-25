#pragma once
#include "CommandQueue.h"
#include "Texture.h"
#include "RenderPass.h"
#include "PipelineCommon.h"

namespace RHI
{
	struct InstancedDrawInfo
	{
		uint32_t verticesPerInstanceNum;
		uint32_t instancesNum = 1;
		uint32_t startVertex = 0;
		uint32_t firstInstance = 0;
	};

	struct IndexedInstancedDrawInfo
	{
		uint32_t indicesPerInstanceNum;
		uint32_t instancesNum = 1;
		uint32_t firstIndex = 0;
		uint32_t firstInstance = 0;
		uint32_t startVertex = 0;
	};

	struct ICommandBuffer
	{
		virtual void BindRenderPipeline(std::shared_ptr<IRenderPipeline> renderPipeline) = 0;
		virtual std::shared_ptr<IRenderPipeline> GetCurrentRenderPipeline() = 0;

		virtual void BindComputePipeline(std::shared_ptr<IComputePipeline> computePipeline) = 0;
		virtual std::shared_ptr<IComputePipeline> GetCurrentComputePipeline() = 0;

		virtual void SubmitToQueue(std::shared_ptr<ICommandQueue> commandQueue) = 0;

		virtual void BeginRecording(std::shared_ptr<ICommandQueue> commandQueue) = 0;
		virtual void EndRecording() = 0;

		virtual void BindPipelineResources() = 0;
		virtual void BindRenderTargets(bool isToClear) = 0;
		virtual void FinishRenderTargets() = 0;

		virtual void ForceWaitUntilFinished(std::shared_ptr<ICommandQueue> commandQueue) = 0;

		virtual void SetViewport(const ViewportInfo& viewportInfo) = 0;
		virtual void SetScissors(const ScissorsRect& scissorsRect) = 0;
		virtual void SetBlendConstants(const std::array<float, 4> constantsValues) = 0;
		virtual void SetPrimitiveTopology(PrimitiveTopology primitiveTopology) = 0;

		//slot has to match binding index in InputAssemblerLayoutDescription
		virtual void SetVertexBuffer(std::shared_ptr<IBuffer> buffer, uint32_t slot, const BufferRegionDescription& bufferBindDesc) = 0;
		virtual void SetIndexBuffer(std::shared_ptr<IBuffer> buffer, const BufferRegionDescription& bufferBindDesc) = 0;

		virtual void DrawInstanced(const InstancedDrawInfo& instancedDrawInfo) = 0;
		virtual void DrawIndexedInstanced(const IndexedInstancedDrawInfo& indexedInstancedDrawInfo) = 0;

		virtual void CopyDataBetweenBuffers(std::shared_ptr<IBuffer> fromBuffer, std::shared_ptr<IBuffer> toBuffer, const BufferRegionCopyDescription& regionCopyDesc) = 0;
		virtual void CopyDataFromBufferToTexture(std::shared_ptr<IBuffer> fromBuffer, std::shared_ptr<ITexture> toTexture, const TextureRegionCopyDescription& regionCopyDesc) = 0;

		virtual ~ICommandBuffer() {}
	};
}