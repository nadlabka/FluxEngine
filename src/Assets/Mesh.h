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
		void SetPerInstanceData(Core::Entity ent, const MeshPerInstanceData& data);
		void SetRHIBuffersForPerInstanceData(std::shared_ptr<RHI::IBuffer> uploadBuffer, std::shared_ptr<RHI::IBuffer> dataBuffer);
		void UpdateRHIBufferWithPerInstanceData(std::shared_ptr<RHI::ICommandBuffer> commandBuffer);

		std::vector<StaticSubmesh> m_submeshes;
		entt::storage<MeshPerInstanceData> m_meshPerInstanceData;
		BuffersWithDirtyIndices m_meshPerInstanceDataBuffers;
	};
}