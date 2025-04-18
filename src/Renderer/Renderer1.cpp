#include "stdafx.h"
#include "RHI/RHIContext.h"
#include "Renderer1.h"
#include <DXSampleHelper.h>
#include "../Application/WinAPI/WinApplication.h"
#include "../Application/WinAPI/WinWindow.h"
#include "RHI/D3D12/D3D12Shader.h"
#include <filesystem>
#include "RHI/D3D12/D3D12Swapchain.h"
#include "RHI/D3D12/D3D12CommandQueue.h"
#include "RHI/D3D12/D3D12CommandBuffer.h"
#include "RHI/D3D12/D3D12Buffer.h"
#include <ECS/Entity.h>
#include <ECS/Components/InstancedStaticMesh.h>
#include <ECS/Components/MaterialParameters.h>
#include "Managers/ConstantBufferManager.h"
#include "DataTypes/PerViewConstantBuffer.h"
#include "Managers/LightSourcesManager.h"
#include "DataTypes/PerFrameConstantBuffer.h"

void Renderer1::Init()
{
    auto& rhiContext = RHIContext::GetInstance();
    auto factory = rhiContext.GetFactory();
    auto device = rhiContext.GetDevice();
    auto rhiAllocator = rhiContext.GetAllocator();
     
    auto& window = Application::WinApplication::GetWindow();
    auto surface = ISurface::CreateSurfaceFromWindow(window);

    m_scissorsRect = {
        0,
        0,
        window.GetWidth(),
        window.GetHeight()
    };

    m_viewportInfo = {
        0.0f,
        0.0f,
        (float)window.GetWidth(),
        (float)window.GetHeight(),
        0.0f,
        1.0f
    };
 
    m_commandQueue = device->CreateCommandQueue(QueueType::AllCommands);
    m_swapchain = factory->CreateSwapchain(surface, m_commandQueue, 2);
    m_commandBuffer = device->CreateCommandBuffer(QueueType::AllCommands);

    TextureDescription depthStencilDesc = {};
    depthStencilDesc.usage = eTextureUsage_DepthStencilAttachment;
    depthStencilDesc.aspect = static_cast<TextureAspect>(eTextureAspect_HasStencil | eTextureAspect_HasDepth);
    depthStencilDesc.format = TextureFormat::D24_UNORM_S8_UINT;
    depthStencilDesc.type = TextureType::Texture2D;
    depthStencilDesc.layout = TextureLayout::DepthStencilAttachmentOptimal;
    depthStencilDesc.width = window.GetWidth();
    depthStencilDesc.height = window.GetHeight();
    m_depthStencil = rhiAllocator->CreateTexture(depthStencilDesc);

    auto& constantBufferMgr = ConstantBufferManager::GetInstance();
    constantBufferMgr.RegisterBuffer<PerViewConstantBuffer>("PerView", m_commandBuffer);
    constantBufferMgr.RegisterBuffer<PerFrameConstantBuffer>("PerFrame", m_commandBuffer);

    auto& lightSourcesManager = LightSourcesManager::GetInstance();
    lightSourcesManager.InitLightsRHIBuffers(m_commandBuffer);
}

void Renderer1::Render()
{
    UpdatePipelineDynamicStates();
    ExperimentalDrawCube();
    //PopulateCommandList();

    m_swapchain->Present();
}

