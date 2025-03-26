#pragma once
#include <memory>
#include <vector>
#include "../RHI/RHIContext.h"

struct PrivateUploadBuffersPair
{
	std::shared_ptr<RHI::IBuffer> uploadBuffer;
	std::shared_ptr<RHI::IBuffer> dataBuffer;

	void Resize(uint32_t newElementCount, uint32_t elementStride, std::shared_ptr<RHI::ICommandBuffer> commandBuffer, RHI::BufferUsage privateBufferUsage = RHI::BufferUsage::DataReadOnlyBuffer);
};

struct BuffersWithDirtyIndices : PrivateUploadBuffersPair
{
	std::vector<uint32_t> dirtyIndices;
};