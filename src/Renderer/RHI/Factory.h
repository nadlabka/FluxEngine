#pragma once
#include <stdafx.h>
#include <memory>
#include "Adapter.h"
#include "Surface.h"
#include "Swapchain.h"

namespace RHI
{
	struct IFactory
	{
		virtual ~IFactory() {}

		virtual std::shared_ptr<IAdapter> CreateAdapter(AdapterCreateDesc adapterDesc) const = 0;
		virtual std::shared_ptr<ISwapchain> CreateSwapchain(std::shared_ptr<ISurface> surface, std::shared_ptr<ICommandQueue> commandQueue, uint32_t framesCount) const = 0;
	};
}