#pragma once
#include <stdafx.h>
#include "../PipelineLayout.h"


namespace RHI
{
	D3D12_SHADER_VISIBILITY ConvertShaderVisibilityToD3D12(BindingVisibility visibility);
	D3D12_RESOURCE_STATES ConvertDescriptorBindingToResourceState(const DescriptorBinding& descriptorBinding);

	struct D3D12PipelineLayout : public IPipelineLayout
	{
        D3D12PipelineLayout(RscPtr<ID3D12RootSignature> rootSignature, PipelineLayoutDescription description);

		PipelineLayoutDescription m_description;
		RscPtr<ID3D12RootSignature> m_rootSignature;
	};
}