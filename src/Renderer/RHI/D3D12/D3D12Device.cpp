#include <stdafx.h>
#include "D3D12Device.h"
#include "D3D12CommandQueue.h"
#include "D3D12Texture.h"
#include "D3D12CommandBuffer.h"
#include "D3D12PipelineLayout.h"

RHI::D3D12Device::D3D12Device()
{

}

RHI::D3D12Device::~D3D12Device()
{

}

std::shared_ptr<RHI::ICommandQueue> RHI::D3D12Device::CreateCommandQueue(QueueType queueType) const
{
    RscPtr<ID3D12CommandQueue> commandQueue;
    RscPtr<ID3D12Fence> fence;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = ConverQueueTypeToCommandListType(queueType);

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
    ThrowIfFailed(m_device->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    return std::make_shared<D3D12CommandQueue>(commandQueue, fence, 0u);
}

std::shared_ptr<RHI::ICommandBuffer> RHI::D3D12Device::CreateCommandBuffer(QueueType bufferSubmitQueueType) const
{
    RscPtr<ID3D12CommandAllocator> commandAllocator;
    RscPtr<ID3D12GraphicsCommandList> commandList;

    auto commandListType = ConverQueueTypeToCommandListType(bufferSubmitQueueType);

    ThrowIfFailed(m_device->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&commandAllocator)));
    ThrowIfFailed(m_device->CreateCommandList(0, commandListType, commandAllocator.ptr(), nullptr, IID_PPV_ARGS(&commandList)));

    return std::make_shared<D3D12CommandBuffer>(commandAllocator, commandList);
}

std::shared_ptr<RHI::IRenderPass> RHI::D3D12Device::CreateRenderPass(const RenderPassDesc& renderPassDesc) const
{
    return std::shared_ptr<D3D12RenderPass>(sdsds);
}

std::shared_ptr<RHI::IPipelineLayout> RHI::D3D12Device::CreatePipelineLayout(const PipelineLayoutDescription& pipelineLayoutDesc) const
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
    std::list< D3D12_DESCRIPTOR_RANGE1> ranges;

    for (auto& constant : pipelineLayoutDesc.constantsBindings) 
    {
        rootParameters.emplace_back().InitAsConstants(constant.size / sizeof(uint32_t), constant.bindingIndex, 0, ConvertShaderVisibilityToD3D12(constant.visibility));
    }

    for (auto& item : pipelineLayoutDesc.buffersBindings)
    {
        switch (item.second.descriptorsType)
        {
        case DescriptorType::StorageBuffer:
            break;
        case DescriptorType::UniformBuffer:
            break;
        }
    }
    for (auto& item : pipelineLayoutDesc.texturesBindings)
    {
        switch (item.second.descriptorsType)
        {
        case DescriptorType::SampledImage:
                break;
        case DescriptorType::StorageImage:
                break;
        }
    }
    for (auto& item : pipelineLayoutDesc.samplersBindings)
    {
        
    }

    return std::shared_ptr<D3D12PipelineLayout>(sdsd);
}

std::shared_ptr<RHI::IRenderPipeline> RHI::D3D12Device::CreateRenderPipeline(const RenderPipelineDescription& renderPipelineDesc) const
{
    return std::shared_ptr<D3D12RenderPipeline>(sdsd);
}
