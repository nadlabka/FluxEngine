#include "stdafx.h"
#include "D3D12Allocator.h"
#include "D3D12Device.h"
#include "D3D12Adapter.h"
#include "D3D12Texture.h"

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

std::shared_ptr<RHI::ITexture> RHI::D3D12Allocator::CreateTexture(const TextureDesc& desc) const
{
	auto format = convertFormatToD3D12(desc.format);
	auto mainResourceFormat = format;

	const bool isDS = ((desc.aspect & eTextureAspect_HasDepth) ||
						desc.aspect & eTextureAspect_HasStencil);

	// we cannot sample depth textures directly
	// instead, we have to create this resource as Typeless, and then cast it to compatbile
	// formats for the SRVs
	if (isDS && (desc.usage & eTextureUsage_Sampled)) {
		mainResourceFormat = GetTyplessVersionOfFormat(format);
	}

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
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

	m_mipLevels = desc.mipLevels;
	numLayers = config.arrayLayers;


	D3D12_CLEAR_VALUE optimizedClearValue = {
		.Format = format,
	};

	nativeState = D3D12_RESOURCE_STATE_COMMON;
	if (isDS) {
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		optimizedClearValue.DepthStencil = { config.optimizedClearValue[0], 0 };
		if (!config.usage.Sampled) {
			nativeState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
		}
	}

	if (config.usage.ColorAttachment) {
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		std::fill(optimizedClearValue.Color, optimizedClearValue.Color + std::size(optimizedClearValue.Color), 0);
		//nativeState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
	}


	D3D12MA::ALLOCATION_DESC allocDesc = {};
	allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

	// allocate the resource

	if (nativeState == D3D12_RESOURCE_STATE_COMMON) {
		nativeState = D3D12_RESOURCE_STATE_GENERIC_READ;
	}

	HRESULT hr = owningDevice->allocator->CreateResource(
		&allocDesc, &resourceDesc,
		nativeState, (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL || resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) ? &optimizedClearValue : nullptr,
		&allocation, IID_PPV_ARGS(&texture));

	std::wstring wide;
	wide.resize(config.debugName.data() == nullptr ? 0 : config.debugName.length());
	MultiByteToWideChar(CP_UTF8, 0, config.debugName.data(), -1, wide.data(), wide.size());
	texture->SetName(wide.data());

	if (config.debugName.data() != nullptr) {
		debugName = config.debugName;
	}

	// add the resource to the appropriate heaps
	PlaceInHeaps(owningDevice, format, config);

	return std::shared_ptr<ITexture>();
}
