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
	struct PerInstanceCommonData
	{
		Matrix perInstanceTransform;
	};

	struct BuffersWithDirtyIndices
	{
		std::shared_ptr<RHI::IBuffer> uploadBuffer;
		std::shared_ptr<RHI::IBuffer> dataBuffer;
		std::vector<uint32_t> dirtyIndices;
	};

	class StaticSubmesh
	{
	public:
		StaticSubmesh() = default;
		StaticSubmesh(RHI::BufferWithRegionDescription primaryVertexData, RHI::BufferWithRegionDescription secondaryVertexData, RHI::BufferWithRegionDescription indicesData);

		template <typename T>
		void SetPerInstanceData(Core::Entity entity, T&& data) // we need to delete all requested instances first and insert only after this
		{
			auto* bufferWithIndices = m_registry.ctx().find<BuffersWithDirtyIndices>(entt::type_id<T>().hash());
			ASSERT(bufferWithIndices, "You have to set RHI buffer before adding any PerInstance data");

			if (!m_registry.valid(entity))
			{
				m_registry.create(entity);
				bufferWithIndices->dirtyIndices.push_back(m_registry.storage<T>().size());
			}
			else
			{
				bufferWithIndices->dirtyIndices.push_back(m_registry.storage<T>().index(entity));
			}

			m_registry.emplace_or_replace<T>(entity, std::forward<T>(data));
		}

		void DestroyEntityReference(Core::Entity entity) // we need to delete all requested instances first and insert only after this
		{
			for (auto&& typedStorage : m_registry.storage())
			{
				auto storageType = typedStorage.first;
				auto& storage = typedStorage.second;

				auto* bufferWithIndices = m_registry.ctx().find<BuffersWithDirtyIndices>(storageType);
				bufferWithIndices->dirtyIndices.push_back(storage.index(entity));
			}
			m_registry.destroy(entity);
		}
		
		template<typename T>
		void SetRHIBuffersForPerInstanceData(std::shared_ptr<RHI::IBuffer> uploadBuffer, std::shared_ptr<RHI::IBuffer> dataBuffer)
		{
			auto* bufferWithIndices = m_registry.ctx().find<BuffersWithDirtyIndices>(entt::type_id<T>().hash());
			if (!bufferWithIndices)
			{
				m_registry.ctx().insert_or_assign(entt::type_id<T>().hash(), BuffersWithDirtyIndices{ uploadBuffer, dataBuffer, {} });
			}
			else
			{
				bufferWithIndices->uploadBuffer = uploadBuffer;
				bufferWithIndices->dataBuffer = dataBuffer;
			}
		}

		template<typename T>
		std::shared_ptr<RHI::IBuffer> GetRHIBufferForPerInstanceData() const
		{
			auto* bufferWithIndices = m_registry.ctx().find<BuffersWithDirtyIndices>(entt::type_id<T>().hash());
			if (!bufferWithIndices)
			{
				throw std::runtime_error("No buffer found for the given type");
			}
			return bufferWithIndices->dataBuffer;
		}

		template<typename T>
		void UpdateRHIBufferWithPerInstanceData(std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
		{
			auto& storage = m_registry.storage<T>();
			auto* bufferWithIndices = m_registry.ctx().find<BuffersWithDirtyIndices>(entt::type_id<T>().hash());
			for (auto& dirtyIndex : bufferWithIndices->dirtyIndices)
			{
				RHI::BufferRegionCopyDescription regionDesc =
				{
					.srcOffset = sizeof(T) * dirtyIndex,
					.destOffset = sizeof(T) * dirtyIndex,
					.width = sizeof(T)
				};
				bufferWithIndices->uploadBuffer->UploadData((void*)storage.data(), regionDesc); // maybe upload as contiguous data to improve cache hits on gpu when copying 1-by-1
				commandBuffer->CopyDataBetweenBuffers(bufferWithIndices->uploadBuffer, bufferWithIndices->dataBuffer, regionDesc);
			}
			bufferWithIndices->dirtyIndices.clear();
		}

	private:
		RHI::BufferWithRegionDescription m_primaryVertexData;
		RHI::BufferWithRegionDescription m_secondaryVertexData;
		RHI::BufferWithRegionDescription m_indicesData;

		entt::registry m_registry;
	};
}