#pragma once
#include <stdafx.h>
#include "Adapter.h"
#include "Factory.h"
#include "Device.h"
#include "Allocator.h"

namespace RHI
{
	enum class ERHIRenderingAPI
	{
		None = 0,
		Vulkan,
		D3D12
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
		void Destroy();
		ERHIRenderingAPI GetCurrentAPI() { return currentAPI; }

		std::shared_ptr<IFactory> GetFactory() { return m_factory; }
		std::shared_ptr<IAdapter> GetAdapter() { return m_adapter; }
		std::shared_ptr<IDevice> GetDevice() { return m_device; }
		std::shared_ptr<IAllocator> GetAllocator() { return m_allocator; }

	private:
		RHIContext() : currentAPI(ERHIRenderingAPI::None) {}

		void InitD3D12(const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc);
		void InitVulkan(const AdapterCreateDesc& adapterDesc, const DeviceCreateDesc& deviceDesc);

		ERHIRenderingAPI currentAPI;

		std::shared_ptr<IFactory> m_factory;
		std::shared_ptr<IAdapter> m_adapter;
		std::shared_ptr<IDevice> m_device;
		std::shared_ptr<IAllocator> m_allocator;
	};
}