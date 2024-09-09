#pragma once
#include <stdafx.h>

namespace RHI
{
	enum class QueueType 
	{
		eQueueType_AllCommands, eQueueType_CopyOnly, eQueueType_ComputeOnly
	};

	struct ICommandQueue
	{
		virtual ~ICommandQueue() {};
	};
}
