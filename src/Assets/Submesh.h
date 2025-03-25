#pragma once
#include <memory>
#include <../Renderer/RHI/RHIContext.h>
#include "Material.h"
#include "AssetsManager.h"
#include "Vertex.h"
#include <entt/entt.hpp>
#include <ECS/Entity.h>
#include "../Renderer/DataTypes/BuffersPair.h"

namespace Assets
{
	struct MeshPerInstanceDataHandle
	{
		uint32_t indexInPool;
	};

	struct PerMaterialContextData
	{
		BuffersWithDirtyIndices materialParamsBuffers;

		entt::storage<MeshPerInstanceDataHandle> perMeshData;
		BuffersWithDirtyIndices perMeshBuffers;
	};

	class StaticSubmesh
	{
	public:
		StaticSubmesh() = default;
		StaticSubmesh(RHI::BufferWithRegionDescription primaryVertexData, RHI::BufferWithRegionDescription secondaryVertexData, RHI::BufferWithRegionDescription indicesData);

		template <typename T, typename... Args>
		void CreatePerInstanceData(Core::Entity entity, const MeshPerInstanceDataHandle& perMeshHandle, Args&&... args) // we need to delete all requested instances first and insert only after this
		{
			ASSERT(!m_registry.valid(entity), "Instance for this entity has already been created");
			m_registry.create(entity);

			auto* perMaterialContextData = m_registry.ctx().find<PerMaterialContextData>(entt::type_id<T>().hash());
			if (!perMaterialContextData)
			{
				perMaterialContextData = &m_registry.ctx().insert_or_assign<PerMaterialContextData>(entt::type_id<T>().hash(), PerMaterialContextData{});
			}

			perMaterialContextData->materialParamsBuffers.dirtyIndices.push_back(m_registry.storage<T>().size());
			perMaterialContextData->perMeshBuffers.dirtyIndices.push_back(perMaterialContextData->perMeshData.size());

			perMaterialContextData->perMeshData.emplace(entity, perMeshHandle);

			m_registry.emplace<T>(entity, std::forward<Args>(args)...);
		}

		template <typename T, typename... Args>
		void UpdatePerInstancePerMaterialData(Core::Entity entity, Args&&... args)
		{
			ASSERT(m_registry.valid(entity) && m_registry.any_of<T>(entity), "PerInstance data has to be set first before being updated");
			m_registry.replace<T>(entity, args);
			PerMaterialContextData& perMaterialContextData = *m_registry.ctx().find<PerMaterialContextData>(entt::type_id<T>().hash());
			perMaterialContextData.materialParamsBuffers.dirtyIndices.push_back(m_registry.storage<T>().index(entity));
		}

		template <typename T>
		void UpdatePerInstancePerMeshDataHandle(Core::Entity entity, const MeshPerInstanceDataHandle& perMeshHandle)
		{
			ASSERT(m_registry.valid(entity) && m_registry.ctx().contains(entt::type_id<T>().hash()), "PerInstance data has to be set first before being updated");
			PerMaterialContextData& perMaterialContextData = *m_registry.ctx().find<PerMaterialContextData>(entt::type_id<T>().hash());
			perMaterialContextData.perMeshData.replace(entity, perMeshHandle);
			perMaterialContextData.perMeshBuffers.dirtyIndices.push_back(perMaterialContextData.perMeshData.index(entity));
		}

		void DestroyEntityReference(Core::Entity entity, Core::Entity swappedMesh, const MeshPerInstanceDataHandle& perMeshHandle) // we need to delete all requested instances first and insert only after this
		{
			for (auto&& typedStorage : m_registry.storage())
			{
				auto storageType = typedStorage.first;
				auto& storage = typedStorage.second;

				ASSERT(m_registry.ctx().find<PerMaterialContextData>(storageType), "For every storage type one PerMaterialContextData has to exist");

				PerMaterialContextData& perMaterialContextData = *m_registry.ctx().find<PerMaterialContextData>(storageType);

				if (!storage.contains(swappedMesh))
				{
					perMaterialContextData.perMeshData.get(swappedMesh) = perMeshHandle;
				}

				if (storage.contains(entity))
				{
					size_t indexInPool = storage.index(entity);

					perMaterialContextData.materialParamsBuffers.dirtyIndices.push_back(indexInPool);
					perMaterialContextData.perMeshBuffers.dirtyIndices.push_back(indexInPool);

					perMaterialContextData.perMeshData.remove(entity);
				}
			}
			m_registry.destroy(entity);
		}

