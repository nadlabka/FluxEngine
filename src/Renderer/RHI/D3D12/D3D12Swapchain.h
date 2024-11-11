#pragma once
#include <stdafx.h>
#include "../Swapchain.h"
#include "D3D12Texture.h"

namespace RHI
{
	struct D3D12Swapchain : public ISwapchain
	{
		D3D12Swapchain();
		D3D12Swapchain(RscPtr<IDXGISwapChain3> swapchain, uint32_t framesCount);
		~D3D12Swapchain();

		void UpdateDescriptors();

		std::shared_ptr<ITexture> GetNextRenderTarget();
		void Resize(uint32_t width, uint32_t height);
		void Present();

		uint32_t m_currentFrameIndex;
		uint32_t m_framesCount;
		std::vector<std::shared_ptr<D3D12Texture>> m_backbufferTextures;
		RscPtr<IDXGISwapChain3> m_swapchain;
	};
}