void Renderer1::LoadPipeline()
{
    auto& rhiContext = RHIContext::GetInstance();
    auto device = rhiContext.GetDevice();
    auto allocator = rhiContext.GetAllocator();

    InputAssemblerLayoutDescription inputAssemblerLayoutDesc;
    inputAssemblerLayoutDesc.vertexBindings.push_back(
        {
            sizeof(Assets::VertexPrimaryAttributes),
            0,
            BindingInputRate::PerVertex
        });
    inputAssemblerLayoutDesc.vertexBindings.push_back(
        {
            sizeof(Assets::VertexSecondaryAttributes),
            1,
            BindingInputRate::PerVertex
        });
    inputAssemblerLayoutDesc.vertexBindings.push_back(
        {
            sizeof(Assets::MeshPerInstanceDataHandle),
            2,
            BindingInputRate::PerInstance
        });
    inputAssemblerLayoutDesc.attributeDescriptions.push_back(
        {
            0,
            0,
            0,
            VertexAttributeFormat::R32G32B32_SignedFloat,
            "POSITION",
            0
        });
    inputAssemblerLayoutDesc.attributeDescriptions.push_back(
        {
            1,
            0,
            12,
            VertexAttributeFormat::R32G32B32_SignedFloat,
            "NORMAL",
            0
        });
    inputAssemblerLayoutDesc.attributeDescriptions.push_back(
        {
            2,
            0,
            24,
            VertexAttributeFormat::R32G32_SignedFloat,
            "TEXCOORD",
            0
        });
    inputAssemblerLayoutDesc.attributeDescriptions.push_back(
        {
            4,
            1,
            0,
            VertexAttributeFormat::R32G32B32_SignedFloat,
            "TANGENT",
            0
        });
    inputAssemblerLayoutDesc.attributeDescriptions.push_back(
        {
            5,
            1,
            12,
            VertexAttributeFormat::R32G32B32_SignedFloat,
            "BITANGENT",
            0
        });


    InputAssemblerDescription inputAssemblerDesc = {};
    inputAssemblerDesc.primitiveTopology = PrimitiveTopology::TriangleList;


    RasterizerDescription rasterizerDesc = {};
    rasterizerDesc.windingOrder = WindingOrder::Clockwise;
    

    ColorBlendDescription colorBlendDesc = {};
    ColorBlendDescription::ColorAttachmentBlendDesc colorAttachmentBlendDesc = {};
    colorBlendDesc.attachmentsBlends.push_back(colorAttachmentBlendDesc);


    DepthStencilDescription depthStencilDesc = {};
    depthStencilDesc.depthTestEnabled = true;
    depthStencilDesc.depthCompareOperation = DepthStencilCompareOperation::GreaterOrEqual;
    depthStencilDesc.depthWriteEnabled = true;


    std::vector<PipelineStageDescription> pipelineStagesDesc = {};
    ShaderCreateDesription vertexShaderDesc =
    {
        "../../../../Assets/Shaders/Source/PBR.hlsl",
        L"VSMain",
        PipelineStageType::Vertex,
        "../../../../Assets/Shaders/PDB"
    };
    ShaderCreateDesription fragmentShaderDesc =
    {
        "../../../../Assets/Shaders/Source/PBR.hlsl",
        L"PSMain",
        PipelineStageType::Fragment,
        "../../../../Assets/Shaders/PDB"
    };
    auto shaderCompiler = rhiContext.GetShaderCompiler();
    std::shared_ptr<IShader> vertexShader = shaderCompiler->CompileShader(vertexShaderDesc);
    std::shared_ptr<IShader> fragmentShader = shaderCompiler->CompileShader(fragmentShaderDesc);
    pipelineStagesDesc.push_back({ vertexShader });
    pipelineStagesDesc.push_back({ fragmentShader });

    PipelineLayoutBindings pipelineLayoutDesc = {};
    std::shared_ptr<IPipelineLayout> pipelineLayout = device->CreatePipelineLayout(pipelineStagesDesc);

    std::shared_ptr<IRenderPass> renderPass = {};
    std::vector<AttachmentDesc> colorAttachmentsDesc = {};
    std::optional<AttachmentDesc> depthStencilAttachmentDesc = {};
    AttachmentDesc rtAttachmentDesc = {};
    rtAttachmentDesc.clearColor = { 0.0f, 0.2f, 0.4f, 1.0f };
    colorAttachmentsDesc.push_back(rtAttachmentDesc);
    AttachmentDesc dsAttachmentDesc = {};
    dsAttachmentDesc.format = TextureFormat::D24_UNORM_S8_UINT;
    dsAttachmentDesc.clearDepth = 0.0f;
    dsAttachmentDesc.clearStencil = 0.0f;
    dsAttachmentDesc.initialLayout = TextureLayout::DepthStencilAttachmentOptimal;
    dsAttachmentDesc.finalLayout = TextureLayout::DepthStencilAttachmentOptimal;
    depthStencilAttachmentDesc = dsAttachmentDesc;

    RenderPassDesc renderPassDesc =
    {
        colorAttachmentsDesc,
        depthStencilAttachmentDesc
    };
    renderPass = device->CreateRenderPass(renderPassDesc);

    RenderPipelineDescription pipelineDesc;
    pipelineDesc.inputAssemblerLayout = inputAssemblerLayoutDesc;
    pipelineDesc.inputAssembler = inputAssemblerDesc;
    pipelineDesc.rasterizer = rasterizerDesc;
    pipelineDesc.colorBlend = colorBlendDesc;
    pipelineDesc.depthStencil = depthStencilDesc;
    pipelineDesc.pipelineStages = pipelineStagesDesc;
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.renderPass = renderPass;

    m_renderPipeline = device->CreateRenderPipeline(pipelineDesc);
    auto& dynamicallyBoundResources = m_renderPipeline->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources;

    dynamicallyBoundResources.SetBufferDescriptorResourceType("perMeshDataBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("perMeshDataBufferIndex", RHI::BindingVisibility::Vertex);

    dynamicallyBoundResources.SetBufferDescriptorResourceType("perInstancePerMeshHandleBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("perInstancePerMeshHandleBufferIndex", RHI::BindingVisibility::Vertex);

    dynamicallyBoundResources.SetBufferDescriptorResourceType("perInstanceMaterialParamsBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("perInstanceMaterialParamsBufferIndex", RHI::BindingVisibility::Vertex);

    auto& constantBuffer = m_renderPipeline->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_BoundConstantBuffers;
    constantBuffer.SetBufferBindingVisibility("PerView", RHI::BindingVisibility::All);
    constantBuffer.SetBufferBindingVisibility("PerFrame", RHI::BindingVisibility::All);

    dynamicallyBoundResources.SetBufferDescriptorResourceType("pointLightsBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("pointLightsBufferIndex", RHI::BindingVisibility::Fragment);

    dynamicallyBoundResources.SetBufferDescriptorResourceType("spotLightsBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("spotLightsBufferIndex", RHI::BindingVisibility::Fragment);

    dynamicallyBoundResources.SetBufferDescriptorResourceType("directionalLightsBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("directionalLightsBufferIndex", RHI::BindingVisibility::Fragment);

    m_commandQueue->WaitUntilCompleted();
}

void Renderer1::PopulateCommandList()
{
    m_commandBuffer->BindRenderPipeline(m_renderPipeline);
    
    m_commandBuffer->BeginRecording(m_commandQueue);

    m_commandBuffer->SetViewport(m_viewportInfo);
    m_commandBuffer->SetScissors(m_scissorsRect);
    m_commandBuffer->SetPrimitiveTopology(PrimitiveTopology::TriangleList);

    BufferRegionDescription bufferbindDesc;
    bufferbindDesc.offset = 0;
    bufferbindDesc.size = m_buffer->GetSize();
    m_commandBuffer->SetVertexBuffer(m_buffer, 0, bufferbindDesc);

    InstancedDrawInfo instancedDrawInfo = {};
    instancedDrawInfo.verticesPerInstanceNum = 3;
    m_commandBuffer->DrawInstanced(instancedDrawInfo);

    m_commandBuffer->EndRecording();
}

void Renderer1::WaitForGpu()
{
    m_commandQueue->WaitUntilCompleted();
}

void Renderer1::ExperimentalDrawCube()
{
    m_commandBuffer->BindRenderPipeline(nullptr);
    m_commandBuffer->BeginRecording(m_commandQueue);
    auto view = Core::EntitiesPool::GetInstance().GetRegistry().view<Components::InstancedStaticMesh>();
    for (auto entity : view)
    {
        auto& meshComponent = view.get<Components::InstancedStaticMesh>(entity);
        auto& staticMesh = Assets::AssetsManager<Assets::StaticMesh>::GetInstance().GetAsset(meshComponent.staticMesh);
        staticMesh.UpdateRHIBufferWithPerInstanceData(m_commandBuffer);
    }
    m_commandBuffer->EndRecording();
    m_commandBuffer->SubmitToQueue(m_commandQueue);

    // one section per Material
    {
        m_commandBuffer->BindRenderPipeline(m_renderPipeline);

        auto& constantBufferManager = ConstantBufferManager::GetInstance();
        auto& boundConstantBuffers = m_commandBuffer->GetCurrentRenderPipeline()->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_BoundConstantBuffers;
        boundConstantBuffers.SetConstantBufferBindingMapping("PerView", constantBufferManager.GetDataBufferByName("PerView"));    
        constantBufferManager.UpdateBuffer<PerViewConstantBuffer>(m_commandQueue, m_commandBuffer, "PerView");
        boundConstantBuffers.SetConstantBufferBindingMapping("PerFrame", constantBufferManager.GetDataBufferByName("PerFrame"));
        constantBufferManager.UpdateBuffer<PerFrameConstantBuffer>(m_commandQueue, m_commandBuffer, "PerFrame");

        m_commandBuffer->BeginRecording(m_commandQueue);

        m_commandBuffer->SetViewport(m_viewportInfo);
        m_commandBuffer->SetScissors(m_scissorsRect);
        m_commandBuffer->SetPrimitiveTopology(PrimitiveTopology::TriangleList);

        m_commandBuffer->BindRenderTargets();

        auto& lightSourcesManager = LightSourcesManager::GetInstance();
        lightSourcesManager.UpdateLightsRHIBuffers(m_commandBuffer);

        auto& dynamicallyBoundResources = m_commandBuffer->GetCurrentRenderPipeline()->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources;    
        dynamicallyBoundResources.SetBufferBindingResource("pointLightsBufferIndex", lightSourcesManager.GetPointLightSRV());
        dynamicallyBoundResources.SetBufferBindingResource("spotLightsBufferIndex", lightSourcesManager.GetSpotLightSRV());
        dynamicallyBoundResources.SetBufferBindingResource("directionalLightsBufferIndex", lightSourcesManager.GetDirectionalLightSRV());

        auto& assetsManager = Assets::AssetsManager<Assets::StaticMesh>::GetInstance();
        for (auto& staticMesh : assetsManager.GetAssetsStorage().GetDataStorage())
        {
            dynamicallyBoundResources.SetBufferBindingResource("perMeshDataBufferIndex", staticMesh.GetRHIBufferForPerInstanceData());
            
            for (auto& submesh : staticMesh.m_submeshes)
            {
                if (submesh.GetActiveInstancesNum<MaterialParameters::PBRMaterial>() == 0) { continue; }

                submesh.UpdateRHIBuffersWithPerInstanceData<MaterialParameters::PBRMaterial>(m_commandBuffer);

                // Bind all buffers
                auto perInstancePerMeshDataBuffer = submesh.GetRHIBufferForPerMeshData<MaterialParameters::PBRMaterial>();
                auto perInstancePerMaterialDataBuffer = submesh.GetRHIBufferForPerMaterialData<MaterialParameters::PBRMaterial>();

                dynamicallyBoundResources.SetBufferBindingResource("perInstancePerMeshHandleBufferIndex", perInstancePerMeshDataBuffer);
                dynamicallyBoundResources.SetBufferBindingResource("perInstanceMaterialParamsBufferIndex", perInstancePerMaterialDataBuffer);

                m_commandBuffer->BindPipelineResources();

                auto bufferWithRegionDescription = submesh.GetPrimaryVertexData();
                m_commandBuffer->SetVertexBuffer(bufferWithRegionDescription.buffer, 0, bufferWithRegionDescription.regionDescription);

                bufferWithRegionDescription = submesh.GetSecondaryVertexData();
                m_commandBuffer->SetVertexBuffer(bufferWithRegionDescription.buffer, 1, bufferWithRegionDescription.regionDescription);

                bufferWithRegionDescription = submesh.GetIndicesData();
                m_commandBuffer->SetIndexBuffer(bufferWithRegionDescription.buffer, bufferWithRegionDescription.regionDescription);

                IndexedInstancedDrawInfo indexedInstancedDrawInfo = {};
                indexedInstancedDrawInfo.indicesPerInstanceNum = submesh.GetIndicesData().buffer->GetStructuredElementsNum();
                indexedInstancedDrawInfo.instancesNum = submesh.GetActiveInstancesNum<MaterialParameters::PBRMaterial>();
                m_commandBuffer->DrawIndexedInstanced(indexedInstancedDrawInfo);
            }
        }
        m_commandBuffer->FinishRenderTargets();
        m_commandBuffer->EndRecording();
        m_commandBuffer->SubmitToQueue(m_commandQueue);
    }
    {

    }
    {

    }
}

void Renderer1::Destroy()
{
    m_depthStencil.reset();
    m_buffer.reset();
    m_renderPipeline.reset();
    m_commandBuffer.reset();
    m_commandQueue.reset();
    m_swapchain.reset();
}

void Renderer1::UpdatePipelineDynamicStates()
{
    std::vector<SubResourceRTsDescription> subresourceRTs = {};
    std::vector<SubResourceRTsDescription::TextureArraySliceToInclude> slicesToInclude = {};
    slicesToInclude.push_back({});
    subresourceRTs.push_back({ m_swapchain->GetNextRenderTarget(), slicesToInclude });
    SubResourceRTsDescription subresourceDST = {};
    subresourceDST.slicesToInclude.push_back({});
    subresourceDST.texture = m_depthStencil;
    m_renderPipeline->GetPipelineDescription().renderPass->SetAttachments(subresourceRTs, subresourceDST);
}
