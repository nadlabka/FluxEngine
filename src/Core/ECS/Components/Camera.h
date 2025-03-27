#pragma once
#include <D3D12Math.h>

namespace Components
{
	struct Camera
	{
        float fovY;
        float aspectRatio;
        float nearPlane; 
        float farPlane;

        Vector3 forward;
        Vector3 right;
        Vector3 up;
	};

    struct CameraControl
    {
        float sensetivity;
        bool isRotating;
        float speed;
    };
}