#include "stdafx.h"
#include "D3D12PipelineLayout.h"

RHI::D3D12PipelineLayout::D3D12PipelineLayout(RscPtr<ID3D12RootSignature> rootSignature, PipelineLayoutBindings pipelineLayoutBindings)
    : m_rootSignature(rootSignature), IPipelineLayout(pipelineLayoutBindings)
{
}


D3D12_RESOURCE_STATES RHI::ConvertDescriptorBindingToResourceState(const DynamicallyBoundResources::ResourceBindingInfo& descriptorBinding)
{
    switch (descriptorBinding.type)
    {
    case DescriptorResourceType::UniformBuffer:
        return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

    case DescriptorResourceType::StorageBuffer:
        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    case DescriptorResourceType::DataReadOnlyBuffer:
    case DescriptorResourceType::SampledImage:
    {
        D3D12_RESOURCE_STATES result = {};
        if (descriptorBinding.visibility & BindingVisibility::Fragment)
        {
            result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        }
        if (descriptorBinding.visibility & ~BindingVisibility::Fragment)
        {
            result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        }
        return result;
    }  

    case DescriptorResourceType::StorageImage:
        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    default:
        return D3D12_RESOURCE_STATE_COMMON;
    }
}
