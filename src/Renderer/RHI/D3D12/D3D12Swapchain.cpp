#include "stdafx.h"
#include "D3D12Swapchain.h"
#include "D3D12Device.h"
#include "../RHIContext.h"

RHI::D3D12Swapchain::D3D12Swapchain()
{
}

RHI::D3D12Swapchain::D3D12Swapchain(RscPtr<IDXGISwapChain1> swapchain, uint32_t framesCount) : m_swapchain(swapchain), m_framesCount(framesCount)
{
}

RHI::D3D12Swapchain::~D3D12Swapchain()
{
}

void RHI::D3D12Swapchain::UpdateDescriptors()
{
	DXGI_SWAP_CHAIN_DESC1 desc;
	m_swapchain->GetDesc1(&desc);

	m_backbufferTextures.clear();

    for (uint32_t i = 0; i < m_framesCount; ++i)
    {
        RscPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        TextureDimensionsInfo dimensions;
        dimensions.m_width = desc.Width;
        dimensions.m_height = desc.Height;

        auto& emplacedTexture = m_backbufferTextures.emplace_back(dimensions, backBuffer, D3D12_RESOURCE_STATE_PRESENT);

        TextureDescription textureDescForIndicesGen;
        textureDescForIndicesGen.usage = eTextureUsage_ColorAttachment;
        textureDescForIndicesGen.aspect = eTextureAspect_HasColor;
        textureDescForIndicesGen.format = TextureFormat::BGRA8_UNORM;
        textureDescForIndicesGen.type = TextureType::Texture2D;
        textureDescForIndicesGen.layout = TextureLayout::Present;
        textureDescForIndicesGen.width = dimensions.m_width;
        textureDescForIndicesGen.height = dimensions.m_height;
        textureDescForIndicesGen.depth = 1;
        textureDescForIndicesGen.mipLevels = 1;
        textureDescForIndicesGen.arrayLayers = 1;

        emplacedTexture.AllocateDescriptorsInHeaps(textureDescForIndicesGen);
    }
}

std::shared_ptr<RHI::ITexture> RHI::D3D12Swapchain::GetNextRenderTarget()
{
    auto& currentTexture = m_backbufferTextures[currentFrameIndex];
    currentFrameIndex = (currentFrameIndex + 1) % m_framesCount;

    return std::make_shared<D3D12Texture>(currentTexture);
}

void RHI::D3D12Swapchain::Resize(uint32_t width, uint32_t height)
{
    m_backbufferTextures.clear();

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    ThrowIfFailed(m_swapchain->GetDesc(&swapChainDesc));
    ThrowIfFailed(m_swapchain->ResizeBuffers(m_framesCount, width, height,
        swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

    for (uint32_t i = 0; i < m_framesCount; ++i)
    {
        RscPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        TextureDimensionsInfo dimensions;
        dimensions.m_width = width;
        dimensions.m_height = height;

        auto& emplacedTexture = m_backbufferTextures.emplace_back(dimensions, backBuffer, D3D12_RESOURCE_STATE_PRESENT);

        TextureDescription textureDescForIndicesGen;
        textureDescForIndicesGen.usage = eTextureUsage_ColorAttachment;
        textureDescForIndicesGen.aspect = eTextureAspect_HasColor;
        textureDescForIndicesGen.format = TextureFormat::BGRA8_UNORM;
        textureDescForIndicesGen.type = TextureType::Texture2D;
        textureDescForIndicesGen.layout = TextureLayout::Present;
        textureDescForIndicesGen.width = dimensions.m_width;
        textureDescForIndicesGen.height = dimensions.m_height;
        textureDescForIndicesGen.depth = 1;
        textureDescForIndicesGen.mipLevels = 1;
        textureDescForIndicesGen.arrayLayers = 1;

        emplacedTexture.AllocateDescriptorsInHeaps(textureDescForIndicesGen);
    }
}

void RHI::D3D12Swapchain::Present()
{
    m_swapchain->Present(1, 0);
}
