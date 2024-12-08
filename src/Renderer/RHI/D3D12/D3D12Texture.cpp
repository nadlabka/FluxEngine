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
	for (auto& idx : m_RTVDescriptorsIndices)
	{
		rtv_heap->EraseIndex(idx);
	}
	for (auto& idx : m_DSVDescriptorIndices)
	{
		dsv_heap->EraseIndex(idx);
	}
	if (m_SRVDescriptorIndex != D3D12DescriptorHeap::INDEX_INVALID)
	{
		cbv_srv_uav_heap->EraseIndex(m_SRVDescriptorIndex);
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

	auto format = ConvertFormatToD3D12(desc.format);

	ID3D12Resource* resourcePtr = m_texture.ptr() ? m_texture.ptr() : m_allocation->GetResource();

	if (isDS)
	{
		for (uint32_t i = 0; i < desc.arrayLayers; i++)
		{
			for (uint32_t j = 0; j < desc.mipLevels; j++)
			{
				uint32_t dsvIDX = dsv_heap->AllocateIndex();

				D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
				dsv_desc.Format = format;

				if (desc.arrayLayers > 1)
				{
					dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
					dsv_desc.Texture2DArray.ArraySize = 1;
					dsv_desc.Texture2DArray.FirstArraySlice = i;
					dsv_desc.Texture2DArray.MipSlice = j;
				}
				else
				{
					dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					dsv_desc.Texture2D.MipSlice = j;
				}

				auto handle = dsv_heap->GetCpuHandle(dsvIDX);

				d3d12device->m_device->CreateDepthStencilView(resourcePtr, &dsv_desc, handle);

				m_DSVDescriptorIndices.push_back(dsvIDX);
			}
		}
	}
	if (desc.usage & eTextureUsage_ColorAttachment)
	{
		for (uint32_t i = 0; i < desc.arrayLayers; i++)
		{
			for (uint32_t j = 0; j < desc.mipLevels; j++)
			{
				uint32_t rtvIDX = rtv_heap->AllocateIndex();

				D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
				rtv_desc.Format = format;

				if (desc.arrayLayers > 1)
				{
					rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
					rtv_desc.Texture2DArray.ArraySize = 1;
					rtv_desc.Texture2DArray.FirstArraySlice = i;
					rtv_desc.Texture2DArray.MipSlice = j;
				}
				else
				{
					rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
					rtv_desc.Texture2D.MipSlice = j;
				}

				auto handle = rtv_heap->GetCpuHandle(rtvIDX);

				d3d12device->m_device->CreateRenderTargetView(resourcePtr, &rtv_desc, handle);

				m_RTVDescriptorsIndices.push_back(rtvIDX);
			}
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
		
		switch (desc.type)
		{
		case TextureType::Texture2D:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = desc.mipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			break;

		case TextureType::TextureCubemap:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = desc.mipLevels;
			srvDesc.TextureCube.MostDetailedMip = 0;
			break;

		case TextureType::Texture2DArray:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.MipLevels = desc.mipLevels;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			srvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.Texture2DArray.ArraySize = desc.arrayLayers;
			break;
		
		default:
			throw std::runtime_error("this texture type is not yet supported");
		}

		auto handle = cbv_srv_uav_heap->GetCpuHandle(srvIDX);
		d3d12device->m_device->CreateShaderResourceView(resourcePtr, &srvDesc, handle);

		m_SRVDescriptorIndex = srvIDX;
	}
	if (desc.usage & eTextureUsage_Storage)
	{
		for (uint32_t layer = 0; layer < desc.arrayLayers; layer++)
		{
			for (uint32_t mip = 0; mip < desc.mipLevels; mip++)
			{
				uint32_t uavIndex = cbv_srv_uav_heap->AllocateIndex();

				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = format;

				if (desc.arrayLayers > 1)
				{
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					uavDesc.Texture2DArray.ArraySize = 1;
					uavDesc.Texture2DArray.FirstArraySlice = layer;
					uavDesc.Texture2DArray.MipSlice = mip;
					uavDesc.Texture2DArray.PlaneSlice = 0;
				}
				else
				{
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					uavDesc.Texture2D.MipSlice = mip;
					uavDesc.Texture2D.PlaneSlice = 0;
				}

				auto handle = cbv_srv_uav_heap->GetCpuHandle(uavIndex);

				d3d12device->m_device->CreateUnorderedAccessView(resourcePtr, nullptr, &uavDesc, handle);

				m_UAVDescriptorsIndices.push_back(uavIndex);
			}
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
