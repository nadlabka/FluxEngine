#pragma once
#include "../Renderer/DataTypes/PerViewConstantBuffer.h"
#include <ECS/Components/Transform.h>
#include <ECS/Components/Camera.h>
#include <../Renderer/Managers/LightSourcesManager.h>


static void FillPerViewBuffer(PerViewConstantBuffer& buffer, const Components::Transform& transformComponent, const Components::Camera& cameraComponent, float viewportWidth, float viewportHeight)
{
    buffer.cameraPosition = Vector4(transformComponent.position.x, transformComponent.position.y, transformComponent.position.z, 1.0f);

    buffer.cameraDirection = Vector4(cameraComponent.forward.x, cameraComponent.forward.y, cameraComponent.forward.z, 0.0f);

    buffer.nearPlane = cameraComponent.nearPlane;
    buffer.farPlane = cameraComponent.farPlane;

    buffer.viewportWidth = viewportWidth;
    buffer.viewportHeight = viewportHeight;

    Vector4 eye = Vector4(transformComponent.position.x, transformComponent.position.y, transformComponent.position.z, 1.0f);
    Vector4 at = eye + cameraComponent.forward;
    Vector4 up = Vector4(cameraComponent.up.x, cameraComponent.up.y, cameraComponent.up.z, 1.0f);
    buffer.viewMatrix = XMMatrixLookAtLH(eye, at, up);

    buffer.projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(cameraComponent.fovY),
        cameraComponent.aspectRatio,
        cameraComponent.farPlane,
        cameraComponent.nearPlane
    );

    buffer.viewProjectionMatrix = XMMatrixMultiply(buffer.viewMatrix, buffer.projectionMatrix);
}

static void FillShadowPerViewBuffer(PerViewConstantBuffer& buffer, Core::Entity entity)
{
    auto& lightSourcesManager = LightSourcesManager::GetInstance();

    const auto& dirLightData = lightSourcesManager.GetDirectionalLightData(entity);
    const auto& transform = entity.GetComponent<Components::Transform>();

    buffer.cameraDirection = Vector4(dirLightData.direction.x, dirLightData.direction.y, dirLightData.direction.z, 0.0f);
    buffer.cameraPosition = Vector4(transform.position.x, transform.position.y, transform.position.z, 1.0f);

    buffer.viewMatrix = dirLightData.matrices.worldToLightView;

    buffer.projectionMatrix = DirectX::XMMatrixOrthographicLH(
        1.0f,
        1.0f,
        LightSourcesManager::lightSourceFarPlane,
        LightSourcesManager::lightSourceNearPlane
    );

    buffer.viewProjectionMatrix = dirLightData.matrices.worldToLightClip;

    // TODO: Add padding to avoid flickering
}