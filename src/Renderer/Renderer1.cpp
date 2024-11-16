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

void Renderer1::Init()
{
    auto& rhiContext = RHIContext::GetInstance();
    auto factory = rhiContext.GetFactory();
    auto device = rhiContext.GetDevice();
     
    auto& window = WinApplication::GetWindow();
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

    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));
    m_assetsPath = assetsPath;
}

void Renderer1::Render()
{
    UpdatePipelineDynamicStates();
    PopulateCommandList();

    m_commandBuffer->SubmitToQueue(m_commandQueue);

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
            28,
            0,
            BindingInputRate::PerVertex
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
            VertexAttributeFormat::R32G32B32A32_SignedFloat,
            "COLOR",
            0
        });


    InputAssemblerDescription inputAssemblerDesc = {};
    inputAssemblerDesc.primitiveTopology = PrimitiveTopology::TriangleList;


    RasterizerDescription rasterizerDesc = {};
    

    ColorBlendDescription colorBlendDesc = {};
    ColorBlendDescription::ColorAttachmentBlendDesc colorAttachmentBlendDesc = {};
    colorBlendDesc.attachmentsBlends.push_back(colorAttachmentBlendDesc);


    DepthStencilDescription depthStencilDesc = {};


    std::vector<PipelineStageDescription> pipelineStagesDesc = {};
    ShaderCreateDesription vertexShaderDesc =
    {
        GetAssetFullPath(L"shaders.hlsl"),
        L"VSMain",
        PipelineStage::Vertex
    };
    ShaderCreateDesription fragmentShaderDesc =
    {
        GetAssetFullPath(L"shaders.hlsl"),
        L"PSMain",
        PipelineStage::Fragment
    };
    std::shared_ptr<IShader> vertexShader = std::make_shared<D3D12Shader>(vertexShaderDesc);
    std::shared_ptr<IShader> fragmentShader = std::make_shared<D3D12Shader>(fragmentShaderDesc);
    pipelineStagesDesc.push_back({ vertexShader });
    pipelineStagesDesc.push_back({ fragmentShader });


    PipelineLayoutDescription pipelineLayoutDesc = {};
    std::shared_ptr<IPipelineLayout> pipelineLayout = device->CreatePipelineLayout(pipelineLayoutDesc);


    std::shared_ptr<IRenderPass> renderPass = {};
    std::vector<AttachmentDesc> colorAttachmentsDesc = {};
    std::optional<AttachmentDesc> depthStencilAttachmentDesc = {};
    AttachmentDesc rtAttachmentDesc = {};
    rtAttachmentDesc.clearColor = { 0.0f, 0.2f, 0.4f, 1.0f };
    colorAttachmentsDesc.push_back(rtAttachmentDesc);
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

    float aspectRatio = WinApplication::GetWindow().GetAspectRatio();
    Vertex1 triangleVertices[] =
    {
        { { 0.0f, 0.25f * aspectRatio, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        { { 0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };
    BufferRegionCopyDescription regionCopyDesc = {};
    regionCopyDesc.width = 84;
    m_buffer->UploadData(triangleVertices, regionCopyDesc);

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

void Renderer1::Destroy()
{
    m_swapchain.reset();
    m_commandQueue.reset();
    m_commandBuffer.reset();
    m_renderPipeline.reset();
    m_buffer.reset();
}

void Renderer1::UpdatePipelineDynamicStates()
{
    std::vector<SubResourceRTsDescription> subresourceRTs = {};
    std::vector<SubResourceRTsDescription::TextureArraySliceToInclude> slicesToInclude = {};
    slicesToInclude.push_back({});
    subresourceRTs.push_back({ m_swapchain->GetNextRenderTarget(), slicesToInclude });
    m_renderPipeline->GetPipelineDescription().renderPass->SetAttachments(subresourceRTs, nullptr);
}
