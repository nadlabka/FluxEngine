#pragma once
#include <stdafx.h>
#include "../Swapchain.h"

namespace RHI
{
	struct D3D12Swapchain : public ISwapchain
	{
		D3D12Swapchain();
		D3D12Swapchain(RscPtr<IDXGISwapChain1> swapchain, uint32_t framesCount) : m_swapchain(swapchain), m_framesCount(framesCount) {}
		~D3D12Swapchain();

		uint32_t m_framesCount;
		RscPtr<IDXGISwapChain1> m_swapchain;
	};
}