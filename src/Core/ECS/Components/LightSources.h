#pragma once
#include <D3D12Math.h>

namespace Components
{
	struct PointLight
	{
		Vector3 color;
		float intensity;
	};

	struct SpotLight
	{
		Vector3 color;
		float intensity;
		float innerCone;
		float outerCone;
	};

	struct DirectionalLight
	{
		Vector3 color;
		float irradiance;
	};
}