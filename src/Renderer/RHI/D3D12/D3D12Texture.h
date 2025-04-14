#pragma once
#include "../Texture.h"
#include "D3D12StatefulResource.h"
#include "Managers/DescriptorsHeapsManager.h"

class D3D12MA::Allocation;

namespace RHI
{
    DXGI_FORMAT ConvertFormatToD3D12(TextureFormat format);
    DXGI_FORMAT GetTyplessVersionOfFormat(DXGI_FORMAT format);
	D3D12_RESOURCE_DIMENSION ConvertTextureTypeToResourceDimension(TextureType type);
	D3D12_SRV_DIMENSION  ConvertTextureTypeToSRVDimension(TextureType type);
    D3D12_RESOURCE_STATES ConvertTextureLayoutToResourceState(TextureLayout layout);
	uint32_t GetDXGIFormatSizeInBytes(DXGI_FORMAT format);
	
	struct TextureDimensionsInfo
	{
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_depth = 1;
		uint32_t m_mipLevels = 1;
		uint32_t m_arrayLayers = 1;
		uint32_t m_formatPixelSizeBytes = 4;
	};

	struct D3D12Texture : public ITexture, public D3D12StatefulResource
	{
		D3D12Texture() = default;
		D3D12Texture(const TextureDimensionsInfo& dimensionsInfo, RscPtr<D3D12MA::Allocation> allocation, D3D12_RESOURCE_STATES resourceState);
		D3D12Texture(const TextureDimensionsInfo& dimensionsInfo, RscPtr<ID3D12Resource> texture, D3D12_RESOURCE_STATES resourceState);
		~D3D12Texture();

		void AllocateDescriptorsInHeaps(const TextureDescription& desc);

		TextureDimensionsInfo m_dimensionsInfo;

		std::vector<uint32_t> m_UAVDescriptorsIndices;
		std::vector<uint32_t> m_RTVDescriptorsIndices;
		std::vector<uint32_t> m_DSVDescriptorIndices;
		uint32_t m_SRVDescriptorIndex = D3D12DescriptorHeap::INDEX_INVALID;

		//use recource OR allocation
		RscPtr<D3D12MA::Allocation> m_allocation;
		RscPtr<ID3D12Resource> m_texture;
	};
}