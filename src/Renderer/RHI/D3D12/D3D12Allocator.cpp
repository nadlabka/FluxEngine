#include "stdafx.h"
#include "D3D12Allocator.h"
#include "D3D12Device.h"
#include "D3D12Adapter.h"
#include "D3D12Texture.h"
#include "D3D12Buffer.h"

RHI::D3D12Allocator::D3D12Allocator(std::shared_ptr<IDevice> device, std::shared_ptr<IAdapter> adapter)
{
	auto d3d12Device = std::static_pointer_cast<D3D12Device>(device);
	auto d3d12Adapter = std::static_pointer_cast<D3D12Adapter>(adapter);

	D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
	allocatorDesc.pDevice = d3d12Device->m_device.ptr();
	allocatorDesc.pAdapter = d3d12Adapter->m_adapter.ptr();
	// These flags are optional but recommended.
	allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED |
		D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;

	ThrowIfFailed(D3D12MA::CreateAllocator(&allocatorDesc, &m_allocator));
}

std::shared_ptr<RHI::ITexture> RHI::D3D12Allocator::CreateTexture(const TextureDescription& desc) const
{
	auto format = ConvertFormatToD3D12(desc.format);
	auto mainResourceFormat = format;

	const bool isDS = ((desc.aspect & eTextureAspect_HasDepth) ||
						desc.aspect & eTextureAspect_HasStencil);

	if (isDS && (desc.usage & eTextureUsage_Sampled)) 
	{
		mainResourceFormat = GetTyplessVersionOfFormat(format);
	}

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = ConvertTextureTypeToResourceDimension(desc.type);
	resourceDesc.Alignment = 0;
	resourceDesc.Width = desc.width;
	resourceDesc.Height = desc.height;
	resourceDesc.DepthOrArraySize = desc.arrayLayers;
	resourceDesc.MipLevels = desc.mipLevels;
	resourceDesc.Format = mainResourceFormat;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = (desc.usage & eTextureUsage_Storage) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

	D3D12_CLEAR_VALUE optimizedClearValue
	{
		.Format = format
	};

	D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
	if (isDS)
	{
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		optimizedClearValue.DepthStencil = { desc.clearDepthValue , desc.clearStencilValue };
	}
	else  if (desc.usage & eTextureUsage_ColorAttachment)
	{
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		std::copy(desc.clearColor.begin(), desc.clearColor.end(), optimizedClearValue.Color);
	}

	initialState = ConvertTextureLayoutToResourceState(desc.layout);

	RscPtr<D3D12MA::Allocation> allocation;

	D3D12MA::ALLOCATION_DESC allocDesc = {};
	allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = m_allocator->CreateResource(
		&allocDesc,
		&resourceDesc,
		initialState,
		(resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL || resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) ? &optimizedClearValue : nullptr,
		&allocation,
		IID_NULL, NULL);

	TextureDimensionsInfo textureDimensions;
	textureDimensions.m_arrayLayers = desc.arrayLayers;
	textureDimensions.m_depth = desc.depth;
	textureDimensions.m_height = desc.height;
	textureDimensions.m_width = desc.width;
	textureDimensions.m_mipLevels = desc.mipLevels;

	auto resultTexture = std::make_shared<D3D12Texture>(textureDimensions, allocation, initialState);
	resultTexture->AllocateDescriptorsInHeaps(desc);

	return resultTexture;
}

std::shared_ptr<RHI::IBuffer> RHI::D3D12Allocator::CreateBuffer(const BufferDescription& desc) const
{
	uint32_t bufferWidth = desc.elementsNum * desc.elementStride;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = (desc.usage & BufferUsage::UniformBuffer) ? ((desc.unstructuredSize + 255) & ~255) : desc.unstructuredSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = desc.flags.isMutable ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

	RscPtr<D3D12MA::Allocation> allocation;

	D3D12MA::ALLOCATION_DESC allocationDesc = {};
	allocationDesc.HeapType = ConvertBufferAccessToD3D12HeapType(desc.access);
	
	D3D12_RESOURCE_STATES initialState = GetD3D12ResourceStateFromDescription(desc);

	HRESULT hr = m_allocator->CreateResource(
		&allocationDesc,
		&resourceDesc,
		initialState,
		NULL,
		&allocation,
		IID_NULL, NULL);

	auto resultBuffer = std::make_shared<D3D12Buffer>(desc.elementsNum, desc.elementStride, allocation, initialState);
	return resultBuffer;
}
