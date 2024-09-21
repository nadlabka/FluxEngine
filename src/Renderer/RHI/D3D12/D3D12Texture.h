#pragma once
#include "../Texture.h"
#include "D3D12StatefulResource.h"
#include "Managers/DescriptorHeapManager.h"

namespace RHI
{
    DXGI_FORMAT ConvertFormatToD3D12(TextureFormat format);
    DXGI_FORMAT GetTyplessVersionOfFormat(DXGI_FORMAT format);
	D3D12_RESOURCE_DIMENSION ConvertTextureTypeToD3D12(TextureType type);
	

	struct TextureAccessInfo
	{
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_depth;
		uint32_t m_mipLevels;
		uint32_t m_arrayLayers;

		DXGI_FORMAT m_format;
		D3D12_RESOURCE_DIMENSION m_dimensions;
	};

	struct D3D12Texture : public ITexture, public D3D12StatefulResource
	{
		D3D12Texture() = default;
		D3D12Texture(const TextureAccessInfo& accessInfo, const ResourceDescriptorsIndices& descriptorsIndices, RscPtr<ID3D12Resource> texture);

		void SetClearColor(const std::array<float, 4>& clearColor);
		void SetClearValue(float clearValueDepth, uint8_t clearValueStencil);

		TextureAccessInfo m_accessInfo;
		D3D12_CLEAR_VALUE m_clearValue;
		ResourceDescriptorsIndices m_descriptorsIndices;
		RscPtr<ID3D12Resource> m_texture;
	};
}