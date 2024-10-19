#include "stdafx.h"
#include "../RHIContext.h"
#include "D3D12Texture.h"
#include "D3D12Allocator.h"
#include "D3D12Device.h"

RHI::D3D12Texture::D3D12Texture(const TextureDimensionsInfo& dimensionsInfo, RscPtr<D3D12MA::Allocation> allocation, D3D12_RESOURCE_STATES resourceState)
	: m_dimensionsInfo(dimensionsInfo), m_allocation(allocation), D3D12StatefulResource(resourceState)
{

}

RHI::D3D12Texture::D3D12Texture(const TextureDimensionsInfo& dimensionsInfo, RscPtr<ID3D12Resource> texture, D3D12_RESOURCE_STATES resourceState) 
	: m_dimensionsInfo(dimensionsInfo), m_texture(texture), D3D12StatefulResource(resourceState)
{

}

RHI::D3D12Texture::~D3D12Texture()
{
	auto& descHeapsMgr = DescriptorsHeapsManager::GetInstance();
	auto rtv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto dsv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	auto cbv_srv_uav_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (auto& idx : m_UAVDescriptorsIndices)
	{
		cbv_srv_uav_heap->EraseIndex(idx);
	}
	for (auto& idx : m_SRVDescriptorsIndices)
	{
		cbv_srv_uav_heap->EraseIndex(idx);
	}
	for (auto& idx : m_RTVDescriptorsIndices)
	{
		rtv_heap->EraseIndex(idx);
	}
	if (m_DSVDescriptorIndex != D3D12DescriptorHeap::INDEX_INVALID)
	{
		dsv_heap->EraseIndex(m_DSVDescriptorIndex);
	}
}

void RHI::D3D12Texture::AllocateDescriptorsInHeaps(const TextureDescription& desc)
{
	auto& rhiContext = RHIContext::GetInstance();
	auto d3d12device = std::static_pointer_cast<D3D12Device>(rhiContext.GetDevice());

    auto& descHeapsMgr = DescriptorsHeapsManager::GetInstance();
    auto rtv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    auto dsv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    auto cbv_srv_uav_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	const bool isDS = (desc.aspect & eTextureAspect_HasDepth || desc.aspect & eTextureAspect_HasStencil);
	m_SRVDescriptorsIndices.reserve(desc.mipLevels);
	m_UAVDescriptorsIndices.reserve(desc.mipLevels);
	m_RTVDescriptorsIndices.reserve(desc.mipLevels);

	auto format = ConvertFormatToD3D12(desc.format);

	ID3D12Resource* resourcePtr = m_texture.ptr() ? m_texture.ptr() : m_allocation->GetResource();

	if (isDS)
	{
		uint32_t dsvIDX = dsv_heap->AllocateIndex();
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc
		{
			.Format = format,
			.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
		};
		auto handle = dsv_heap->GetCpuHandle(dsvIDX);
		d3d12device->m_device->CreateDepthStencilView(resourcePtr, &dsv_desc, handle);

		m_DSVDescriptorIndex = dsvIDX;
	}
	if (desc.usage & eTextureUsage_ColorAttachment) 
	{
		for (uint32_t i = 0; i < desc.mipLevels; i++)
		{
			uint32_t rtvIDX = rtv_heap->AllocateIndex();
			D3D12_RENDER_TARGET_VIEW_DESC rtv_desc
			{
				.Format = format,
				.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
				.Texture2D = {.MipSlice = i }
			};
			auto handle = rtv_heap->GetCpuHandle(rtvIDX);
			d3d12device->m_device->CreateRenderTargetView(resourcePtr, &rtv_desc, handle);

			m_RTVDescriptorsIndices.push_back(rtvIDX);
		}
	}
	if (desc.usage & eTextureUsage_Sampled) 
	{
		auto formatForDS_SRV = format == DXGI_FORMAT_D32_FLOAT ? DXGI_FORMAT_R32_FLOAT : format;

		uint32_t srvIDX = cbv_srv_uav_heap->AllocateIndex();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = isDS ? formatForDS_SRV : format;
		srvDesc.ViewDimension = ConvertTextureTypeToSRVDimension(desc.type);
		srvDesc.Texture2D.MipLevels = desc.mipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;

		auto handle = cbv_srv_uav_heap->GetCpuHandle(srvIDX);
		d3d12device->m_device->CreateShaderResourceView(resourcePtr, &srvDesc, handle);

		m_SRVDescriptorsIndices.push_back(srvIDX);
	}
	if (desc.usage & eTextureUsage_Storage) 
	{
		for (uint32_t i = 0; i < desc.mipLevels; i++)
		{
			uint32_t uavIDX = cbv_srv_uav_heap->AllocateIndex();

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc
			{
				.Format = format,
				.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
				.Texture2D = 
				{
					.MipSlice = i,
					.PlaneSlice = 0
				}
			};
			auto handle = cbv_srv_uav_heap->GetCpuHandle(uavIDX);
			d3d12device->m_device->CreateUnorderedAccessView(resourcePtr, nullptr, &uavDesc, handle);

			m_UAVDescriptorsIndices.push_back(uavIDX);
		}
	}
}

