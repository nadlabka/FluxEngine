#pragma once
#include <stdafx.h>
#include "../Swapchain.h"

namespace RHI
{
	struct D3D12Swapchain : ISwapchain
	{
		D3D12Swapchain();
		D3D12Swapchain(RscPtr<IDXGISwapChain1> swapchain) : m_swapchain(swapchain) {}
		~D3D12Swapchain();

		RscPtr<IDXGISwapChain1> m_swapchain;
	}
}