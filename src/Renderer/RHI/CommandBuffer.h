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
		virtual void SubmitToQueue(std::shared_ptr<ICommandQueue> commandQueue) = 0;

		virtual void BindRenderPipeline(std::shared_ptr<IRenderPipeline> renderPipeline) = 0;

		virtual void BeginRecording() = 0;
		virtual void EndRecording() = 0;

		virtual void SetDescriptorStoragesForBindless() = 0;

		virtual void SetViewport(const ViewportInfo& viewportInfo) = 0;
		virtual void SetScissors(const ScissorsRect& rect) = 0;
		virtual void SetBlendConstants(const std::array<float, 4> constantsValues) = 0;

		virtual void DrawInstanced(const InstancedDrawInfo& instancedDrawInfo) = 0;
		virtual void DrawIndexedInstanced(const IndexedInstancedDrawInfo& indexedInstancedDrawInfo) = 0;

		virtual void TransitionTextureLayout(TextureLayout newLayout) = 0;

		virtual ~ICommandBuffer() {}
	};
}