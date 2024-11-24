#pragma once
#include <../Renderer/RHI/RHIContext.h>
#include "Material.h"
#include "AssetsManager.h"
#include "Vertex.h"

namespace Assets
{
	struct SubmeshRawData
	{
		std::vector<VertexPrimaryAttributes> m_primaryVertexAttributes;
		std::vector<VertexSecondaryAttributes> m_secondaryVertexAttributes;

		std::vector<std::byte> m_indices;
		uint32_t m_indexSizeInBytes;
	};

	struct SubmeshAsset
	{
		std::pair<std::shared_ptr<RHI::IBuffer>, RHI::BufferRegionDescription> m_primaryVertexAttributesBuffer;
		std::pair<std::shared_ptr<RHI::IBuffer>, RHI::BufferRegionDescription> m_secondaryVertexAttributesBuffer;
		std::pair<std::shared_ptr<RHI::IBuffer>, RHI::BufferRegionDescription> m_indexBuffer;

		AssetsManager<Material>::AssetId m_materialId;
	};
}