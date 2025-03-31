#include "stdafx.h"
#include "LightSourcesManager.h"

void LightSourcesManager::Init()
{

}

void LightSourcesManager::Destroy()
{

}

void LightSourcesManager::UpdateLightsRHIBuffers(std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
{

}

void LightSourcesManager::AddPointLight(Core::Entity entity)
{

}

void LightSourcesManager::AddSpotLight(Core::Entity entity)
{

}

void LightSourcesManager::AddDirectionalLight(Core::Entity entity)
{

}

void LightSourcesManager::UpdatePointLightParams(Core::Entity entity, const Components::PointLight& pointLight)
{

}

void LightSourcesManager::UpdateSpotLightParams(Core::Entity entity, const Components::SpotLight& spotLight)
{

}

void LightSourcesManager::UpdateDirectionalLightParams(Core::Entity entity, const Components::DirectionalLight& dirLight)
{

}

void LightSourcesManager::UpdatePointLightTransform(Core::Entity entity, const Matrix& transform)
{

}

void LightSourcesManager::UpdateSpotLightTransform(Core::Entity entity, const Matrix& transform)
{

}

void LightSourcesManager::UpdateDirectionalLightTransform(Core::Entity entity, const Matrix& transform)
{

}

void LightSourcesManager::EnsureBufferCapacity(std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
{

}
