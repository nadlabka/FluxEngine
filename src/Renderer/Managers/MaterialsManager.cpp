#include "stdafx.h"
#include "MaterialsManager.h"
#include "../RHI/RHIContext.h"
#include "../../Assets/Submesh.h"

void MaterialsManager::Destroy()
{
    m_nameToRenderPipeline.clear();
    m_nameToComputePipeline.clear();
}

std::shared_ptr<RHI::IRenderPipeline> MaterialsManager::GetRenderPipeline(const std::string& name)
{
    auto it = m_nameToRenderPipeline.find(name);
    return it != m_nameToRenderPipeline.end() ? it->second : nullptr;
}

std::shared_ptr<RHI::IComputePipeline> MaterialsManager::GetComputePipeline(const std::string& name)
{
    auto it = m_nameToComputePipeline.find(name);
    return it != m_nameToComputePipeline.end() ? it->second : nullptr;
}

void MaterialsManager::SetRenderPipeline(const std::string& name, std::shared_ptr<RHI::IRenderPipeline> pipeline)
{
    if (!name.empty() && pipeline)
    {
        m_nameToRenderPipeline[name] = pipeline;
    }
}

void MaterialsManager::SetComputePipeline(const std::string& name, std::shared_ptr<RHI::IComputePipeline> pipeline)
{
    if (!name.empty() && pipeline)
    {
        m_nameToComputePipeline[name] = pipeline;
    }
}

void MaterialsManager::RemoveRenderPipeline(const std::string& name)
{
    m_nameToRenderPipeline.erase(name);
}

void MaterialsManager::RemoveComputePipeline(const std::string& name)
{
    m_nameToComputePipeline.erase(name);
}

