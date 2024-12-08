#pragma once
#include <stdafx.h>
#include "../PipelineLayout.h"


namespace RHI
{
	D3D12_RESOURCE_STATES ConvertDescriptorBindingToResourceState(const DynamicallyBoundResources::ResourceBindingInfo& descriptorBinding);

	struct D3D12PipelineLayout : public IPipelineLayout
	{
        D3D12PipelineLayout(RscPtr<ID3D12RootSignature> rootSignature, PipelineLayoutBindings pipelineLayoutBindings);

		RscPtr<ID3D12RootSignature> m_rootSignature;
	};
}