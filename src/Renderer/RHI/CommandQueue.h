#pragma once
#include <stdafx.h>

namespace RHI
{
	enum class QueueType 
	{
		AllCommands, CopyOnly, ComputeOnly
	};

	struct ICommandQueue
	{
		virtual ~ICommandQueue() {};

		virtual void WaitUntilCompleted() = 0;
	};
}
