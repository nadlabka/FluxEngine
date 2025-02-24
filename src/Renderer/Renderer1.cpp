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

    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));
    m_assetsPath = assetsPath;
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
            "NORMALS",
            0
        });
    inputAssemblerLayoutDesc.attributeDescriptions.push_back(
        {
            2,
            0,
            24,
            VertexAttributeFormat::R32G32_SignedFloat,
            "TEX_COORDS",
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
    inputAssemblerLayoutDesc.attributeDescriptions.push_back(
        {
            6,
            2,
            0,
            VertexAttributeFormat::R32_Uint,
            "PER_INSTANCE_PER_MESH_INDEX",
            0
        });


    InputAssemblerDescription inputAssemblerDesc = {};
    inputAssemblerDesc.primitiveTopology = PrimitiveTopology::TriangleList;


    RasterizerDescription rasterizerDesc = {};
    

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
        GetAssetFullPath(L"shaders.hlsl"),
        L"VSMain",
        PipelineStageType::Vertex
    };
    ShaderCreateDesription fragmentShaderDesc =
    {
        GetAssetFullPath(L"shaders.hlsl"),
        L"PSMain",
        PipelineStageType::Fragment
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

    BufferDescription bufferDesc = 
    {
        .elementsNum = 3,
        .elementStride = 28,
        .unstructuredSize = 84,
        .access = BufferAccess::Upload,
        .usage = BufferUsage::VertexBuffer,
        .flags = { .requiredCopyStateToInit = false }
    };

    m_buffer = allocator->CreateBuffer(bufferDesc);

    float aspectRatio = Application::WinApplication::GetWindow().GetAspectRatio();
    Vertex1 triangleVertices[] =
    {
        { { 0.0f, 0.25f * aspectRatio, 0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        { { 0.25f, -0.25f * aspectRatio, 0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.25f, -0.25f * aspectRatio, 0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };
    BufferRegionCopyDescription regionCopyDesc = {};
    regionCopyDesc.width = 84;
    m_buffer->UploadData(triangleVertices, regionCopyDesc);

    m_commandQueue->WaitUntilCompleted();

    m_commandBuffer->BeginRecording(m_commandQueue);
    m_commandBuffer->BindDescriptorsHeaps();
    m_commandBuffer->EndRecording();
    m_commandBuffer->SubmitToQueue(m_commandQueue);
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

        m_commandBuffer->BeginRecording(m_commandQueue);

        m_commandBuffer->SetViewport(m_viewportInfo);
        m_commandBuffer->SetScissors(m_scissorsRect);
        m_commandBuffer->SetPrimitiveTopology(PrimitiveTopology::TriangleList);

        auto view = Core::EntitiesPool::GetInstance().GetRegistry().view<Components::InstancedStaticMesh>();
        for (auto entity : view)
        {
            auto& meshComponent = view.get<Components::InstancedStaticMesh>(entity);
            auto& staticMesh = Assets::AssetsManager<Assets::StaticMesh>::GetInstance().GetAsset(meshComponent.staticMesh);
            auto& dynamicallyBoundResources = m_commandBuffer->GetCurrentRenderPipeline()->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources;
            dynamicallyBoundResources.SetBufferBindingResource("perMeshDataBufferIndex", staticMesh.GetRHIBufferForPerInstanceData());

            m_commandBuffer->BindRenderTargets();
            m_commandBuffer->BindPipelineResources();
            for (auto& submesh : staticMesh.m_submeshes)
            {
                submesh.UpdateRHIBufferWithPerInstanceData<Assets::MeshPerInstanceDataHandle>(m_commandBuffer);

                // Bind all buffers
                auto perInstancePerMeshDataBuffer = submesh.GetRHIBufferForPerInstanceData<Assets::MeshPerInstanceDataHandle>();

                auto bufferWithRegionDescription = submesh.GetPrimaryVertexData();
                m_commandBuffer->SetVertexBuffer(bufferWithRegionDescription.buffer, 0, bufferWithRegionDescription.regionDescription);

                bufferWithRegionDescription = submesh.GetSecondaryVertexData();
                m_commandBuffer->SetVertexBuffer(bufferWithRegionDescription.buffer, 1, bufferWithRegionDescription.regionDescription);

                bufferWithRegionDescription = submesh.GetIndicesData();
                m_commandBuffer->SetIndexBuffer(bufferWithRegionDescription.buffer, bufferWithRegionDescription.regionDescription);

                BufferRegionDescription bufferPerInstanceRegionDesc;
                bufferPerInstanceRegionDesc.offset = 0;
                bufferPerInstanceRegionDesc.size = perInstancePerMeshDataBuffer->GetSize();
                m_commandBuffer->SetVertexBuffer(perInstancePerMeshDataBuffer, 2, bufferPerInstanceRegionDesc);

                IndexedInstancedDrawInfo indexedInstancedDrawInfo = {};
                indexedInstancedDrawInfo.indicesPerInstanceNum = submesh.GetIndicesData().buffer->GetStructuredElementsNum();
                indexedInstancedDrawInfo.instancesNum = submesh.GetActiveInstancesNum<Assets::MeshPerInstanceDataHandle>();
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
