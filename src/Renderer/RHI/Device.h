#pragma once
#include "CommandQueue.h"

namespace RHI
{
	struct DeviceCreateDesc
	{
	};

	struct IDevice
	{
		virtual ~IDevice() {}

		virtual std::shared_ptr<ICommandQueue> CreateCommandQueue(QueueType queueType) const = 0;
	};
}
