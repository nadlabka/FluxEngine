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

    entt::storage<PointLightSourceData> perLightCachedMatrices;
    entt::storage<SpotLightSourceData> perLightCachedMatrices;
    entt::storage<DirectionalLightSourceData> perLightCachedMatrices;
};