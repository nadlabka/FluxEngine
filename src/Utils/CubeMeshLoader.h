#pragma once
#include <../Assets/AssetsManager.h>
#include "../Assets/Mesh.h"
#include "../Renderer/Renderer1.h"

static Assets::AssetsManager<Assets::StaticMesh>::AssetId LoadCubeMesh()
{
	auto& renderer = Renderer1::GetInstance();
	auto commandQueue = renderer.m_commandQueue;
	auto commandBuffer = renderer.m_commandBuffer;

	Assets::VertexPrimaryAttributes cubeVertices[24] = 
	{
		// Front face (Z = 0.5, Normal {0, 0, 1})
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 0.0f}},
		{{0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 1.0f}},

		// Back face (Z = -0.5, Normal {0, 0, -1})
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
		{{0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},

		// Left face (X = -0.5, Normal {-1, 0, 0})
		{{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

		// Right face (X = 0.5, Normal {1, 0, 0})
		{{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
		{{0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

		// Top face (Y = 0.5, Normal {0, 1, 0})
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
		{{0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		{{0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},

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
		0, 1, 2, 2, 3, 0, // Front
		4, 5, 6, 6, 7, 4, // Back
		0, 4, 7, 7, 3, 0, // Left
		1, 5, 6, 6, 2, 1, // Right
		3, 2, 6, 6, 7, 3, // Top
		0, 1, 5, 5, 4, 0  // Bottom
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
	indicesBufferDesc.elementsNum = 32;
	indicesBufferDesc.elementStride = sizeof(uint32_t);
	indicesBufferDesc.flags = { .requiredCopyStateToInit = true };
	indicesBufferDesc.usage = RHI::BufferUsage::IndexBuffer;
	RHI::BufferWithRegionDescription indicesData =
	{
		.buffer = allocator->CreateBuffer(indicesBufferDesc),
		.regionDescription =
		{
			.offset = 0,
			.size = 32 * sizeof(uint32_t)
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

	meshAsset.m_submeshes.emplace_back(primaryVertexData, secondaryVertexData, indicesData);

	return meshAssetID;
}