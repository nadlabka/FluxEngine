#pragma once
#include <D3D12Math.h>

namespace Components
{
	struct PointLightSource
	{
		Vector3 position;
		float intensity;
		Vector3 color;
		float radius;
	};
}