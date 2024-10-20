#include "stdafx.h"
#include "D3D12PipelineLayout.h"

RHI::D3D12PipelineLayout::D3D12PipelineLayout(RscPtr<ID3D12RootSignature> rootSignature) 
    : m_rootSignature(rootSignature)
{

}

D3D12_SHADER_VISIBILITY RHI::ConvertShaderVisibilityToD3D12(BindingVisibility visibility)
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
