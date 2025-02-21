#pragma once
#include <vector>
#include "Submesh.h"

namespace Assets
{
	struct MeshPerInstanceData
	{
		Matrix transform;
	};

	struct StaticMesh
	{
		void AddPerInstanceData(entt::entity ent, const MeshPerInstanceData& data);

		std::vector<StaticSubmesh> m_submeshes;
		entt::storage<MeshPerInstanceData> m_meshPerInstanceData;
		BuffersWithDirtyIndices m_meshPerInstanceDataBuffers;
	};
}