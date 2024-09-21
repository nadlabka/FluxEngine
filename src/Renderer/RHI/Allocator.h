#pragma once
#include "Texture.h"

namespace RHI
{
	struct IAllocator
	{
		virtual ~IAllocator() {}

		virtual std::shared_ptr<ITexture> CreateTexture(const TextureDesc& desc) const = 0;
	};
}