#pragma once
#include <cstdint>
#include <memory>
#include "Buffer.h"
#include "Sampler.h"

namespace RHI
{
	struct PipelineLayoutDescription
	{
		struct BufferInfo 
		{
			std::shared_ptr<IBuffer> buffer;
			uint32_t offset;
			uint32_t size;
		};

		std::vector<BufferInfo> buffers;
		std::vector<ITexture> textures;
		std::vector<ISampler> samplers;
	};

	struct IPipelineLayout
	{
		virtual ~IPipelineLayout() {}
	};
}