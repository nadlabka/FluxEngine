#include "stdafx.h"
#include "Mesh.h"

void Assets::StaticMesh::SetPerInstanceData(Core::Entity ent, const MeshPerInstanceData& data)
{
	ASSERT(m_meshPerInstanceDataBuffers.dataBuffer && m_meshPerInstanceDataBuffers.uploadBuffer, "You have to set RHI buffer before adding any PerInstance data");
	if (!m_meshPerInstanceData.contains(ent))
	{
		m_meshPerInstanceData.emplace(ent);
		m_meshPerInstanceDataBuffers.dirtyIndices.push_back(m_meshPerInstanceData.size());
	}
	else
	{
		m_meshPerInstanceDataBuffers.dirtyIndices.push_back(m_meshPerInstanceData.index(ent));
	}

	m_meshPerInstanceData.get(ent) = data;

	uint32_t perInstanceDataPoolIndex = m_meshPerInstanceData.index(ent);
	for (auto& submesh : m_submeshes)
	{
		submesh.SetPerInstanceData<MeshPerInstanceDataHandle>(ent, perInstanceDataPoolIndex);
	}
}

void Assets::StaticMesh::SetRHIBuffersForPerInstanceData(std::shared_ptr<RHI::IBuffer> uploadBuffer, std::shared_ptr<RHI::IBuffer> dataBuffer)
{
	if (!m_meshPerInstanceDataBuffers.dataBuffer || !m_meshPerInstanceDataBuffers.uploadBuffer)
	{
		m_meshPerInstanceDataBuffers = BuffersWithDirtyIndices{ uploadBuffer, dataBuffer, {} };
	}
	else
	{
		m_meshPerInstanceDataBuffers.uploadBuffer = uploadBuffer;
		m_meshPerInstanceDataBuffers.dataBuffer = dataBuffer;
	}
}

void Assets::StaticMesh::UpdateRHIBufferWithPerInstanceData(std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
{
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