DXGI_FORMAT RHI::ConvertFormatToD3D12(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::BGRA8_UNORM:  return DXGI_FORMAT_B8G8R8A8_UNORM;
    case TextureFormat::RGBA8_UNORM:  return DXGI_FORMAT_R8G8B8A8_UNORM;
    case TextureFormat::RGBA8_UINT:  return DXGI_FORMAT_R8G8B8A8_UINT;

    case TextureFormat::RGBA16_UNORM:  return DXGI_FORMAT_R16G16B16A16_UNORM;
    case TextureFormat::RGBA16_SNORM:  return DXGI_FORMAT_R16G16B16A16_SNORM;

    case TextureFormat::RGBA16_FLOAT:  return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case TextureFormat::RGBA32_FLOAT:  return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case TextureFormat::R32_UINT:  return DXGI_FORMAT_R32_UINT;
    case TextureFormat::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;

    case TextureFormat::D32_FLOAT:  return DXGI_FORMAT_D32_FLOAT;
    case TextureFormat::D24_UNORM_S8_UINT:  return DXGI_FORMAT_D24_UNORM_S8_UINT;

    case TextureFormat::Undefined:  return DXGI_FORMAT_UNKNOWN;

    default:
        throw std::runtime_error("Unsupported texture format");
    }
}

DXGI_FORMAT RHI::GetTyplessVersionOfFormat(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_B8G8R8A8_UNORM: 
        return DXGI_FORMAT_B8G8R8A8_TYPELESS;

    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UINT: 
        return DXGI_FORMAT_R8G8B8A8_TYPELESS;

    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_FLOAT: 
        return DXGI_FORMAT_R16G16B16A16_TYPELESS;

    case DXGI_FORMAT_R32G32B32A32_FLOAT: 
        return DXGI_FORMAT_R32G32B32A32_TYPELESS;

    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_FLOAT: 
        return DXGI_FORMAT_R32_TYPELESS;

    case DXGI_FORMAT_D32_FLOAT: 
        return DXGI_FORMAT_R32_TYPELESS;

    case DXGI_FORMAT_D24_UNORM_S8_UINT: 
        return DXGI_FORMAT_R24G8_TYPELESS;

    default:
        throw std::runtime_error("Unsupported texture format");
    }
}

D3D12_RESOURCE_DIMENSION RHI::ConvertTextureTypeToResourceDimension(TextureType type)
{
	switch (type)
	{
	case TextureType::Texture1D:
	case TextureType::Texture1DArray:
		return D3D12_RESOURCE_DIMENSION_TEXTURE1D;

	case TextureType::Texture2D:
	case TextureType::Texture2DArray:
	case TextureType::TextureCubemap:
	case TextureType::TextureCubemapArray:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	case TextureType::Texture3D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE3D;

	default:
		return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	}
}

D3D12_SRV_DIMENSION RHI::ConvertTextureTypeToSRVDimension(TextureType type)
{
	switch (type)
	{
	case TextureType::Texture1D:
		return D3D12_SRV_DIMENSION_TEXTURE1D;

	case TextureType::Texture1DArray:
		return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;

	case TextureType::Texture2D:
		return D3D12_SRV_DIMENSION_TEXTURE2D;

	case TextureType::Texture2DArray:
		return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;

	case TextureType::TextureCubemap:
		return D3D12_SRV_DIMENSION_TEXTURECUBE;

	case TextureType::TextureCubemapArray:
		return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;

	case TextureType::Texture3D:
		return D3D12_SRV_DIMENSION_TEXTURE3D;

	default:
		return D3D12_SRV_DIMENSION_UNKNOWN;
	}
}

D3D12_RESOURCE_STATES RHI::ConvertTextureLayoutToResourceState(TextureLayout layout)
{
	switch (layout)
	{
	case TextureLayout::Undefined:
		return D3D12_RESOURCE_STATE_COMMON;

	case TextureLayout::General:
		return D3D12_RESOURCE_STATE_GENERIC_READ;

	case TextureLayout::ColorAttachmentOptimal:
		return D3D12_RESOURCE_STATE_RENDER_TARGET;

	case TextureLayout::DepthStencilAttachmentOptimal:
		return D3D12_RESOURCE_STATE_DEPTH_WRITE;

	case TextureLayout::DepthStencilReadOnlyOptimal:
		return D3D12_RESOURCE_STATE_DEPTH_READ;

	case TextureLayout::ShaderReadOnlyOptimal:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	case TextureLayout::TransferSourceOptimal:
		return D3D12_RESOURCE_STATE_COPY_SOURCE;

	case TextureLayout::TransferDestinationOptimal:
		return D3D12_RESOURCE_STATE_COPY_DEST;

	case TextureLayout::Reinitialized:
		return D3D12_RESOURCE_STATE_COMMON;

	case TextureLayout::DepthReadOnlyStencilAttachmentOptimal:
		return D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_DEPTH_WRITE;

	case TextureLayout::DepthAttachmentStencilReadOnlyOptimal:
		return D3D12_RESOURCE_STATE_DEPTH_WRITE | D3D12_RESOURCE_STATE_DEPTH_READ;

	case TextureLayout::DepthAttachmentOptimal:
		return D3D12_RESOURCE_STATE_DEPTH_WRITE;

	case TextureLayout::DepthReadOnlyOptimal:
		return D3D12_RESOURCE_STATE_DEPTH_READ;

	case TextureLayout::StencilAttachmentOptimal:
		return D3D12_RESOURCE_STATE_DEPTH_WRITE;

	case TextureLayout::StencilReadOnlyOptimal:
		return D3D12_RESOURCE_STATE_DEPTH_READ;

	case TextureLayout::ReadOnlyOptimal:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	case TextureLayout::AttachmentOptimal:
		return D3D12_RESOURCE_STATE_RENDER_TARGET;

	case TextureLayout::Present:
		return D3D12_RESOURCE_STATE_PRESENT;

	default:
		return D3D12_RESOURCE_STATE_COMMON;
	}
}
