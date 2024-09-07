#pragma once
#include <stdafx.h>
#include "Adapter.h"
#include <memory>

namespace RHI
{
	struct IFactory
	{
		virtual ~IFactory() {}

		virtual std::shared_ptr<IAdapter> CreateAdapter(AdapterCreateDesc adapterDesc) = 0;
	};
}