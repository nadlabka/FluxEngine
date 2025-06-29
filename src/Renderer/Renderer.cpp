#include "stdafx.h"
#include "RHI/RHIContext.h"
#include "Renderer.h"
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
#include <ECS/Components/LightShadow.h>
#include <FillPerViewBuffer.h>

void Renderer::Init()
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
    constantBufferMgr.RegisterBuffer<PerViewConstantBuffer>("PerViewShadow", m_commandBuffer);
    constantBufferMgr.RegisterBuffer<PerFrameConstantBuffer>("PerFrame", m_commandBuffer);

    auto& lightSourcesManager = LightSourcesManager::GetInstance();
    lightSourcesManager.InitLightsRHIBuffers(m_commandBuffer);
}

void Renderer::Render()
{
    UpdatePipelineDynamicStates();
    RecordAndSubmitCommandBuffers();

    m_swapchain->Present();
}

void Renderer::LoadPipeline()
{
    auto& materialsManager = MaterialsManager::GetInstance();
    materialsManager.SetRenderPipeline("OpaqueShadow", materialsManager.CreateOpaqueDepthOnlyPipeline());
    materialsManager.SetRenderPipeline("ForwardPBR", materialsManager.CreateForwardPBRPipeline());
    materialsManager.SetRenderPipeline("MaskedForwardPBR", materialsManager.CreateForwardMaskedPBRPipeline());
    materialsManager.SetRenderPipeline("PostProcess", materialsManager.CreatePostProcessPipeline());

    m_commandQueue->WaitUntilCompleted();
}

void Renderer::WaitForGpu()
{
    m_commandQueue->WaitUntilCompleted();
}

