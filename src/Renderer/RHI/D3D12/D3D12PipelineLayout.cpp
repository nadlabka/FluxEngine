#include "stdafx.h"
#include "D3D12PipelineLayout.h"

RHI::D3D12PipelineLayout::D3D12PipelineLayout(RscPtr<ID3D12RootSignature> rootSignature, PipelineLayoutDescription description)
    : m_rootSignature(rootSignature), m_description(description)
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


D3D12_RESOURCE_STATES RHI::ConvertDescriptorBindingToResourceState(const DescriptorBinding& descriptorBinding)
{
    switch (descriptorBinding.descriptorType)
    {
    case DescriptorType::UniformBuffer:
        return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

    case DescriptorType::StorageBuffer:
        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    case DescriptorType::DataReadBuffer:
    case DescriptorType::SampledImage:
    {
        D3D12_RESOURCE_STATES result = {};
        if (descriptorBinding.stageVisbility & BindingVisibility::Fragment)
        {
            result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        }
        if (descriptorBinding.stageVisbility & ~BindingVisibility::Fragment)
        {
            result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        }
        return result;
    }  

    case DescriptorType::StorageImage:
        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    default:
        return D3D12_RESOURCE_STATE_COMMON;
    }
}
