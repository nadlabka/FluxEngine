#pragma once
#include <stdafx.h>

namespace RHI
{
	struct D3D12StatefulResource
	{
		D3D12_RESOURCE_STATES m_resourceState = D3D12_RESOURCE_STATE_COMMON;
	};
}