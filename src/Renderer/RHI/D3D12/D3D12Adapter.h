#pragma once
#include <stdafx.h>
#include "../Adapter.h"
#include "../Device.h"

namespace RHI
{
	struct D3D12Adapter : public IAdapter
	{
		D3D12Adapter();
		D3D12Adapter(RscPtr<IDXGIAdapter1> adapter) : m_adapter(adapter) {}
		~D3D12Adapter();

		std::shared_ptr<IDevice> CreateDevice(DeviceCreateDesc deviceDesc) const;

		RscPtr<IDXGIAdapter1> m_adapter;
	};
}