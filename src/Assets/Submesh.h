#pragma once
#include <memory>
#include <../Renderer/RHI/RHIContext.h>
#include "Material.h"
#include "AssetsManager.h"
#include "Vertex.h"
#include <entt/entt.hpp>
#include <ECS/Entity.h>

namespace Assets
{
	struct MeshPerInstanceDataHandle
	{
		uint32_t indexInPool;
	};

	struct PrivateUploadBuffersPair
	{
		std::shared_ptr<RHI::IBuffer> uploadBuffer;
		std::shared_ptr<RHI::IBuffer> dataBuffer;

		void Resize(uint32_t newElementCount, uint32_t elementStride, std::shared_ptr<RHI::ICommandBuffer> commandBuffer);
	};

	struct BuffersWithDirtyIndices : PrivateUploadBuffersPair
	{
		std::vector<uint32_t> dirtyIndices;
	};

	struct PerMaterialContextData
	{
		BuffersWithDirtyIndices materialParamsBuffers;

		entt::storage<MeshPerInstanceDataHandle> perMeshData;
		PrivateUploadBuffersPair perMeshBuffers;
	};

	class StaticSubmesh
	{
	public:
		StaticSubmesh() = default;
		StaticSubmesh(RHI::BufferWithRegionDescription primaryVertexData, RHI::BufferWithRegionDescription secondaryVertexData, RHI::BufferWithRegionDescription indicesData);

		template <typename T, typename... Args>
		void CreatePerInstanceData(Core::Entity entity, const MeshPerInstanceDataHandle& perMeshHandle, Args&&... args) // we need to delete all requested instances first and insert only after this
		{
			auto* perMaterialContextData = m_registry.ctx().find<PerMaterialContextData>(entt::type_id<T>().hash());
			ASSERT(perMaterialContextData, "You have to set RHI buffers before adding any PerInstance data");

			ASSERT(!m_registry.valid(entity), "Instance for this entity has already been created");

			m_registry.create(entity);
			perMaterialContextData->materialParamsBuffers.dirtyIndices.push_back(m_registry.storage<T>().size());

			perMaterialContextData->perMeshData.emplace(entity, perMeshHandle);

			m_registry.emplace<T>(entity, std::forward<Args>(args)...);
		}

		template <typename T, typename... Args>
		void UpdatePerInstancePerMaterialData(Core::Entity entity, Args&&... args)
		{
			ASSERT(m_registry.valid(entity) && m_registry.any_of<T>(entity), "PerInstance has to be set first before being updated");
			m_registry.replace<T>(entity, args);
		}

		template <typename T>
		void UpdatePerInstancePerMeshData(Core::Entity entity, const MeshPerInstanceDataHandle& perMeshHandle)
		{
			ASSERT(m_registry.valid(entity) && m_registry.ctx().contains(entt::type_id<T>().hash()), "PerInstance has to be set first before being updated");
			auto* perMaterialContextData = m_registry.ctx().find<PerMaterialContextData>(entt::type_id<T>().hash());
			perMaterialContextData->perMeshData.replace(entity, perMeshHandle);
		}

		void DestroyEntityReference(Core::Entity entity, Core::Entity swappedMesh, const MeshPerInstanceDataHandle& perMeshHandle) // we need to delete all requested instances first and insert only after this
		{
			for (auto&& typedStorage : m_registry.storage())
			{
				auto storageType = typedStorage.first;
				auto& storage = typedStorage.second;

				auto* perMaterialContextData = m_registry.ctx().find<PerMaterialContextData>(storageType);

				if (!storage.contains(swappedMesh))
				{
					perMaterialContextData->perMeshData.get(swappedMesh) = perMeshHandle;
				}

				if (storage.contains(entity))
				{
					size_t indexInPool = storage.index(entity);

					perMaterialContextData->materialParamsBuffers.dirtyIndices.push_back(indexInPool);

					perMaterialContextData->perMeshData.remove(entity);
				}
			}
			m_registry.destroy(entity);
		}

		template<typename T>
		void UpdateRHIBuffersWithPerInstanceData(std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
		{
			auto& storage = m_registry.storage<T>();
			auto* perMaterialContextData = m_registry.ctx().find<PerMaterialContextData>(entt::type_id<T>().hash());
			for (auto& dirtyIndex : perMaterialContextData->materialParamsBuffers.dirtyIndices)
			{
				RHI::BufferRegionCopyDescription regionDesc =
				{
					.srcOffset = sizeof(T) * dirtyIndex,
					.destOffset = sizeof(T) * dirtyIndex,
					.width = sizeof(T)
				};
				perMaterialContextData->materialParamsBuffers.uploadBuffer->UploadData((void*)*storage.raw(), regionDesc); // maybe upload as contiguous data to improve cache hits on gpu when copying 1-by-1
				commandBuffer->CopyDataBetweenBuffers(perMaterialContextData->materialParamsBuffers.uploadBuffer, perMaterialContextData->materialParamsBuffers.dataBuffer, regionDesc);

				regionDesc =
				{
					.srcOffset = sizeof(MeshPerInstanceDataHandle) * dirtyIndex,
					.destOffset = sizeof(MeshPerInstanceDataHandle) * dirtyIndex,
					.width = sizeof(MeshPerInstanceDataHandle)
				};
				perMaterialContextData->perMeshBuffers.uploadBuffer->UploadData((void*)*(perMaterialContextData->perMeshData.raw(), regionDesc); // maybe upload as contiguous data to improve cache hits on gpu when copying 1-by-1
				commandBuffer->CopyDataBetweenBuffers(perMaterialContextData->perMeshBuffers.uploadBuffer, perMaterialContextData->perMeshBuffers.dataBuffer, regionDesc);
			}
			perMaterialContextData->materialParamsBuffers.dirtyIndices.clear();
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
		void InitializeBuffers(PerMaterialContextData& contextData, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
		{
			auto& rhiContext = RHI::RHIContext::GetInstance();
			auto allocator = rhiContext.GetAllocator();

			contextData.materialParamsBuffers.Resize(16, sizeof(T), allocator, commandBuffer);
			contextData.perMeshBuffers.Resize(16, sizeof(MeshPerInstanceDataHandle), allocator, commandBuffer);
		}

		template <typename T>
		void EnsureBufferCapacity(PerMaterialContextData& contextData, uint32_t requiredSize, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
		{
			auto& rhiContext = RHI::RHIContext::GetInstance();
			auto allocator = rhiContext.GetAllocator();

			auto& materialBuffer = contextData.materialParamsBuffers.dataBuffer;
			uint32_t currentMaterialSize = materialBuffer ? materialBuffer->GetStructuredElementsNum() : 0;
			if (currentMaterialSize < requiredSize)
			{
				contextData.materialParamsBuffers.Resize(requiredSize * 2, sizeof(T), allocator, commandBuffer);
			}

			auto& perMeshBuffer = contextData.perMeshBuffers.dataBuffer;
			uint32_t currentPerMeshSize = perMeshBuffer ? materialBuffer->GetStructuredElementsNum() : 0;
			if (currentPerMeshSize < requiredSize)
			{
				contextData.perMeshBuffers.Resize(requiredSize * 2, sizeof(MeshPerInstanceDataHandle), allocator, commandBuffer);
			}
		}

		RHI::BufferWithRegionDescription m_primaryVertexData;
		RHI::BufferWithRegionDescription m_secondaryVertexData;
		RHI::BufferWithRegionDescription m_indicesData;

		entt::registry m_registry;
	};
}