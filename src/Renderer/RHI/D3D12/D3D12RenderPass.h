#pragma once
#include "../RenderPass.h"

namespace RHI
{
	struct D3D12RenderPass : public IRenderPass
	{
		virtual ~D3D12RenderPass() {}
	};
}