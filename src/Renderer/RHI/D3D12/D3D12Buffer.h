#pragma once
#include "../Buffer.h"

namespace RHI
{
	struct D3D12Buffer : public IBuffer
	{

		size_t offset;
		size_t stride;
		RscPtr<>
	};
}