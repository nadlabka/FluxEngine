#pragma once
#include <cstdint>
#include <memory>

namespace RHI
{
	struct PipelineLayoutDescription
	{
		struct BufferInfo 
		{
			std::shared_ptr<IBuffer> buffer;
			size_t offset = 0;
			size_t size = 0;
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