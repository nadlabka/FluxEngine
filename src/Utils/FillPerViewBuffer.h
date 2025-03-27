#pragma once
#include "../Renderer/DataTypes/PerViewConstantBuffer.h"
#include <ECS/Components/Transform.h>
#include <ECS/Components/Camera.h>


void FillPerViewBuffer(PerViewConstantBuffer& buffer, const Components::Transform transformComponent, const Components::Camera cameraComponent, float viewportWidth, float viewportHeight)
{
    buffer.cameraPosition = XMVectorSetW(transformComponent.position, 1.0f);

    buffer.cameraDirection = XMVectorSetW(cameraComponent.forward, 0.0f);

    buffer.nearPlane = cameraComponent.nearPlane;
    buffer.farPlane = cameraComponent.farPlane;

    buffer.viewportWidth = viewportWidth;
    buffer.viewportHeight = viewportHeight;

    Vector4 eye = Vector4(transformComponent.position.x, transformComponent.position.y, transformComponent.position.z, 1.0f);
    Vector4 at = eye + cameraComponent.forward;
    Vector4 up = Vector4(cameraComponent.up.x, cameraComponent.up.y, cameraComponent.up.z, 1.0f);
    buffer.viewMatrix = XMMatrixLookAtLH(eye, at, up);

    buffer.projectionMatrix = Matrix::CreatePerspectiveFieldOfView(
        cameraComponent.fovY,
        cameraComponent.aspectRatio,
        cameraComponent.nearPlane,
        cameraComponent.farPlane
    );

    // Compute view-projection matrix
    buffer.viewProjectionMatrix = XMMatrixMultiply(buffer.viewMatrix, buffer.projectionMatrix);
}