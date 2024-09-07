#pragma once
#include <stdafx.h>
#include "Adapter.h"
#include "Factory.h"
#include "Device.h"

namespace RHI
{
	enum class ERHIRenderingAPI
	{
		eAPI_Vulkan = 0,
		eAPI_D3D12
	};


	class RHIContext
	{
	public:
		static RHIContext& GetInstance()
		{
			static RHIContext instance;
			return instance;
		}

		RHIContext(const RHIContext&) = delete;
		RHIContext& operator=(const RHIContext&) = delete;

		void Init(ERHIRenderingAPI api, const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc);

	private:
		RHIContext() {};

		void InitD3D12(const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc);
		void InitVulkan(const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc);

		std::shared_ptr<IFactory> m_factory;
		std::shared_ptr<IAdapter> m_adapter;
		std::shared_ptr<IDevice> m_device;
	};
}