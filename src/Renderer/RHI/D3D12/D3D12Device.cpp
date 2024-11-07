#include <stdafx.h>
#include "D3D12Device.h"
#include "D3D12CommandQueue.h"
#include "D3D12Texture.h"
#include "D3D12CommandBuffer.h"
#include "D3D12PipelineLayout.h"
#include "D3D12RenderPass.h"
#include "D3D12RenderPipeline.h"
#include "D3D12Shader.h"
#include "D3D12Buffer.h"
#include "D3D12Sampler.h"

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
    queueDesc.Type = ConvertQueueTypeToCommandListType(queueType);

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
    ThrowIfFailed(m_device->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    return std::make_shared<D3D12CommandQueue>(commandQueue, fence, 0u);
}

std::shared_ptr<RHI::ICommandBuffer> RHI::D3D12Device::CreateCommandBuffer(QueueType bufferSubmitQueueType) const
{
    RscPtr<ID3D12CommandAllocator> commandAllocator;
    RscPtr<ID3D12GraphicsCommandList> commandList;

    auto commandListType = ConvertQueueTypeToCommandListType(bufferSubmitQueueType);

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
        auto buffer = std::static_pointer_cast<D3D12Buffer>(item.first);
        const DescriptorBinding& binding = item.second;
        CD3DX12_DESCRIPTOR_RANGE1 range = {};
        switch (binding.descriptorType)
        {
        case DescriptorType::DataReadBuffer:
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, binding.bindingIndex, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
            range.OffsetInDescriptorsFromTableStart = buffer->m_SRVDescriptorIndex;
            ranges.push_back(range);
            break;
        case DescriptorType::StorageBuffer:
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, binding.bindingIndex, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
            range.OffsetInDescriptorsFromTableStart = buffer->m_UAVDescriptorIndex;
            ranges.push_back(range);
            break;
        case DescriptorType::UniformBuffer:
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, binding.bindingIndex, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
            range.OffsetInDescriptorsFromTableStart = buffer->m_CBVDescriptorIndex;
            ranges.push_back(range);
            break;
        }
        CD3DX12_ROOT_PARAMETER1 rootParam;
        rootParam.InitAsDescriptorTable(1, &ranges.back(), ConvertShaderVisibilityToD3D12(binding.stageVisbility));
        rootParameters.push_back(rootParam);
    }

    for (auto& item : pipelineLayoutDesc.texturesBindings)
    {
        auto texture = std::static_pointer_cast<D3D12Texture>(item.first);
        const DescriptorBinding& binding = item.second;
        CD3DX12_DESCRIPTOR_RANGE1 range = {};
        switch (item.second.descriptorType)
        {
        case DescriptorType::SampledImage:
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, binding.bindingIndex, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
            range.OffsetInDescriptorsFromTableStart = texture->m_SRVDescriptorIndex;
            ranges.push_back(range);
            break;
        case DescriptorType::StorageImage:
            for(int i = 0; i < binding.mipsToIncludeForUAV.size(); i++)
            {
                range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, binding.bindingIndex, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
                range.OffsetInDescriptorsFromTableStart = texture->m_UAVDescriptorsIndices[i];
                ranges.push_back(range);
            }
            break;
        }
        CD3DX12_ROOT_PARAMETER1 rootParam;
        rootParam.InitAsDescriptorTable(1, &ranges.back(), ConvertShaderVisibilityToD3D12(binding.stageVisbility));
        rootParameters.push_back(rootParam);
    }

    for (auto& item : pipelineLayoutDesc.samplersBindings)
    {
        auto sampler = std::static_pointer_cast<D3D12Sampler>(item.first);
        const DescriptorBinding& binding = item.second;
        CD3DX12_DESCRIPTOR_RANGE1 range = {};
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, binding.bindingIndex, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
        range.OffsetInDescriptorsFromTableStart = sampler->m_descriptorIndex;
        ranges.push_back(range);

        CD3DX12_ROOT_PARAMETER1 rootParam;
        rootParam.InitAsDescriptorTable(1, &ranges.back(), ConvertShaderVisibilityToD3D12(binding.stageVisbility));
        rootParameters.push_back(rootParam);
    }

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(rootParameters.size(), rootParameters.data(), 0, nullptr, rootSignatureFlags);

    RscPtr<ID3DBlob> signature;
    RscPtr<ID3DBlob> error;
    ThrowIfFailed(
        D3DX12SerializeVersionedRootSignature(
            &rootSignatureDescription,
            featureData.HighestVersion,
            &signature,
            &error)
    );

    RscPtr<ID3D12RootSignature> rootSignature;
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
    {
        psoDesc.pRootSignature = std::static_pointer_cast<D3D12PipelineLayout>(renderPipelineDesc.pipelineLayout)->m_rootSignature.ptr();
    }
    {
        for (auto& pipelineStage : renderPipelineDesc.pipelineStages)
        {
            auto d3d12pipelineStage = std::static_pointer_cast<D3D12Shader>(pipelineStage.shader);
            switch (d3d12pipelineStage->m_pipelineStage)
            {
            case PipelineStage::Vertex:
                psoDesc.VS = CD3DX12_SHADER_BYTECODE(d3d12pipelineStage->m_compiledShader.ptr());
                break;
            case PipelineStage::Fragment:
                psoDesc.PS = CD3DX12_SHADER_BYTECODE(d3d12pipelineStage->m_compiledShader.ptr());
                break;
            }
        }
    }
    {
        D3D12_BLEND_DESC d3d12BlendDesc = {};
        d3d12BlendDesc.IndependentBlendEnable = renderPipelineDesc.colorBlend.independentBlendEnabled;
        d3d12BlendDesc.AlphaToCoverageEnable = false;
        for (uint32_t i = 0; i < renderPipelineDesc.colorBlend.attachmentsBlends.size(); i++)
        {
            auto& RTBlendDesc = renderPipelineDesc.colorBlend.attachmentsBlends[i];

            D3D12_RENDER_TARGET_BLEND_DESC d3d12RTBlendDesc = {};
            d3d12RTBlendDesc.BlendEnable = RTBlendDesc.blendEnabled;
            d3d12RTBlendDesc.LogicOpEnable = renderPipelineDesc.colorBlend.logicalOperationEnabled;
            d3d12RTBlendDesc.LogicOp = RHI::ConvertLogicalOperationToD3D12(renderPipelineDesc.colorBlend.logicalOperation);

            d3d12RTBlendDesc.SrcBlend = RHI::ConvertBlendFactorToD3D12(RTBlendDesc.sourceColorBlendFactor);
            d3d12RTBlendDesc.DestBlend = RHI::ConvertBlendFactorToD3D12(RTBlendDesc.destinationColorBlendFactor);
            d3d12RTBlendDesc.BlendOp = RHI::ConvertBlendOperationToD3D12(RTBlendDesc.colorBlendOperation);

            d3d12RTBlendDesc.SrcBlendAlpha = RHI::ConvertBlendFactorToD3D12(RTBlendDesc.sourceAlphaBlendFactor);
            d3d12RTBlendDesc.DestBlendAlpha = RHI::ConvertBlendFactorToD3D12(RTBlendDesc.destinationAlphaBlendFactor);
            d3d12RTBlendDesc.BlendOpAlpha = RHI::ConvertBlendOperationToD3D12(RTBlendDesc.alphaBlendOperation);

            d3d12RTBlendDesc.RenderTargetWriteMask = static_cast<UINT8>(RTBlendDesc.colorWriteMask);

            d3d12BlendDesc.RenderTarget[i] = d3d12RTBlendDesc;
        }
        psoDesc.BlendState = d3d12BlendDesc;
    }
    {
        psoDesc.SampleMask = UINT_MAX;
    }
    {
        D3D12_RASTERIZER_DESC d3d12RasterizerState = {};
        d3d12RasterizerState.FillMode = RHI::ConvertPolygonFillModeToD3D12(renderPipelineDesc.rasterizer.polygonOverride);
        d3d12RasterizerState.CullMode = RHI::ConvertCullModeToD3D12(renderPipelineDesc.rasterizer.cullMode);
        d3d12RasterizerState.FrontCounterClockwise = (renderPipelineDesc.rasterizer.windingOrder == WindingOrder::Counterclockwise) ? TRUE : FALSE;
        d3d12RasterizerState.DepthBias = static_cast<INT>(renderPipelineDesc.rasterizer.depthBias.constantFactor);
        d3d12RasterizerState.DepthBiasClamp = renderPipelineDesc.rasterizer.depthBias.clampValue;
        d3d12RasterizerState.SlopeScaledDepthBias = renderPipelineDesc.rasterizer.depthBias.slopeScaledFactor;
        d3d12RasterizerState.DepthClipEnable = renderPipelineDesc.rasterizer.depthClipEnable;
        d3d12RasterizerState.MultisampleEnable = FALSE;
        d3d12RasterizerState.AntialiasedLineEnable = FALSE;
        d3d12RasterizerState.ForcedSampleCount = 0;
        d3d12RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        psoDesc.RasterizerState = d3d12RasterizerState;
    }
    {
        D3D12_DEPTH_STENCIL_DESC d3d12DepthStencil = {};
        d3d12DepthStencil.DepthEnable = renderPipelineDesc.depthStencil.depthTestEnabled ? TRUE : FALSE;
        d3d12DepthStencil.DepthWriteMask = renderPipelineDesc.depthStencil.depthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        d3d12DepthStencil.DepthFunc = ConvertDepthStencilCompareOperationToD3D12(renderPipelineDesc.depthStencil.depthCompareOperation);

        d3d12DepthStencil.StencilEnable = renderPipelineDesc.depthStencil.stencilTestEnabled ? TRUE : FALSE;
        d3d12DepthStencil.StencilReadMask = renderPipelineDesc.depthStencil.stencilReadMask;
        d3d12DepthStencil.StencilWriteMask = renderPipelineDesc.depthStencil.stencilWriteMask;

        d3d12DepthStencil.FrontFace.StencilFailOp = ConvertStencilOperationToD3D12(renderPipelineDesc.depthStencil.frontStencilState.stencilFailOperation);
        d3d12DepthStencil.FrontFace.StencilDepthFailOp = ConvertStencilOperationToD3D12(renderPipelineDesc.depthStencil.frontStencilState.stencilDepthFailOperation);
        d3d12DepthStencil.FrontFace.StencilPassOp = ConvertStencilOperationToD3D12(renderPipelineDesc.depthStencil.frontStencilState.stencilPassOperation);
        d3d12DepthStencil.FrontFace.StencilFunc = ConvertDepthStencilCompareOperationToD3D12(renderPipelineDesc.depthStencil.frontStencilState.stencilCompareOperation);

        d3d12DepthStencil.BackFace.StencilFailOp = ConvertStencilOperationToD3D12(renderPipelineDesc.depthStencil.backStencilState.stencilFailOperation);
        d3d12DepthStencil.BackFace.StencilDepthFailOp = ConvertStencilOperationToD3D12(renderPipelineDesc.depthStencil.backStencilState.stencilDepthFailOperation);
        d3d12DepthStencil.BackFace.StencilPassOp = ConvertStencilOperationToD3D12(renderPipelineDesc.depthStencil.backStencilState.stencilPassOperation);
        d3d12DepthStencil.BackFace.StencilFunc = ConvertDepthStencilCompareOperationToD3D12(renderPipelineDesc.depthStencil.backStencilState.stencilCompareOperation);

        psoDesc.DepthStencilState = d3d12DepthStencil;
    }
    {
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
    }
    {
        psoDesc.PrimitiveTopologyType = ConvertPrimitiveTopologyToD3D12TopologyType(renderPipelineDesc.inputAssembler.primitiveTopology);
    }
    {
        auto d3d12RenderPass = std::static_pointer_cast<D3D12RenderPass>(renderPipelineDesc.renderPass);
        psoDesc.NumRenderTargets = d3d12RenderPass->m_description.colorAttachments.size();
        for (int i = 0; i < d3d12RenderPass->m_description.colorAttachments.size(); i++)
        {
            psoDesc.RTVFormats[i] = ConvertFormatToD3D12(d3d12RenderPass->m_description.colorAttachments[i].format);
        }
        psoDesc.DSVFormat = d3d12RenderPass->m_description.depthStencilAttachment.has_value() ? ConvertFormatToD3D12(d3d12RenderPass->m_description.depthStencilAttachment->format) : DXGI_FORMAT_UNKNOWN;
    }
    {
        DXGI_SAMPLE_DESC d3d12SampleDesc = {};
        d3d12SampleDesc.Count = 1;
        psoDesc.SampleDesc = d3d12SampleDesc;
    }

    RscPtr<ID3D12PipelineState> d3d12PipelineState;
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&d3d12PipelineState)));

    return std::make_shared<D3D12RenderPipeline>(d3d12PipelineState, renderPipelineDesc);
}
