#include "stdafx.h"
#include "LightSourcesManager.h"
#include <ECS/Components/LightShadow.h>
#include "../RHI/D3D12/D3D12Texture.h"

void LightSourcesManager::Init()
{
    
}

void LightSourcesManager::Destroy()
{
    m_directionalLightBuffer.dataBuffer.reset();
    m_directionalLightBuffer.uploadBuffer.reset();

    m_pointLightBuffer.dataBuffer.reset();
    m_pointLightBuffer.uploadBuffer.reset();

    m_spotLightBuffer.dataBuffer.reset();
    m_spotLightBuffer.uploadBuffer.reset();
}

void LightSourcesManager::InitLightsRHIBuffers(std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
{
    m_pointLightBuffer.Resize(16, sizeof(PointLightSourceData), commandBuffer);
    m_spotLightBuffer.Resize(16, sizeof(SpotLightSourceData), commandBuffer);
    m_directionalLightBuffer.Resize(16, sizeof(DirectionalLightSourceData), commandBuffer);
}

void LightSourcesManager::UpdateLightsRHIBuffers(std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
{
    EnsureBufferCapacity(commandBuffer);

    // Update Point Lights
    for (auto dirtyIndex : m_pointLightBuffer.dirtyIndices)
    {
        RHI::BufferRegionCopyDescription regionDesc =
        {
            .srcOffset = sizeof(PointLightSourceData) * dirtyIndex,
            .destOffset = sizeof(PointLightSourceData) * dirtyIndex,
            .width = sizeof(PointLightSourceData)
        };
        m_pointLightBuffer.uploadBuffer->UploadData((void*)(*m_perPointLightSourceData.raw()), regionDesc);
        commandBuffer->CopyDataBetweenBuffers(m_pointLightBuffer.uploadBuffer, m_pointLightBuffer.dataBuffer, regionDesc);
    }
    m_pointLightBuffer.dirtyIndices.clear();
    m_pointLightBufferDirtyCache.clear();

    // Update Spot Lights
    for (auto dirtyIndex : m_spotLightBuffer.dirtyIndices)
    {
        RHI::BufferRegionCopyDescription regionDesc =
        {
            .srcOffset = sizeof(SpotLightSourceData) * dirtyIndex,
            .destOffset = sizeof(SpotLightSourceData) * dirtyIndex,
            .width = sizeof(SpotLightSourceData)
        };
        m_spotLightBuffer.uploadBuffer->UploadData((void*)(*m_perSpotLightSourceData.raw()), regionDesc);
        commandBuffer->CopyDataBetweenBuffers(m_spotLightBuffer.uploadBuffer, m_spotLightBuffer.dataBuffer, regionDesc);
    }
    m_spotLightBuffer.dirtyIndices.clear();
    m_spotLightBufferDirtyCache.clear();

    // Update Directional Lights
    for (auto dirtyIndex : m_directionalLightBuffer.dirtyIndices)
    {
        RHI::BufferRegionCopyDescription regionDesc =
        {
            .srcOffset = sizeof(DirectionalLightSourceData) * dirtyIndex,
            .destOffset = sizeof(DirectionalLightSourceData) * dirtyIndex,
            .width = sizeof(DirectionalLightSourceData)
        };
        m_directionalLightBuffer.uploadBuffer->UploadData((void*)(*m_perDirectionalLightSourceData.raw()), regionDesc);
        commandBuffer->CopyDataBetweenBuffers(m_directionalLightBuffer.uploadBuffer, m_directionalLightBuffer.dataBuffer, regionDesc);
    }
    m_directionalLightBuffer.dirtyIndices.clear();
    m_directionalLightBufferDirtyCache.clear();
}

void LightSourcesManager::AddPointLight(Core::Entity entity)
{
    ASSERT(!m_perPointLightSourceData.contains(entity), "Point light already exists for this entity");

    m_perPointLightSourceData.emplace<PointLightSourceData>(entity, {});
}

void LightSourcesManager::AddSpotLight(Core::Entity entity)
{
    ASSERT(!m_perSpotLightSourceData.contains(entity), "Spot light already exists for this entity");

    m_perSpotLightSourceData.emplace<SpotLightSourceData>(entity, {});
}

void LightSourcesManager::AddDirectionalLight(Core::Entity entity)
{
    ASSERT(!m_perDirectionalLightSourceData.contains(entity), "Directional light already exists for this entity");

    m_perDirectionalLightSourceData.emplace<DirectionalLightSourceData>(entity, {});
}

void LightSourcesManager::UpdatePointLightParams(Core::Entity entity, const Components::PointLight& pointLight)
{
    ASSERT(m_perPointLightSourceData.contains(entity), "Point light must exist to update params");

    auto& data = m_perPointLightSourceData.get(entity);
    data.color = Vector4(pointLight.color.x, pointLight.color.y, pointLight.color.z, pointLight.intensity);

    size_t index = m_perPointLightSourceData.index(entity);
    m_pointLightBuffer.dirtyIndices.push_back(index);
}

void LightSourcesManager::UpdateSpotLightParams(Core::Entity entity, const Components::SpotLight& spotLight)
{
    ASSERT(m_perSpotLightSourceData.contains(entity), "Spot light must exist to update params");

    auto& data = m_perSpotLightSourceData.get(entity);
    data.color = Vector4(spotLight.color.x, spotLight.color.y, spotLight.color.z, spotLight.intensity);
    data.innerConeCos = cosf(spotLight.innerCone);
    data.outerConeCos = cosf(spotLight.outerCone);

    size_t index = m_perSpotLightSourceData.index(entity);
    m_spotLightBuffer.dirtyIndices.push_back(index);
}

void LightSourcesManager::UpdateDirectionalLightParams(Core::Entity entity, const Components::DirectionalLight& dirLight)
{
    ASSERT(m_perDirectionalLightSourceData.contains(entity), "Directional light must exist to update params");

    auto& data = m_perDirectionalLightSourceData.get(entity);
    data.color = Vector4(dirLight.color.x, dirLight.color.y, dirLight.color.z, dirLight.irradiance);

    size_t index = m_perDirectionalLightSourceData.index(entity);
    m_directionalLightBuffer.dirtyIndices.push_back(index);
}

void LightSourcesManager::UpdatePointLightTransform(Core::Entity entity, const Matrix& transform)
{
    ASSERT(m_perPointLightSourceData.contains(entity), "Point light must exist to update transform");

    auto& data = m_perPointLightSourceData.get(entity);
    data.matrices.worldToLightClip = transform;
    data.matrices.worldToLightView = transform.Invert();
    data.position = transform.Translation();

    size_t index = m_perPointLightSourceData.index(entity);
    if (!m_pointLightBufferDirtyCache.contains(index))
    {
        m_pointLightBuffer.dirtyIndices.push_back(index);
        m_pointLightBufferDirtyCache.insert(index);
    }
}

void LightSourcesManager::UpdateSpotLightTransform(Core::Entity entity, const Matrix& transform)
{
    ASSERT(m_perSpotLightSourceData.contains(entity), "Spot light must exist to update transform");

    auto& data = m_perSpotLightSourceData.get(entity);
    data.matrices.worldToLightClip = transform;
    data.matrices.worldToLightView = transform.Invert();
    data.position = transform.Translation();
    data.direction = transform.Backward();
    data.direction.Normalize();

    size_t index = m_perSpotLightSourceData.index(entity);
    if(!m_spotLightBufferDirtyCache.contains(index))
    {
        m_spotLightBuffer.dirtyIndices.push_back(index);
        m_spotLightBufferDirtyCache.insert(index);
    }
}

void LightSourcesManager::UpdateDirectionalLightTransform(Core::Entity entity, const Matrix& transform)
{
    ASSERT(m_perDirectionalLightSourceData.contains(entity), "Directional light must exist to update transform");

    auto& data = m_perDirectionalLightSourceData.get(entity);
    data.matrices.worldToLightView = transform.Invert();
    data.matrices.worldToLightClip = data.matrices.worldToLightView * DirectX::XMMatrixOrthographicLH(
        1.0f,
        1.0f,
        LightSourcesManager::lightSourceFarPlane,
        LightSourcesManager::lightSourceNearPlane
    );
    data.direction = transform.Backward();
    data.direction.Normalize();

    size_t index = m_perDirectionalLightSourceData.index(entity);
    if (!m_directionalLightBufferDirtyCache.contains(index))
    {
        m_directionalLightBuffer.dirtyIndices.push_back(index);
        m_directionalLightBufferDirtyCache.insert(index);
    }
}

LightSourcesManager::PointLightSourceData& LightSourcesManager::GetPointLightData(Core::Entity entity)
{
    return m_perPointLightSourceData.get(entity);
}

LightSourcesManager::SpotLightSourceData& LightSourcesManager::GetSpotLightData(Core::Entity entity)
{
    return m_perSpotLightSourceData.get(entity);
}

LightSourcesManager::DirectionalLightSourceData& LightSourcesManager::GetDirectionalLightData(Core::Entity entity)
{
    return m_perDirectionalLightSourceData.get(entity);
}

void LightSourcesManager::CreateDirectionalLightShadowMap(Core::Entity entity)
{
    auto& rhiContext = RHI::RHIContext::GetInstance();
    auto allocator = rhiContext.GetAllocator();

    RHI::TextureDescription desc = {
        .usage = static_cast<RHI::TextureUsage>(RHI::TextureUsage::eTextureUsage_DepthStencilAttachment |
                                              RHI::TextureUsage::eTextureUsage_Sampled),
        .aspect = RHI::TextureAspect::eTextureAspect_HasDepth,
        .format = RHI::TextureFormat::D32_FLOAT,
        .type = RHI::TextureType::Texture2D,
        .layout = RHI::TextureLayout::DepthAttachmentOptimal,
        .width = 4096,
        .height = 4096,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1
    };

    auto texture = allocator->CreateTexture(desc);

    ASSERT(entity.HasComponent<Components::LightShadowmap>(), "NO SHADOWMAP COMPONENT HAS BEEN PROVIDED FOR THIS SOURCE");
    auto& component = entity.GetComponent<Components::LightShadowmap>();
    component.shadowmap = texture;

    ASSERT(m_perDirectionalLightSourceData.contains(entity), "Directional light must exist to update params");
    auto& data = m_perDirectionalLightSourceData.get(entity);

    data.shadowmapDescriptorIndex = std::static_pointer_cast<RHI::D3D12Texture>(texture)->m_SRVDescriptorIndex;
}

void LightSourcesManager::EnsureBufferCapacity(std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
{
    uint32_t pointLightBufferSize = m_pointLightBuffer.dataBuffer ? m_pointLightBuffer.dataBuffer->GetStructuredElementsNum() : 0;
    if (pointLightBufferSize < m_perPointLightSourceData.size())
    {
        m_pointLightBuffer.Resize(m_perPointLightSourceData.size() * 2, sizeof(PointLightSourceData), commandBuffer);
    }

    uint32_t spotLightBufferSize = m_spotLightBuffer.dataBuffer ? m_spotLightBuffer.dataBuffer->GetStructuredElementsNum() : 0;
    if (spotLightBufferSize < m_perSpotLightSourceData.size())
    {
        m_spotLightBuffer.Resize(m_perSpotLightSourceData.size() * 2, sizeof(SpotLightSourceData), commandBuffer);
    }

    uint32_t dirLightBufferSize = m_directionalLightBuffer.dataBuffer ? m_directionalLightBuffer.dataBuffer->GetStructuredElementsNum() : 0;
    if (dirLightBufferSize < m_perDirectionalLightSourceData.size())
    {
        m_directionalLightBuffer.Resize(m_perDirectionalLightSourceData.size() * 2, sizeof(DirectionalLightSourceData), commandBuffer);
    }
}
