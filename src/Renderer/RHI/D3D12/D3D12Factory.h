#pragma once
#include "../Factory.h"
#include "../Surface.h"

namespace RHI
{
	struct D3D12Factory : IFactory
	{
		D3D12Factory();
		~D3D12Factory();

		std::shared_ptr<IAdapter> CreateAdapter(AdapterCreateDesc adapterDesc) const;
		std::shared_ptr<ISwapchain> CreateSwapchain(std::shared_ptr<ISurface> surface, std::shared_ptr<ICommandQueue> commandQueue, uint32_t framesCount) const;

		RscPtr<IDXGIFactory4> m_factory;
	};
}