#pragma once
#include <../Renderer/RHI/RHIContext.h>
#include "Material.h"
#include "AssetsManager.h"
#include "Vertex.h"

namespace Assets
{
	struct SubmeshRawData
	{
		struct DataAdressRange
		{
			uint32_t offset;
			uint32_t size;
		};

		std::pair<std::shared_ptr<std::vector<VertexPrimaryAttributes>>, DataAdressRange> m_primaryVertexAttributes;  //u don't want indidvial buffers, u want 1 big shared data buffer with accessors details for each submesh
		std::pair<std::shared_ptr<std::vector<VertexSecondaryAttributes>>, DataAdressRange> m_secondaryVertexAttributes; asasasasasasa;

		std::pair<std::shared_ptr<std::vector<std::byte>>, DataAdressRange> m_indices;
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