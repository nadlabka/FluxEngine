#pragma once
#include "Texture.h"

namespace RHI
{
	struct IAllocator
	{
		virtual ~IAllocator() {}

		virtual std::shared_ptr<ITexture> CreateTexture(const TextureDescription& desc) const = 0;
		virtual std::shared_ptr<IBuffer> CreateBuffer(const BufferDescription& desc) const = 0;
	};
}