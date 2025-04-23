#pragma once
#include "../Sampler.h"

namespace RHI
{
	D3D12_SAMPLER_DESC ConvertSamplerDescriptionToD3D12(const SamplerDescription& desc);

	struct D3D12Sampler : ISampler
	{
		uint32_t m_descriptorIndex;
	};
}