std::shared_ptr<RHI::IRenderPipeline> MaterialsManager::CreateForwardPBRPipeline()
{
    using namespace RHI;

    auto& rhiContext = RHIContext::GetInstance();
    auto device = rhiContext.GetDevice();
    auto allocator = rhiContext.GetAllocator();

    RHI::InputAssemblerLayoutDescription inputAssemblerLayoutDesc;
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
    rtAttachmentDesc.format = TextureFormat::RGBA16_FLOAT;
    rtAttachmentDesc.finalLayout = TextureLayout::ReadOnlyOptimal;
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

    auto renderPipeline = device->CreateRenderPipeline(pipelineDesc);
    auto& dynamicallyBoundResources = renderPipeline->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources;

    dynamicallyBoundResources.SetBufferDescriptorResourceType("perMeshDataBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("perMeshDataBufferIndex", RHI::BindingVisibility::Vertex);

    dynamicallyBoundResources.SetBufferDescriptorResourceType("perInstancePerMeshHandleBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("perInstancePerMeshHandleBufferIndex", RHI::BindingVisibility::Vertex);

    dynamicallyBoundResources.SetBufferDescriptorResourceType("perInstanceMaterialParamsBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("perInstanceMaterialParamsBufferIndex", RHI::BindingVisibility::Vertex);

    auto& constantBuffer = renderPipeline->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_BoundConstantBuffers;
    constantBuffer.SetBufferBindingVisibility("PerView", RHI::BindingVisibility::All);
    constantBuffer.SetBufferBindingVisibility("PerFrame", RHI::BindingVisibility::All);

    dynamicallyBoundResources.SetBufferDescriptorResourceType("pointLightsBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("pointLightsBufferIndex", RHI::BindingVisibility::Fragment);

    dynamicallyBoundResources.SetBufferDescriptorResourceType("spotLightsBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("spotLightsBufferIndex", RHI::BindingVisibility::Fragment);

    dynamicallyBoundResources.SetBufferDescriptorResourceType("directionalLightsBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("directionalLightsBufferIndex", RHI::BindingVisibility::Fragment);

    SamplerDescription defaultSamplerDescription = {};
    defaultSamplerDescription.Filter = RHI::FilterMode::Linear;
    defaultSamplerDescription.AddressU = RHI::AddressMode::Wrap;
    defaultSamplerDescription.AddressV = RHI::AddressMode::Wrap;
    defaultSamplerDescription.AddressW = RHI::AddressMode::Wrap;
    defaultSamplerDescription.ComparisonFunc = RHI::SamplerComparisonFunc::Always;
    defaultSamplerDescription.MipLODBias = 0.0f;
    defaultSamplerDescription.MaxAnisotropy = 1;
    defaultSamplerDescription.BorderColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    defaultSamplerDescription.MinLOD = 0.0f;
    defaultSamplerDescription.MaxLOD = 1e10f;
    auto defaultSampler = device->CreateSampler(defaultSamplerDescription);
    dynamicallyBoundResources.SetSamplerToBinding("samplerDescriptorIndex", defaultSampler);

    SamplerDescription shadowSamplerDescription = {};
    shadowSamplerDescription.Filter = RHI::FilterMode::Linear;
    shadowSamplerDescription.AddressU = RHI::AddressMode::Wrap;
    shadowSamplerDescription.AddressV = RHI::AddressMode::Wrap;
    shadowSamplerDescription.AddressW = RHI::AddressMode::Wrap;
    shadowSamplerDescription.ComparisonFunc = RHI::SamplerComparisonFunc::Greater;
    shadowSamplerDescription.MipLODBias = 0.0f;
    shadowSamplerDescription.MaxAnisotropy = 1;
    shadowSamplerDescription.BorderColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    shadowSamplerDescription.MinLOD = 0.0f;
    shadowSamplerDescription.MaxLOD = 1e10f;
    auto shadowSampler = device->CreateSampler(shadowSamplerDescription);
    dynamicallyBoundResources.SetSamplerToBinding("shadowSamplerDescriptorIndex", shadowSampler);

    return renderPipeline;
}

std::shared_ptr<RHI::IRenderPipeline> MaterialsManager::CreateOpaqueDepthOnlyPipeline()
{
    using namespace RHI;

    auto& rhiContext = RHIContext::GetInstance();
    auto device = rhiContext.GetDevice();
    auto allocator = rhiContext.GetAllocator();

    RHI::InputAssemblerLayoutDescription inputAssemblerLayoutDesc;
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
    rasterizerDesc.windingOrder = WindingOrder::Counterclockwise;
    rasterizerDesc.depthBias = 
    {
            .clampValue = 0.0f,
            .constantFactor = -5.0f,
            .slopeScaledFactor = -5.0f,
            .enable = true
    };


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
    auto shaderCompiler = rhiContext.GetShaderCompiler();
    std::shared_ptr<IShader> vertexShader = shaderCompiler->CompileShader(vertexShaderDesc);
    pipelineStagesDesc.push_back({ vertexShader });

    PipelineLayoutBindings pipelineLayoutDesc = {};
    std::shared_ptr<IPipelineLayout> pipelineLayout = device->CreatePipelineLayout(pipelineStagesDesc);

    std::shared_ptr<IRenderPass> renderPass = {};
    std::vector<AttachmentDesc> colorAttachmentsDesc = {};
    std::optional<AttachmentDesc> depthStencilAttachmentDesc = {};
    AttachmentDesc dsAttachmentDesc = {};
    dsAttachmentDesc.format = TextureFormat::D32_FLOAT;
    dsAttachmentDesc.clearDepth = 0.0f;
    dsAttachmentDesc.clearStencil = 0.0f;
    dsAttachmentDesc.initialLayout = TextureLayout::DepthStencilAttachmentOptimal;
    dsAttachmentDesc.finalLayout = TextureLayout::ReadOnlyOptimal;
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

    auto renderPipeline = device->CreateRenderPipeline(pipelineDesc);
    auto& dynamicallyBoundResources = renderPipeline->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources;

    dynamicallyBoundResources.SetBufferDescriptorResourceType("perMeshDataBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("perMeshDataBufferIndex", RHI::BindingVisibility::Vertex);

    dynamicallyBoundResources.SetBufferDescriptorResourceType("perInstancePerMeshHandleBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("perInstancePerMeshHandleBufferIndex", RHI::BindingVisibility::Vertex);

    dynamicallyBoundResources.SetBufferDescriptorResourceType("perInstanceMaterialParamsBufferIndex", RHI::DescriptorResourceType::DataReadOnlyBuffer);
    dynamicallyBoundResources.SetBufferDescriptorVisibility("perInstanceMaterialParamsBufferIndex", RHI::BindingVisibility::Vertex);

    auto& constantBuffer = renderPipeline->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_BoundConstantBuffers;
    constantBuffer.SetBufferBindingVisibility("PerView", RHI::BindingVisibility::All);

    return renderPipeline;
}

std::shared_ptr<RHI::IComputePipeline> MaterialsManager::CreateMipGenerationPipeline()
{
    return std::shared_ptr<RHI::IComputePipeline>();
}

std::shared_ptr<RHI::IRenderPipeline> MaterialsManager::CreatePostProcessPipeline()
{
    using namespace RHI;

    auto& rhiContext = RHIContext::GetInstance();
    auto device = rhiContext.GetDevice();
    auto allocator = rhiContext.GetAllocator();

    InputAssemblerDescription inputAssemblerDesc = {};
    inputAssemblerDesc.primitiveTopology = PrimitiveTopology::TriangleList;

    RasterizerDescription rasterizerDesc = {};
    rasterizerDesc.windingOrder = WindingOrder::Clockwise;
    rasterizerDesc.cullMode = CullMode::None;

    ColorBlendDescription colorBlendDesc = {};
    ColorBlendDescription::ColorAttachmentBlendDesc colorAttachmentBlendDesc = {};
    colorBlendDesc.attachmentsBlends.push_back(colorAttachmentBlendDesc);

    DepthStencilDescription depthStencilDesc = {};
    depthStencilDesc.depthTestEnabled = false;
    depthStencilDesc.depthWriteEnabled = false;

    std::vector<PipelineStageDescription> pipelineStagesDesc = {};
    ShaderCreateDesription vertexShaderDesc =
    {
        "../../../../Assets/Shaders/Source/PostProcess.hlsl",
        L"VSMain",
        PipelineStageType::Vertex,
        "../../../../Assets/Shaders/PDB"
    };
    ShaderCreateDesription fragmentShaderDesc =
    {
        "../../../../Assets/Shaders/Source/PostProcess.hlsl",
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
    AttachmentDesc rtAttachmentDesc = {};
    rtAttachmentDesc.format = TextureFormat::RGBA8_UNORM;
    rtAttachmentDesc.clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    rtAttachmentDesc.initialLayout = TextureLayout::ColorAttachmentOptimal;
    rtAttachmentDesc.finalLayout = TextureLayout::Present;
    colorAttachmentsDesc.push_back(rtAttachmentDesc);

    RenderPassDesc renderPassDesc =
    {
        colorAttachmentsDesc,
        {}
    };
    renderPass = device->CreateRenderPass(renderPassDesc);

    RenderPipelineDescription pipelineDesc;
    pipelineDesc.inputAssembler = inputAssemblerDesc;
    pipelineDesc.rasterizer = rasterizerDesc;
    pipelineDesc.colorBlend = colorBlendDesc;
    pipelineDesc.depthStencil = depthStencilDesc;
    pipelineDesc.pipelineStages = pipelineStagesDesc;
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.renderPass = renderPass;

    auto renderPipeline = device->CreateRenderPipeline(pipelineDesc);

    auto& dynamicallyBoundResources = renderPipeline->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources;

    dynamicallyBoundResources.SetTextureDescriptorResourceType("hdrTextureIndex", RHI::DescriptorResourceType::SampledImage);
    dynamicallyBoundResources.SetTextureDescriptorVisibility("hdrTextureIndex", RHI::BindingVisibility::Fragment);

    SamplerDescription pointSamplerDescription = {};
    pointSamplerDescription.Filter = RHI::FilterMode::Nearest;
    pointSamplerDescription.AddressU = RHI::AddressMode::Wrap;
    pointSamplerDescription.AddressV = RHI::AddressMode::Wrap;
    pointSamplerDescription.AddressW = RHI::AddressMode::Wrap;
    pointSamplerDescription.ComparisonFunc = RHI::SamplerComparisonFunc::Always;
    pointSamplerDescription.MipLODBias = 0.0f;
    pointSamplerDescription.MaxAnisotropy = 1;
    pointSamplerDescription.BorderColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    pointSamplerDescription.MinLOD = 0.0f;
    pointSamplerDescription.MaxLOD = 1e10f;
    auto pointSampler = device->CreateSampler(pointSamplerDescription);
    dynamicallyBoundResources.SetSamplerToBinding("pointSamplerIndex", pointSampler);

    return renderPipeline;
}
