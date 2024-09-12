#pragma once
#include <memory>
#include "../Allocator.h"
#include "../Device.h"
#include "../Adapter.h"

namespace RHI
{
	struct D3D12Allocator : IAllocator
	{
		D3D12Allocator(std::shared_ptr<IDevice>, std::shared_ptr<IAdapter>);

		RscPtr<D3D12MA::Allocator> m_allocator;
	};
}