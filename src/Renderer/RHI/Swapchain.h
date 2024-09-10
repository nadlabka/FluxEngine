#pragma once

namespace RHI
{
	struct ISwapchain
	{
		ISwapchain() = default;
		ISwapchain(uint32_t framesCount) : m_framesCount(framesCount) {}
		virtual ~ISwapchain() {}

		uint32_t m_framesCount;
	};
}