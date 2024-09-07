#pragma once
#include "../Factory.h"

namespace RHI
{
	struct D3D12Factory : IFactory
	{
		D3D12Factory();
		~D3D12Factory();

		std::shared_ptr<IAdapter> CreateAdapter(AdapterCreateDesc adapterDesc);

		RscPtr<IDXGIFactory4> m_factory;
	};
}