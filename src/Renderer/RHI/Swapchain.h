#pragma once
#include "Texture.h"

namespace RHI
{
	struct ISwapchain
	{
		virtual ~ISwapchain() {}

		virtual std::shared_ptr<ITexture> GetNextRenderTarget() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void Present() = 0;
	};
}