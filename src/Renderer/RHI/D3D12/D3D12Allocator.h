#pragma once
#include <memory>
#include "../Allocator.h"
#include "../Device.h"
#include "../Adapter.h"

namespace RHI
{
	struct D3D12Allocator : public IAllocator
	{
		D3D12Allocator(std::shared_ptr<IDevice>, std::shared_ptr<IAdapter>);

		std::shared_ptr<ITexture> CreateTexture(const TextureDescription& desc) const;
		std::shared_ptr<IBuffer> CreateBuffer(const BufferDescription& desc) const;

		RscPtr<D3D12MA::Allocator> m_allocator;
	};
}