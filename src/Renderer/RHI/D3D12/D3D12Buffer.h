#pragma once
#include "../Buffer.h"
#include <cstdint>

namespace RHI
{
	struct D3D12Buffer : public IBuffer
	{

		uint32_t size;
	};
}