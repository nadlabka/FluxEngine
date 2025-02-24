#pragma once
#include <../Assets/AssetsManager.h>
#include "../Assets/Mesh.h"
#include "../Renderer/Renderer1.h"
#include "../Assets/Material.h"

static Assets::AssetsManager<Assets::StaticMesh>::AssetId LoadCubeMesh()
{
	auto& renderer = Renderer1::GetInstance();
	auto commandQueue = renderer.m_commandQueue;
	auto commandBuffer = renderer.m_commandBuffer;

	Assets::VertexPrimaryAttributes cubeVertices[24] = 
	{
		// Front face (Z = 0.5, Normal {0, 0, 1})
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f}},
		{{-0.5f, 0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 0.0f}},
		{{0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 1.0f}},
		{{0.5f,  -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 1.0f}},

		// Back face (Z = -0.5, Normal {0, 0, -1})
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
		{{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
		{{0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
		{{0.5f,  -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},

		// Left face (X = -0.5, Normal {-1, 0, 0})
		{{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

		// Right face (X = 0.5, Normal {1, 0, 0})
		{{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f,  -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
		{{0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

		// Top face (Y = 0.5, Normal {0, 1, 0})
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
		{{-0.5f,  0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		{{0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f,  0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},

		// Bottom face (Y = -0.5, Normal {0, -1, 0})
		{{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
		{{0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
		{{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}
	};

	Assets::VertexSecondaryAttributes cubeSecondaryVertices[24] = 
	{
		// Front face  (Normal:  {0, 0, 1})
		{{1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
		{{1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
		{{1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
		{{1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},

		// Back face (Normal: {0, 0, -1})
		{{-1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
		{{-1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
		{{-1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
		{{-1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},

		// Left face (Normal: {-1, 0, 0})
		{{0.0f, 0.0f, -1.0f}, {0.0f, -1.0f, 0.0f}},
		{{0.0f, 0.0f, -1.0f}, {0.0f, -1.0f, 0.0f}},
		{{0.0f, 0.0f, -1.0f}, {0.0f, -1.0f, 0.0f}},
		{{0.0f, 0.0f, -1.0f}, {0.0f, -1.0f, 0.0f}},

		// Right face (Normal: {1, 0, 0})
		{{0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
		{{0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
		{{0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
		{{0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},

		// Top face (Normal: {0, 1, 0})
		{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},

		// Bottom face (Normal: {0, -1, 0})
		{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
		{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
		{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
		{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}
	};

	uint32_t cubeIndices[36] =
	{
		// Front face
		0, 1, 2, 0, 2, 3,
		// Back face
		4, 6, 5, 4, 7, 6,
		// Left face
		8, 10, 9, 8, 11, 10,
		// Right face
		12, 14, 13, 12, 15, 14,
		// Top face
		16, 18, 17, 16, 19, 18,
		// Bottom face
		20, 22, 21, 20, 23, 22
	};

	auto& assetsManager = Assets::AssetsManager<Assets::StaticMesh>::GetInstance();
	auto meshAssetID = assetsManager.CreateAsset();
	auto& meshAsset = assetsManager.GetAsset(meshAssetID);

	auto& rhiContext = RHI::RHIContext::GetInstance();
	auto allocator = rhiContext.GetAllocator();

	RHI::BufferDescription commonUploadBufferDesc = {};
	commonUploadBufferDesc.access = RHI::BufferAccess::Upload;
	commonUploadBufferDesc.elementsNum = 1;
	commonUploadBufferDesc.elementStride = 768u;
	commonUploadBufferDesc.flags = { .requiredCopyStateToInit = false };
	commonUploadBufferDesc.usage = RHI::BufferUsage::None;
	auto commonUploadBuffer = allocator->CreateBuffer(commonUploadBufferDesc);

	RHI::BufferDescription primaryVertexBufferDesc = {};
	primaryVertexBufferDesc.access = RHI::BufferAccess::DefaultPrivate;
	primaryVertexBufferDesc.elementsNum = 24;
	primaryVertexBufferDesc.elementStride = sizeof(Assets::VertexPrimaryAttributes);
	primaryVertexBufferDesc.flags = { .requiredCopyStateToInit = true };
	primaryVertexBufferDesc.usage = RHI::BufferUsage::VertexBuffer;
	RHI::BufferWithRegionDescription primaryVertexData =
	{
		.buffer = allocator->CreateBuffer(primaryVertexBufferDesc),
		.regionDescription = 
		{
			.offset = 0,
			.size = 24 * sizeof(Assets::VertexPrimaryAttributes)
		}
	};

	RHI::BufferDescription secondaryVertexBufferDesc = {};
	secondaryVertexBufferDesc.access = RHI::BufferAccess::DefaultPrivate;
	secondaryVertexBufferDesc.elementsNum = 24;
	secondaryVertexBufferDesc.elementStride = sizeof(Assets::VertexSecondaryAttributes);
	secondaryVertexBufferDesc.flags = { .requiredCopyStateToInit = true };
	secondaryVertexBufferDesc.usage = RHI::BufferUsage::VertexBuffer;
	RHI::BufferWithRegionDescription secondaryVertexData =
	{
		.buffer = allocator->CreateBuffer(secondaryVertexBufferDesc),
		.regionDescription =
		{
			.offset = 0,
			.size = 24 * sizeof(Assets::VertexSecondaryAttributes)
		}
	};

	RHI::BufferDescription indicesBufferDesc = {};
	indicesBufferDesc.access = RHI::BufferAccess::DefaultPrivate;
	indicesBufferDesc.elementsNum = 36;
	indicesBufferDesc.elementStride = sizeof(uint32_t);
	indicesBufferDesc.flags = { .requiredCopyStateToInit = true };
	indicesBufferDesc.usage = RHI::BufferUsage::IndexBuffer;
	RHI::BufferWithRegionDescription indicesData =
	{
		.buffer = allocator->CreateBuffer(indicesBufferDesc),
		.regionDescription =
		{
			.offset = 0,
			.size = 36 * sizeof(uint32_t)
		}
	};

	RHI::BufferRegionCopyDescription copyDesc = {};
	copyDesc.srcOffset = primaryVertexData.regionDescription.offset;
	copyDesc.destOffset = 0;
	copyDesc.width = primaryVertexData.regionDescription.size;
	commonUploadBuffer->UploadData(cubeVertices, copyDesc);

	commandBuffer->BeginRecording(commandQueue);
	commandBuffer->CopyDataBetweenBuffers(commonUploadBuffer, primaryVertexData.buffer, { .srcOffset = 0, .destOffset = 0, .width = copyDesc.width });
	commandBuffer->EndRecording();
	commandBuffer->SubmitToQueue(commandQueue);

	commandBuffer->ForceWaitUntilFinished(commandQueue);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	copyDesc.srcOffset = secondaryVertexData.regionDescription.offset;
	copyDesc.destOffset = 0;
	copyDesc.width = secondaryVertexData.regionDescription.size;
	commonUploadBuffer->UploadData(cubeSecondaryVertices, copyDesc);

	commandBuffer->BeginRecording(commandQueue);
	commandBuffer->CopyDataBetweenBuffers(commonUploadBuffer, secondaryVertexData.buffer, { .srcOffset = 0, .destOffset = 0, .width = copyDesc.width });
	commandBuffer->EndRecording();
	commandBuffer->SubmitToQueue(commandQueue);

	commandBuffer->ForceWaitUntilFinished(commandQueue);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	copyDesc.srcOffset = indicesData.regionDescription.offset;
	copyDesc.destOffset = 0;
	copyDesc.width = indicesData.regionDescription.size;
	commonUploadBuffer->UploadData(cubeIndices, copyDesc);

	commandBuffer->BeginRecording(commandQueue);
	commandBuffer->CopyDataBetweenBuffers(commonUploadBuffer, indicesData.buffer, { .srcOffset = 0, .destOffset = 0, .width = copyDesc.width });
	commandBuffer->EndRecording();
	commandBuffer->SubmitToQueue(commandQueue);

	commandBuffer->ForceWaitUntilFinished(commandQueue);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	auto& resultSubmesh = meshAsset.m_submeshes.emplace_back(primaryVertexData, secondaryVertexData, indicesData);

	RHI::BufferDescription perInstanceStoreBufferDesc = {};
	perInstanceStoreBufferDesc.access = RHI::BufferAccess::DefaultPrivate;
	perInstanceStoreBufferDesc.elementsNum = 10;										 //hardcoded size, fix asap
	perInstanceStoreBufferDesc.elementStride = sizeof(Assets::MeshPerInstanceDataHandle);
	perInstanceStoreBufferDesc.flags = { .requiredCopyStateToInit = true };
	perInstanceStoreBufferDesc.usage = RHI::BufferUsage::VertexBuffer;
	auto perInstanceStoreBuffer = allocator->CreateBuffer(perInstanceStoreBufferDesc);

	RHI::BufferDescription perInstanceUploadBufferDesc = {};
	perInstanceUploadBufferDesc.access = RHI::BufferAccess::Upload;
	perInstanceUploadBufferDesc.elementsNum = 10;										 //hardcoded size, fix asap
	perInstanceUploadBufferDesc.elementStride = sizeof(Assets::MeshPerInstanceDataHandle);
	perInstanceUploadBufferDesc.flags = { .requiredCopyStateToInit = false };
	perInstanceUploadBufferDesc.usage = RHI::BufferUsage::None;
	auto perInstanceUploadBuffer = allocator->CreateBuffer(perInstanceUploadBufferDesc);

	resultSubmesh.SetRHIBuffersForPerInstanceData<Assets::MeshPerInstanceDataHandle>(perInstanceUploadBuffer, perInstanceStoreBuffer); //hardcoded size, fix asap


	RHI::BufferDescription perMeshBufferDesc = {};
	perMeshBufferDesc.access = RHI::BufferAccess::DefaultPrivate;
	perMeshBufferDesc.elementsNum = 10;										 //hardcoded size, fix asap
	perMeshBufferDesc.elementStride = sizeof(Assets::MeshPerInstanceData);
	perMeshBufferDesc.flags = { .requiredCopyStateToInit = true };
	perMeshBufferDesc.usage = RHI::BufferUsage::DataReadOnlyBuffer;
	auto perMeshBuffer = allocator->CreateBuffer(perMeshBufferDesc);

	RHI::BufferDescription perMeshUploadBufferDesc = {};
	perMeshUploadBufferDesc.access = RHI::BufferAccess::Upload;
	perMeshUploadBufferDesc.elementsNum = 10;										 //hardcoded size, fix asap
	perMeshUploadBufferDesc.elementStride = sizeof(Assets::MeshPerInstanceData);
	perMeshUploadBufferDesc.flags = { .requiredCopyStateToInit = false };
	perMeshUploadBufferDesc.usage = RHI::BufferUsage::None;
	auto perMeshUploadBuffer = allocator->CreateBuffer(perMeshUploadBufferDesc);

	meshAsset.SetRHIBuffersForPerInstanceData(perMeshUploadBuffer, perMeshBuffer);

	return meshAssetID;
}