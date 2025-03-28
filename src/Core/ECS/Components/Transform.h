#pragma once
#include <D3D12Math.h>

namespace Components
{
	struct Transform
	{
		Vector3 position;
		Vector3 rotationAngles;
		Vector3 scale;
	};

	struct TransformFlags
	{
		bool isDirty = false;
	};

	struct AccumulatedHierarchicalTransformMatrix
	{
		Matrix matrix;
	};
}