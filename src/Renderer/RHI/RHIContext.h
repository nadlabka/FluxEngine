#pragma once
#include <stdafx.h>
#include "Adapter.h"
#include "Factory.h"
#include "Device.h"

namespace RHI
{
	enum class ERHIRenderingAPI
	{
		eAPI_None = 0,
		eAPI_Vulkan,
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
		ERHIRenderingAPI GetCurrentAPI() { return currentRHI; }

	private:
		RHIContext() : currentRHI(ERHIRenderingAPI::eAPI_None) {}

		void InitD3D12(const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc);
		void InitVulkan(const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc);

		ERHIRenderingAPI currentRHI;

		std::shared_ptr<IFactory> m_factory;
		std::shared_ptr<IAdapter> m_adapter;
		std::shared_ptr<IDevice> m_device;
	};
}