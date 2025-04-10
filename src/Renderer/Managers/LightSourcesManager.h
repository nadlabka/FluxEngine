#pragma once
#include <memory>
#include <ECS/Components/LightSources.h>
#include <ECS/Entity.h>
#include <../Renderer/RHI/RHIContext.h>
#include "../DataTypes/BuffersPair.h"

class LightSourcesManager
{
public:
    struct PerLightMatrices
    {
        Matrix worldToLight;
        Matrix lightToWorld;
    };

    struct PointLightSourceData
    {
        PerLightMatrices matrices;
        Vector4 color;       // w contains intensity/irradiance
        Vector3 position;
        float padding;
    };

    struct SpotLightSourceData
    {
        PerLightMatrices matrices;
        Vector3 position;
        float innerConeCos;
        Vector4 color;
        Vector3 direction;
        float outerConeCos;
    };

    struct DirectionalLightSourceData
    {
        PerLightMatrices matrices;
        Vector4 color;       // w contains intensity/irradiance
        Vector3 direction;
        float padding;
    };

    static LightSourcesManager& GetInstance()
    {
        static LightSourcesManager instance;
        return instance;
    }

    LightSourcesManager(const LightSourcesManager&) = delete;
    LightSourcesManager& operator=(const LightSourcesManager&) = delete;

    void Init();
    void Destroy();
    void InitLightsRHIBuffers(std::shared_ptr<RHI::ICommandBuffer> commandBuffer);
    void UpdateLightsRHIBuffers(std::shared_ptr<RHI::ICommandBuffer> commandBuffer);

    void AddPointLight(Core::Entity entity);
    void AddSpotLight(Core::Entity entity);
    void AddDirectionalLight(Core::Entity entity);

    void UpdatePointLightParams(Core::Entity entity, const Components::PointLight& pointLight);
    void UpdateSpotLightParams(Core::Entity entity, const Components::SpotLight& spotLight);
    void UpdateDirectionalLightParams(Core::Entity entity, const Components::DirectionalLight& dirLight);

    void UpdatePointLightTransform(Core::Entity entity, const Matrix& transform);
    void UpdateSpotLightTransform(Core::Entity entity, const Matrix& transform);
    void UpdateDirectionalLightTransform(Core::Entity entity, const Matrix& transform);

    uint32_t GetPointLightsNum() { return perPointLightSourceData.size(); }
    uint32_t GetSpotLightsNum() { return perSpotLightSourceData.size(); }
    uint32_t GetDirectionalLightsNum() { return perDirectionalLightSourceData.size(); }

    std::shared_ptr<RHI::IBuffer> GetPointLightSRV() const { return m_pointLightBuffer.dataBuffer; }
    std::shared_ptr<RHI::IBuffer> GetSpotLightSRV() const { return m_spotLightBuffer.dataBuffer; }
    std::shared_ptr<RHI::IBuffer> GetDirectionalLightSRV() const { return m_directionalLightBuffer.dataBuffer; }

private:
    LightSourcesManager() {}
    ~LightSourcesManager() {}

    void EnsureBufferCapacity(std::shared_ptr<RHI::ICommandBuffer> commandBuffer);

    BuffersWithDirtyIndices m_pointLightBuffer;
    BuffersWithDirtyIndices m_spotLightBuffer;
    BuffersWithDirtyIndices m_directionalLightBuffer;

    std::unordered_set<uint32_t> m_pointLightBufferDirtyCache;
    std::unordered_set<uint32_t> m_spotLightBufferDirtyCache;
    std::unordered_set<uint32_t> m_directionalLightBufferDirtyCache;

    entt::storage<PointLightSourceData> perPointLightSourceData;
    entt::storage<SpotLightSourceData> perSpotLightSourceData;
    entt::storage<DirectionalLightSourceData> perDirectionalLightSourceData;
};