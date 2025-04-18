#pragma once
#include "../Sampler.h"

namespace RHI
{
	struct D3D12Sampler : ISampler
	{
		uint32_t m_descriptorIndex;
	};
}