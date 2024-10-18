#pragma once
#include <stdafx.h>
#include "../PipelineLayout.h"


namespace RHI
{
    D3D12_SHADER_VISIBILITY ConvertShaderVisibilityToD3D12(BindingVisibility visibility)
    {
        switch (visibility)
        {
        case BindingVisibility::Vertex:
            return D3D12_SHADER_VISIBILITY_VERTEX;
        case BindingVisibility::Fragment:
            return D3D12_SHADER_VISIBILITY_PIXEL;
        case BindingVisibility::Compute:
            return D3D12_SHADER_VISIBILITY_ALL;
        case BindingVisibility::VertexFragment:
            return D3D12_SHADER_VISIBILITY_ALL;
        default:
            return D3D12_SHADER_VISIBILITY_ALL;
        }
    }

	struct D3D12PipelineLayout : public IPipelineLayout
	{
		RscPtr<ID3D12RootSignature> m_rootSignature;
	};
}