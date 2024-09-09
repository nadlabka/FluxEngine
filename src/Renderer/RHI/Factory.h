#pragma once
#include <stdafx.h>
#include <memory>
#include "Adapter.h"
#include "Swapchain.h"

namespace RHI
{
	struct IFactory
	{
		virtual ~IFactory() {}

		virtual std::shared_ptr<IAdapter> CreateAdapter(AdapterCreateDesc adapterDesc) const = 0;
		virtual std::shared_ptr<ISwapchain> CreateSwapchain() const = 0;
	};
}