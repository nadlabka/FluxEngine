#pragma once
#include <SimpleMath.h>

namespace Assets
{
	using namespace DirectX::SimpleMath;

	struct VertexPrimaryAttributes
	{
		Vector3 position;
		Vector3 normals;
		Vector2 texCoords;
	};

	struct VertexSecondaryAttributes
	{
		Vector3 Tangent;
		Vector3 Bitangent;
	};
}