void Renderer::RecordAndSubmitCommandBuffers()
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
    auto& lightSourcesManager = LightSourcesManager::GetInstance();
    lightSourcesManager.UpdateLightsRHIBuffers(m_commandBuffer);

    m_commandBuffer->EndRecording();
    m_commandBuffer->SubmitToQueue(m_commandQueue);

    // one section per Material
    {
        m_commandBuffer->BindRenderPipeline(materialsManager.GetRenderPipeline("OpaqueShadow"));

        auto& constantBufferManager = ConstantBufferManager::GetInstance();
        auto& boundConstantBuffers = m_commandBuffer->GetCurrentRenderPipeline()->GetPipelineDescription().pipelineLayout->m_pipelineLayoutBindings.m_BoundConstantBuffers;

        auto view = Core::EntitiesPool::GetInstance().GetRegistry().view<Components::DirectionalLight, Components::LightShadowmap, Components::Transform>();
        for (auto directionalLight : view)
        {
            auto shadowmapTexture = view.get<Components::LightShadowmap>(directionalLight).shadowmap;

            auto& perViewBuffer = constantBufferManager.GetCpuBuffer<PerViewConstantBuffer>("PerViewShadow");
            FillShadowPerViewBuffer(perViewBuffer, (Core::Entity)directionalLight);

            boundConstantBuffers.SetConstantBufferBindingMapping("PerView", constantBufferManager.GetDataBufferByName("PerViewShadow"));
            constantBufferManager.UpdateBuffer<PerViewConstantBuffer>(m_commandQueue, m_commandBuffer, "PerViewShadow");

            std::vector<SubResourceRTsDescription> subresourceRTs = {};
            SubResourceRTsDescription subresourceDST = {};
            subresourceDST.slicesToInclude.push_back({});
            subresourceDST.texture = shadowmapTexture;

            m_commandBuffer->GetCurrentRenderPipeline()->GetPipelineDescription().renderPass->SetAttachments(subresourceRTs, subresourceDST);

            ScissorsRect scissorsRect = {
                0,
                0,
                shadowmapTexture->m_dimensionsInfo.m_width,
                shadowmapTexture->m_dimensionsInfo.m_height
            };

            ViewportInfo viewportInfo = {
                0.0f,
                0.0f,
                (float)shadowmapTexture->m_dimensionsInfo.m_width,
                (float)shadowmapTexture->m_dimensionsInfo.m_height,
                0.0f,
                1.0f
            };

            m_commandBuffer->BeginRecording(m_commandQueue);

            m_commandBuffer->SetViewport(viewportInfo);
            m_commandBuffer->SetScissors(scissorsRect);
            m_commandBuffer->SetPrimitiveTopology(PrimitiveTopology::TriangleList);

            m_commandBuffer->BindRenderTargets(true);

            auto& assetsManager = Assets::AssetsManager<Assets::StaticMesh>::GetInstance();
            for (auto& staticMesh : assetsManager.GetAssetsStorage().GetDataStorage())
            {
                m_commandBuffer->SetBindingResource("perMeshDataBufferIndex", staticMesh.GetRHIBufferForPerInstanceData());

                for (auto& submesh : staticMesh.m_submeshes)
                {
                    if (submesh.GetActiveInstancesNum<MaterialParameters::PBRMaterial>() == 0) { continue; }

                    submesh.UpdateRHIBuffersWithPerInstanceData<MaterialParameters::PBRMaterial>(m_commandBuffer);

                    // Bind all buffers
                    auto perInstancePerMeshDataBuffer = submesh.GetRHIBufferForPerMeshData<MaterialParameters::PBRMaterial>();
                    auto perInstancePerMaterialDataBuffer = submesh.GetRHIBufferForPerMaterialData<MaterialParameters::PBRMaterial>();

                    m_commandBuffer->SetBindingResource("perInstancePerMeshHandleBufferIndex", perInstancePerMeshDataBuffer);
                    m_commandBuffer->SetBindingResource("perInstanceMaterialParamsBufferIndex", perInstancePerMaterialDataBuffer);

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
    }
    {
        m_commandBuffer->BindRenderPipeline(materialsManager.GetRenderPipeline("ForwardPBR"));

        auto& constantBufferManager = ConstantBufferManager::GetInstance();
        m_commandBuffer->SetConstantBufferBindingMapping("PerView", constantBufferManager.GetDataBufferByName("PerView"));  // u don't need to do it every frame
        constantBufferManager.UpdateBuffer<PerViewConstantBuffer>(m_commandQueue, m_commandBuffer, "PerView");
        m_commandBuffer->SetConstantBufferBindingMapping("PerFrame", constantBufferManager.GetDataBufferByName("PerFrame"));  // u don't need to do it every frame
        constantBufferManager.UpdateBuffer<PerFrameConstantBuffer>(m_commandQueue, m_commandBuffer, "PerFrame");

        m_commandBuffer->BeginRecording(m_commandQueue);

        m_commandBuffer->SetViewport(m_viewportInfo);
        m_commandBuffer->SetScissors(m_scissorsRect);
        m_commandBuffer->SetPrimitiveTopology(PrimitiveTopology::TriangleList);

        m_commandBuffer->BindRenderTargets(true);
 
        m_commandBuffer->SetBindingResource("pointLightsBufferIndex", lightSourcesManager.GetPointLightSRV()); // u don't need to do it every frame
        m_commandBuffer->SetBindingResource("spotLightsBufferIndex", lightSourcesManager.GetSpotLightSRV()); // u don't need to do it every frame
        m_commandBuffer->SetBindingResource("directionalLightsBufferIndex", lightSourcesManager.GetDirectionalLightSRV()); // u don't need to do it every frame

        auto& assetsManager = Assets::AssetsManager<Assets::StaticMesh>::GetInstance();
        for (auto& staticMesh : assetsManager.GetAssetsStorage().GetDataStorage())
        {
            m_commandBuffer->SetBindingResource("perMeshDataBufferIndex", staticMesh.GetRHIBufferForPerInstanceData());
            
            for (auto& submesh : staticMesh.m_submeshes)
            {
                if (submesh.GetActiveInstancesNum<MaterialParameters::PBRMaterial>() == 0) { continue; }

                submesh.UpdateRHIBuffersWithPerInstanceData<MaterialParameters::PBRMaterial>(m_commandBuffer);

                // Bind all buffers
                auto perInstancePerMeshDataBuffer = submesh.GetRHIBufferForPerMeshData<MaterialParameters::PBRMaterial>();
                auto perInstancePerMaterialDataBuffer = submesh.GetRHIBufferForPerMaterialData<MaterialParameters::PBRMaterial>();

                m_commandBuffer->SetBindingResource("perInstancePerMeshHandleBufferIndex", perInstancePerMeshDataBuffer);
                m_commandBuffer->SetBindingResource("perInstanceMaterialParamsBufferIndex", perInstancePerMaterialDataBuffer);

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
        m_commandBuffer->BindRenderPipeline(materialsManager.GetRenderPipeline("MaskedForwardPBR"));

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

        m_commandBuffer->BindRenderTargets(false);

        m_commandBuffer->SetBindingResource("pointLightsBufferIndex", lightSourcesManager.GetPointLightSRV()); // u don't need to do it every frame
        m_commandBuffer->SetBindingResource("spotLightsBufferIndex", lightSourcesManager.GetSpotLightSRV()); // u don't need to do it every frame
        m_commandBuffer->SetBindingResource("directionalLightsBufferIndex", lightSourcesManager.GetDirectionalLightSRV()); // u don't need to do it every frame

        auto& assetsManager = Assets::AssetsManager<Assets::StaticMesh>::GetInstance();
        for (auto& staticMesh : assetsManager.GetAssetsStorage().GetDataStorage())
        {
            m_commandBuffer->SetBindingResource("perMeshDataBufferIndex", staticMesh.GetRHIBufferForPerInstanceData());

            for (auto& submesh : staticMesh.m_submeshes)
            {
                if (submesh.GetActiveInstancesNum<MaterialParameters::MaskedPBRMaterial>() == 0) { continue; }

                submesh.UpdateRHIBuffersWithPerInstanceData<MaterialParameters::MaskedPBRMaterial>(m_commandBuffer);

                // Bind all buffers
                auto perInstancePerMeshDataBuffer = submesh.GetRHIBufferForPerMeshData<MaterialParameters::MaskedPBRMaterial>();
                auto perInstancePerMaterialDataBuffer = submesh.GetRHIBufferForPerMaterialData<MaterialParameters::MaskedPBRMaterial>();

                m_commandBuffer->SetBindingResource("perInstancePerMeshHandleBufferIndex", perInstancePerMeshDataBuffer);
                m_commandBuffer->SetBindingResource("perInstanceMaterialParamsBufferIndex", perInstancePerMaterialDataBuffer);

                m_commandBuffer->BindPipelineResources();

                auto bufferWithRegionDescription = submesh.GetPrimaryVertexData();
                m_commandBuffer->SetVertexBuffer(bufferWithRegionDescription.buffer, 0, bufferWithRegionDescription.regionDescription);

                bufferWithRegionDescription = submesh.GetSecondaryVertexData();
                m_commandBuffer->SetVertexBuffer(bufferWithRegionDescription.buffer, 1, bufferWithRegionDescription.regionDescription);

                bufferWithRegionDescription = submesh.GetIndicesData();
                m_commandBuffer->SetIndexBuffer(bufferWithRegionDescription.buffer, bufferWithRegionDescription.regionDescription);

                IndexedInstancedDrawInfo indexedInstancedDrawInfo = {};
                indexedInstancedDrawInfo.indicesPerInstanceNum = submesh.GetIndicesData().buffer->GetStructuredElementsNum();
                indexedInstancedDrawInfo.instancesNum = submesh.GetActiveInstancesNum<MaterialParameters::MaskedPBRMaterial>();
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

    m_commandBuffer->BindRenderTargets(true);

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

void Renderer::Destroy()
{
    m_hdrTarget.reset();
    m_depthStencil.reset();
    m_buffer.reset();
    m_commandBuffer.reset();
    m_commandQueue.reset();
    m_swapchain.reset();
}

void Renderer::UpdatePipelineDynamicStates()
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
        materialsManager.GetRenderPipeline("MaskedForwardPBR")->GetPipelineDescription().renderPass->SetAttachments(subresourceRTs, subresourceDST);
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