		template<typename T>
		void UpdateRHIBuffersWithPerInstanceData(std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
		{
			auto& storage = m_registry.storage<T>();
			PerMaterialContextData& perMaterialContextData = *m_registry.ctx().find<PerMaterialContextData>(entt::type_id<T>().hash());

			EnsureBufferCapacity<T>(perMaterialContextData, storage.size(), commandBuffer);
			for (auto& dirtyIndex : perMaterialContextData.materialParamsBuffers.dirtyIndices)
			{
				RHI::BufferRegionCopyDescription regionDesc =
				{
					.srcOffset = sizeof(T) * dirtyIndex,
					.destOffset = sizeof(T) * dirtyIndex,
					.width = sizeof(T)
				};
				perMaterialContextData.materialParamsBuffers.uploadBuffer->UploadData((void*)*storage.raw(), regionDesc); // maybe upload as contiguous data to improve cache hits on gpu when copying 1-by-1
				commandBuffer->CopyDataBetweenBuffers(perMaterialContextData.materialParamsBuffers.uploadBuffer, perMaterialContextData.materialParamsBuffers.dataBuffer, regionDesc);

				
			}

			for (auto& dirtyIndex : perMaterialContextData.perMeshBuffers.dirtyIndices)
			{
				RHI::BufferRegionCopyDescription regionDesc =
				{
					.srcOffset = sizeof(MeshPerInstanceDataHandle) * dirtyIndex,
					.destOffset = sizeof(MeshPerInstanceDataHandle) * dirtyIndex,
					.width = sizeof(MeshPerInstanceDataHandle)
				};
				perMaterialContextData.perMeshBuffers.uploadBuffer->UploadData((void*)(*perMaterialContextData.perMeshData.raw()), regionDesc); // maybe upload as contiguous data to improve cache hits on gpu when copying 1-by-1
				commandBuffer->CopyDataBetweenBuffers(perMaterialContextData.perMeshBuffers.uploadBuffer, perMaterialContextData.perMeshBuffers.dataBuffer, regionDesc);
			}

			perMaterialContextData.materialParamsBuffers.dirtyIndices.clear();
			perMaterialContextData.perMeshBuffers.dirtyIndices.clear();
		}

		template <typename T>
		std::shared_ptr<RHI::IBuffer> GetRHIBufferForPerMaterialData() const
		{
			auto* perMaterialContextData = m_registry.ctx().find<PerMaterialContextData>(entt::type_id<T>().hash());
			ASSERT(perMaterialContextData, "No per-material context found");
			return perMaterialContextData->materialParamsBuffers.dataBuffer;
		}

		template <typename T>
		std::shared_ptr<RHI::IBuffer> GetRHIBufferForPerMeshData() const
		{
			auto* perMaterialContextData = m_registry.ctx().find<PerMaterialContextData>(entt::type_id<T>().hash());
			ASSERT(perMaterialContextData, "No per-material context found");
			return perMaterialContextData->perMeshBuffers.dataBuffer;
		}

		template <typename T>
		uint32_t GetActiveInstancesNum()
		{
			return m_registry.storage<T>().size();
		}

		const RHI::BufferWithRegionDescription& GetPrimaryVertexData() const 
		{
			return m_primaryVertexData;
		}

		const RHI::BufferWithRegionDescription& GetSecondaryVertexData() const 
		{
			return m_secondaryVertexData;
		}

		const RHI::BufferWithRegionDescription& GetIndicesData() const 
		{
			return m_indicesData;
		}

	private:
		template <typename T>
		void EnsureBufferCapacity(PerMaterialContextData& contextData, uint32_t requiredSize, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
		{
			auto& materialBuffer = contextData.materialParamsBuffers.dataBuffer;
			uint32_t currentMaterialSize = materialBuffer ? materialBuffer->GetStructuredElementsNum() : 0;
			if (currentMaterialSize < requiredSize)
			{
				contextData.materialParamsBuffers.Resize(requiredSize * 2, sizeof(T), commandBuffer);
			}

			auto& perMeshBuffer = contextData.perMeshBuffers.dataBuffer;
			uint32_t currentPerMeshSize = perMeshBuffer ? materialBuffer->GetStructuredElementsNum() : 0;
			if (currentPerMeshSize < requiredSize)
			{
				contextData.perMeshBuffers.Resize(requiredSize * 2, sizeof(MeshPerInstanceDataHandle), commandBuffer);
			}
		}

		RHI::BufferWithRegionDescription m_primaryVertexData;
		RHI::BufferWithRegionDescription m_secondaryVertexData;
		RHI::BufferWithRegionDescription m_indicesData;

		entt::registry m_registry;
	};
}