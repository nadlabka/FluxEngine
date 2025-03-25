#include "stdafx.h"
#include "Submesh.h"

Assets::StaticSubmesh::StaticSubmesh(RHI::BufferWithRegionDescription primaryVertexData, RHI::BufferWithRegionDescription secondaryVertexData, RHI::BufferWithRegionDescription indicesData)
	: m_primaryVertexData(primaryVertexData), m_secondaryVertexData(secondaryVertexData), m_indicesData(indicesData), m_registry()
{
}
