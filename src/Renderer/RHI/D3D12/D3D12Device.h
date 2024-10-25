#pragma once
#include <stdafx.h>
#include "../Device.h"
#include "../Swapchain.h"
#include "../Surface.h"
#include "../PipelineCommon.h"

namespace RHI
{
	struct D3D12Device : public IDevice
	{
		D3D12Device();
		D3D12Device(RscPtr<ID3D12Device> device) :  m_device(device) {}
		~D3D12Device();

		std::shared_ptr<ICommandQueue> CreateCommandQueue(QueueType queueType) const;
		std::shared_ptr<ICommandBuffer> CreateCommandBuffer(QueueType bufferSubmitQueueType) const;
		std::shared_ptr<IRenderPass> CreateRenderPass(const RenderPassDesc& renderPassDesc) const;
		std::shared_ptr<IPipelineLayout> CreatePipelineLayout(const PipelineLayoutDescription& pipelineLayoutDesc) const;
		std::shared_ptr<IRenderPipeline> CreateRenderPipeline(const RenderPipelineDescription& renderPipelineDesc) const;

		RscPtr<ID3D12Device> m_device;
	};
}