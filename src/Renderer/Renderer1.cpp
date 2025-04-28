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
#include "Managers/MaterialsManager.h"

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

    TextureDescription hdrTargetDesc = {};
    hdrTargetDesc.usage = static_cast<TextureUsage>(eTextureUsage_ColorAttachment | eTextureUsage_Sampled);
    hdrTargetDesc.aspect = static_cast<TextureAspect>(eTextureAspect_HasColor);
    hdrTargetDesc.format = TextureFormat::RGBA16_FLOAT;
    hdrTargetDesc.type = TextureType::Texture2D;
    hdrTargetDesc.layout = TextureLayout::ColorAttachmentOptimal;
    hdrTargetDesc.width = window.GetWidth();
    hdrTargetDesc.height = window.GetHeight();
    hdrTargetDesc.clearColor = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_hdrTarget = rhiAllocator->CreateTexture(hdrTargetDesc);

    auto& constantBufferMgr = ConstantBufferManager::GetInstance();
    constantBufferMgr.RegisterBuffer<PerViewConstantBuffer>("PerView", m_commandBuffer);
    constantBufferMgr.RegisterBuffer<PerFrameConstantBuffer>("PerFrame", m_commandBuffer);

    auto& lightSourcesManager = LightSourcesManager::GetInstance();
    lightSourcesManager.InitLightsRHIBuffers(m_commandBuffer);
}

void Renderer1::Render()
{
    UpdatePipelineDynamicStates();
    PopulateCommandList();

    m_swapchain->Present();
}

void Renderer1::LoadPipeline()
{
    auto& materialsManager = MaterialsManager::GetInstance();
    materialsManager.SetRenderPipeline("ForwardPBR", materialsManager.CreateForwardPBRPipeline());
    materialsManager.SetRenderPipeline("PostProcess", materialsManager.CreatePostProcessPipeline());

    m_commandQueue->WaitUntilCompleted();
}

void Renderer1::WaitForGpu()
{
    m_commandQueue->WaitUntilCompleted();
}

void Renderer1::PopulateCommandList()
{
    auto& materialsManager = MaterialsManager::GetInstance();

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
        m_commandBuffer->BindRenderPipeline(materialsManager.GetRenderPipeline("ForwardPBR"));

        auto& constantBufferManager = ConstantBufferManager::GetInstance();
        auto& boundConstantBuffers = m_commandBuffer->GetCurrentRenderPipeline()->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_BoundConstantBuffers;
        boundConstantBuffers.SetConstantBufferBindingMapping("PerView", constantBufferManager.GetDataBufferByName("PerView"));  // u don't need to do it every frame
        constantBufferManager.UpdateBuffer<PerViewConstantBuffer>(m_commandQueue, m_commandBuffer, "PerView");
        boundConstantBuffers.SetConstantBufferBindingMapping("PerFrame", constantBufferManager.GetDataBufferByName("PerFrame"));  // u don't need to do it every frame
        constantBufferManager.UpdateBuffer<PerFrameConstantBuffer>(m_commandQueue, m_commandBuffer, "PerFrame");

        m_commandBuffer->BeginRecording(m_commandQueue);

        m_commandBuffer->SetViewport(m_viewportInfo);
        m_commandBuffer->SetScissors(m_scissorsRect);
        m_commandBuffer->SetPrimitiveTopology(PrimitiveTopology::TriangleList);

        m_commandBuffer->BindRenderTargets();

        auto& lightSourcesManager = LightSourcesManager::GetInstance();
        lightSourcesManager.UpdateLightsRHIBuffers(m_commandBuffer);

        auto& dynamicallyBoundResources = m_commandBuffer->GetCurrentRenderPipeline()->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources;    
        dynamicallyBoundResources.SetBufferBindingResource("pointLightsBufferIndex", lightSourcesManager.GetPointLightSRV()); // u don't need to do it every frame
        dynamicallyBoundResources.SetBufferBindingResource("spotLightsBufferIndex", lightSourcesManager.GetSpotLightSRV()); // u don't need to do it every frame
        dynamicallyBoundResources.SetBufferBindingResource("directionalLightsBufferIndex", lightSourcesManager.GetDirectionalLightSRV()); // u don't need to do it every frame

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

    m_commandBuffer->BindRenderPipeline(materialsManager.GetRenderPipeline("PostProcess"));
    m_commandBuffer->BeginRecording(m_commandQueue);

    m_commandBuffer->SetViewport(m_viewportInfo);
    m_commandBuffer->SetScissors(m_scissorsRect);
    m_commandBuffer->SetPrimitiveTopology(PrimitiveTopology::TriangleList);

    m_commandBuffer->BindRenderTargets();

    auto& dynamicallyBoundResources = m_commandBuffer->GetCurrentRenderPipeline()->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources;

    dynamicallyBoundResources.SetTextureBindingResource("hdrTextureIndex", m_hdrTarget);

    m_commandBuffer->BindPipelineResources();

    InstancedDrawInfo indexedInstancedDrawInfo = {};
    indexedInstancedDrawInfo.instancesNum = 1;
    indexedInstancedDrawInfo.verticesPerInstanceNum = 3;
    m_commandBuffer->DrawInstanced(indexedInstancedDrawInfo);

    m_commandBuffer->FinishRenderTargets();
    m_commandBuffer->EndRecording();
    m_commandBuffer->SubmitToQueue(m_commandQueue);
}

void Renderer1::Destroy()
{
    m_hdrTarget.reset();
    m_depthStencil.reset();
    m_buffer.reset();
    m_commandBuffer.reset();
    m_commandQueue.reset();
    m_swapchain.reset();
}

void Renderer1::UpdatePipelineDynamicStates()
{
    {
        std::vector<SubResourceRTsDescription> subresourceRTs = {};
        std::vector<SubResourceRTsDescription::TextureArraySliceToInclude> slicesToInclude = {};
        slicesToInclude.push_back({});
        subresourceRTs.push_back({ m_hdrTarget, slicesToInclude });
        SubResourceRTsDescription subresourceDST = {};
        subresourceDST.slicesToInclude.push_back({});
        subresourceDST.texture = m_depthStencil;

        auto& materialsManager = MaterialsManager::GetInstance();
        materialsManager.GetRenderPipeline("ForwardPBR")->GetPipelineDescription().renderPass->SetAttachments(subresourceRTs, subresourceDST);
    }

    {
        std::vector<SubResourceRTsDescription> subresourceRTs = {};
        std::vector<SubResourceRTsDescription::TextureArraySliceToInclude> slicesToInclude = {};
        slicesToInclude.push_back({});
        subresourceRTs.push_back({ m_swapchain->GetNextRenderTarget(), slicesToInclude });
        SubResourceRTsDescription subresourceDST = {};
        subresourceDST.slicesToInclude.push_back({});
        subresourceDST.texture = m_depthStencil;

        auto& materialsManager = MaterialsManager::GetInstance();
        materialsManager.GetRenderPipeline("PostProcess")->GetPipelineDescription().renderPass->SetAttachments(subresourceRTs, subresourceDST);
    }
}
