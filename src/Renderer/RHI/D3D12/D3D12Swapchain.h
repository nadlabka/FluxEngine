#pragma once
#include <stdafx.h>
#include "../Swapchain.h"

namespace RHI
{
	struct D3D12Swapchain : ISwapchain
	{
		D3D12Swapchain();
		D3D12Swapchain(RscPtr<IDXGISwapChain1> swapchain, uint32_t framesCount) : m_swapchain(swapchain), ISwapchain(framesCount) {}
		~D3D12Swapchain();

		RscPtr<IDXGISwapChain1> m_swapchain;
	};
}