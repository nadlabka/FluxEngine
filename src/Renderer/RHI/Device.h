#pragma once
#include "CommandQueue.h"
#include "CommandBuffer.h"
#include "Texture.h"
#include "RenderPass.h"
#include "PipelineCommon.h"

namespace RHI
{
	struct DeviceCreateDesc
	{
	};

	struct IDevice
	{
		virtual ~IDevice() {}

		virtual std::shared_ptr<ICommandQueue> CreateCommandQueue(QueueType queueType) const = 0;
		virtual std::shared_ptr<ICommandBuffer> CreateCommandBuffer(QueueType bufferSubmitQueueType) const = 0;
		virtual std::shared_ptr<IRenderPass> CreateRenderPass(const RenderPassDesc& renderPassDesc) const = 0;
		virtual std::shared_ptr<IPipelineLayout> CreatePipelineLayout(const std::vector<PipelineStageDescription>& pipelineStages) const = 0;
		virtual std::shared_ptr<IRenderPipeline> CreateRenderPipeline(const RenderPipelineDescription& renderPipelineDesc) const = 0;
		virtual std::shared_ptr<IComputePipeline> CreateComputePipeline(const ComputePipelineDescription& computePipelineDesc) const = 0;
		virtual std::shared_ptr<ISampler> CreateSampler(const SamplerDescription& sampplerDesc) const = 0;
	};
}
