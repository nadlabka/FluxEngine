#pragma once
#include "CommandQueue.h"
#include "CommandBuffer.h"
#include "Texture.h"

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
	};
}
