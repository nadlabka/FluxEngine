#include <stdafx.h>
#include "D3D12Device.h"
#include "D3D12CommandQueue.h"
#include "D3D12Texture.h"
#include "D3D12CommandBuffer.h"
#include "D3D12PipelineLayout.h"
#include "D3D12RenderPass.h"
#include "D3D12RenderPipeline.h"

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
    return std::make_shared<D3D12RenderPass>(renderPassDesc);
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
        const DescriptorBinding& binding = item.second;
        CD3DX12_DESCRIPTOR_RANGE1 range = {};
        switch (binding.descriptorsType)
        {
        case DescriptorType::ConstantBuffer:
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, binding.descriptorsCount, binding.bindingIndex, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
            ranges.push_back(range);
            break;
        case DescriptorType::StorageBuffer:
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, binding.descriptorsCount, binding.bindingIndex, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
            ranges.push_back(range);
            break;
        case DescriptorType::UniformBuffer:
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, binding.descriptorsCount, binding.bindingIndex, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
            ranges.push_back(range);
            break;
        }
        CD3DX12_ROOT_PARAMETER1 rootParam;
        rootParam.InitAsDescriptorTable(1, &ranges.back(), ConvertShaderVisibilityToD3D12(binding.stageVisbility));
        rootParameters.push_back(rootParam);
    }

    for (auto& item : pipelineLayoutDesc.texturesBindings)
    {
        const DescriptorBinding& binding = item.second;
        CD3DX12_DESCRIPTOR_RANGE1 range = {};
        switch (item.second.descriptorsType)
        {
        case DescriptorType::SampledImage:
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, binding.descriptorsCount, binding.bindingIndex, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
            ranges.push_back(range);
            break;
        case DescriptorType::StorageImage:
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, binding.descriptorsCount, binding.bindingIndex, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
            ranges.push_back(range);
            break;
        }
        CD3DX12_ROOT_PARAMETER1 rootParam;
        rootParam.InitAsDescriptorTable(1, &ranges.back(), ConvertShaderVisibilityToD3D12(binding.stageVisbility));
        rootParameters.push_back(rootParam);
    }

    for (auto& item : pipelineLayoutDesc.samplersBindings)
    {
        const DescriptorBinding& binding = item.second;
        CD3DX12_DESCRIPTOR_RANGE1 range = {};
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, binding.descriptorsCount, binding.bindingIndex, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
        ranges.push_back(range);

        CD3DX12_ROOT_PARAMETER1 rootParam;
        rootParam.InitAsDescriptorTable(1, &ranges.back(), ConvertShaderVisibilityToD3D12(binding.stageVisbility));
        rootParameters.push_back(rootParam);
    }

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(rootParameters.size(), rootParameters.data(), 0, nullptr, rootSignatureFlags);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(
        D3DX12SerializeVersionedRootSignature(
            &rootSignatureDescription,
            featureData.HighestVersion,
            &signature,
            &error)
    );

    ComPtr<ID3D12RootSignature> rootSignature;
    ThrowIfFailed(m_device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)
    ));

    return std::make_shared<D3D12PipelineLayout>(rootSignature);
}

std::shared_ptr<RHI::IRenderPipeline> RHI::D3D12Device::CreateRenderPipeline(const RenderPipelineDescription& renderPipelineDesc) const
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;
    for (const auto& attribute : renderPipelineDesc.inputAssemblerLayout.attributeDescriptions)
    {
        auto& bindingsVector = renderPipelineDesc.inputAssemblerLayout.vertexBindings;
        auto bindingDescriptionIter = std::find_if(bindingsVector.begin(), bindingsVector.end(), 
            [&](const InputAssemblerLayoutDescription::BindingDescription& bindingDesc)
            {
            return bindingDesc.binding == attribute.binding;
            }
        );
        auto& bindingDescription = *bindingDescriptionIter;

        D3D12_INPUT_ELEMENT_DESC elementDesc = {};
        elementDesc.SemanticName = attribute.semanticsName.c_str();
        elementDesc.SemanticIndex = attribute.semanticsIndex;
        elementDesc.Format = ConvertVertexAttributeFormatToDXGI(attribute.format);
        elementDesc.InputSlot = attribute.binding;
        elementDesc.AlignedByteOffset = attribute.offset;
        elementDesc.InputSlotClass = bindingDescription.inputRate == BindingInputRate::PerVertex ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
        inputElementDescs.push_back(elementDesc);
    }
    psoDesc.InputLayout = { inputElementDescs.data(), (UINT)inputElementDescs.size() };

    psoDesc.PrimitiveTopologyType = ConvertPrimitiveTopologyToD3D12(renderPipelineDesc.inputAssembler.primitiveTopology);

}
