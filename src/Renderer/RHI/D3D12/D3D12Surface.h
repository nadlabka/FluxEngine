#pragma once
#include <cstdint>
#include "../Surafce.h"

namespace RHI
{
	struct D3D12Surface : ISurface
	{
		D3D12Surface(const void* windowHandle, uint32_t width, uint32_t height)
			: m_windowHandle(windowHandle), m_width(width), m_height(height) {}

		const void* m_windowHandle;
		uint32_t m_width;
		uint32_t m_height;
	};
}