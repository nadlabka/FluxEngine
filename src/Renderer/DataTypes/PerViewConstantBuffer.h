#pragma once
#include <D3D12Math.h>

struct PerViewConstantBuffer
{
    Matrix viewMatrix;
    Matrix projectionMatrix;
    Matrix viewProjectionMatrix;
    Vector4 cameraPosition;
    Vector4 cameraDirection;

    float nearPlane;
    float farPlane;

    float viewportWidth;
    float viewportHeight;

    float padding[2]; // Padding to ensure 16-byte alignment
};