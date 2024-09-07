#pragma once
#include <stdafx.h>
#include "../Device.h"

namespace RHI
{
	struct D3D12Device : IDevice
	{
		D3D12Device();
		D3D12Device(RscPtr<ID3D12Device> device) :  m_device(device) {}
		~D3D12Device();

		RscPtr<ID3D12Device> m_device;
	};
}