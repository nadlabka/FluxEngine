#include "stdafx.h"
#include "Mesh.h"

void Assets::StaticMesh::CreatePerInstanceData(Core::Entity ent, const MeshPerInstanceData& data)
{
	if (!m_meshPerInstanceData.contains(ent))
	{
		m_meshPerInstanceDataBuffers.dirtyIndices.push_back(m_meshPerInstanceData.size());
		m_meshPerInstanceData.emplace(ent);
	}
	else
	{
		m_meshPerInstanceDataBuffers.dirtyIndices.push_back(m_meshPerInstanceData.index(ent));
	}

	m_meshPerInstanceData.get(ent) = data;
}

void Assets::StaticMesh::UpdatePerInstanceData(Core::Entity ent, const MeshPerInstanceData& data)
{
	m_meshPerInstanceData.get(ent) = data;
	m_meshPerInstanceDataBuffers.dirtyIndices.push_back(m_meshPerInstanceData.index(ent));
}

void Assets::StaticMesh::UpdateRHIBufferWithPerInstanceData(std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
{
	auto& rhiContext = RHI::RHIContext::GetInstance();
	auto allocator = rhiContext.GetAllocator();

	uint32_t currentSize = m_meshPerInstanceDataBuffers.dataBuffer ? m_meshPerInstanceDataBuffers.dataBuffer->GetStructuredElementsNum() : 0;
	if (currentSize < m_meshPerInstanceData.size())
	{
		m_meshPerInstanceDataBuffers.Resize(m_meshPerInstanceData.size() * 2, sizeof(MeshPerInstanceData), commandBuffer);
	}

	for (auto& dirtyIndex : m_meshPerInstanceDataBuffers.dirtyIndices)
	{
		RHI::BufferRegionCopyDescription regionDesc =
		{
			.srcOffset = sizeof(MeshPerInstanceData) * dirtyIndex,
			.destOffset = sizeof(MeshPerInstanceData) * dirtyIndex,
			.width = sizeof(MeshPerInstanceData)
		};
		m_meshPerInstanceDataBuffers.uploadBuffer->UploadData((void*)*m_meshPerInstanceData.raw(), regionDesc); // maybe upload as contiguous data to improve cache hits on gpu when copying 1-by-1
		commandBuffer->CopyDataBetweenBuffers(m_meshPerInstanceDataBuffers.uploadBuffer, m_meshPerInstanceDataBuffers.dataBuffer, regionDesc);
	}
	m_meshPerInstanceDataBuffers.dirtyIndices.clear();
}

std::shared_ptr<RHI::IBuffer> Assets::StaticMesh::GetRHIBufferForPerInstanceData() const
{
	ASSERT(m_meshPerInstanceDataBuffers.dataBuffer, "You have to set RHI buffer");
	return m_meshPerInstanceDataBuffers.dataBuffer;
}

void Assets::StaticMesh::DeleteInstance(Core::Entity ent)
{
	for (auto& submesh : m_submeshes)
	{
		submesh.DestroyEntityReference(ent, (Core::Entity)m_meshPerInstanceData.data()[m_meshPerInstanceData.size() - 1], MeshPerInstanceDataHandle{ (uint32_t)m_meshPerInstanceData.index(ent) });
	}
	m_meshPerInstanceData.remove(ent);
}
