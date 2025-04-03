#pragma once
#include <vector>
#include "Submesh.h"

namespace Assets
{
	struct MeshPerInstanceData
	{
		Matrix transform;
		Matrix inverseTransposeTransform;
	};

	struct StaticMesh
	{
		void CreatePerInstanceData(Core::Entity ent, const MeshPerInstanceData& data); //set up perMesh data for instance first, then procceed with submesh
		void UpdatePerInstanceData(Core::Entity ent, const MeshPerInstanceData& data);
		void UpdateRHIBufferWithPerInstanceData(std::shared_ptr<RHI::ICommandBuffer> commandBuffer);
		std::shared_ptr<RHI::IBuffer> GetRHIBufferForPerInstanceData() const;
		void DeleteInstance(Core::Entity ent);

		template <typename T, typename... Args>
		void CreateSubmeshPerInstanceData(Core::Entity ent, uint32_t submeshIndex, Args&&... args)
		{
			ASSERT(m_meshPerInstanceData.contains(ent), "PerMesh perInstance data has to be created first");
			m_submeshes[submeshIndex].CreatePerInstanceData<T>(ent, MeshPerInstanceDataHandle{ (uint32_t)m_meshPerInstanceData.index(ent) }, std::forward<Args>(args)...);
		}

		std::vector<StaticSubmesh> m_submeshes;
		entt::storage<MeshPerInstanceData> m_meshPerInstanceData;
		BuffersWithDirtyIndices m_meshPerInstanceDataBuffers;
	};
}