#pragma once
#include "Device.h"

namespace RHI
{
	struct AdapterCreateDesc
	{
		bool useWarpDevice;
		bool useHighPerformanceAdapter;
	};

	struct IAdapter
	{
		virtual ~IAdapter() {}

		virtual std::shared_ptr<IDevice> CreateDevice(const DeviceCreateDesc& deviceDesc) const = 0;
	};
}