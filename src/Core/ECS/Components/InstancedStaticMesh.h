#pragma once
#include "../../../Assets/AssetsManager.h"
#include "../../../Assets/Mesh.h"

namespace Components
{
	struct InstancedStaticMesh
	{
		Assets::AssetsManager<Assets::StaticMesh>::AssetId staticMesh;
	};
}