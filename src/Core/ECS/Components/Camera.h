#pragma once
#include <D3D12Math.h>

namespace Components
{
	struct Camera
	{
        Vector3 position;
        Vector3 forward;
        Vector3 up;
        float fovY;
        float aspectRatio;
        float nearPlane; 
        float farPlane;
	};
}