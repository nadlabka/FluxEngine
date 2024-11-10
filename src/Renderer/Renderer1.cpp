#include "stdafx.h"
#include "RHI/RHIContext.h"
#include "Renderer1.h"
#include <DXSampleHelper.h>
#include "../Application/WinAPI/WinApplication.h"
#include "../Application/WinAPI/WinWindow.h"
#include "RHI/D3D12/D3D12Shader.h"
#include <filesystem>

void Renderer1::Init()
{
    auto& rhiContext = RHIContext::GetInstance();

    AdapterCreateDesc adapterCreateDesc;
    adapterCreateDesc.useHighPerformanceAdapter = true;
    adapterCreateDesc.useWarpDevice = false;
    DeviceCreateDesc deviceCreateDesc;

    rhiContext.Init(ERHIRenderingAPI::D3D12, adapterCreateDesc, deviceCreateDesc);

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
        3,
        28,
        84,
        BufferAccess::Upload,
        BufferUsage::VertexBuffer,
        { false }
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

    m_commandBuffer->BeginRecording();

    m_commandBuffer->SetViewport(m_viewportInfo);
    m_commandBuffer->SetScissors(m_scissorsRect);
    m_commandBuffer->SetPrimitiveTopology(PrimitiveTopology::TriangleList);

    BufferBindDescription bufferbindDesc;
    bufferbindDesc.offset = 0;
    bufferbindDesc.size = m_buffer->GetSize();
    m_commandBuffer->SetVertexBuffer(m_buffer, 0, bufferbindDesc);

    InstancedDrawInfo instancedDrawInfo = {};
    instancedDrawInfo.verticesPerInstanceNum = 3;
    m_commandBuffer->DrawInstanced(instancedDrawInfo);

    m_commandBuffer->EndRecording();
}
