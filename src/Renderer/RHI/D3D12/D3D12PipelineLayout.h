#pragma once
#include <stdafx.h>
#include "../PipelineLayout.h"


namespace RHI
{
	D3D12_SHADER_VISIBILITY ConvertShaderVisibilityToD3D12(BindingVisibility visibility);

	struct D3D12PipelineLayout : public IPipelineLayout
	{
        D3D12PipelineLayout(RscPtr<ID3D12RootSignature> rootSignature);

		RscPtr<ID3D12RootSignature> m_rootSignature;
	};
}