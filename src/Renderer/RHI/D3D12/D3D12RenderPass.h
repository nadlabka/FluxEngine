#pragma once
#include "../RenderPass.h"

namespace RHI
{
	struct D3D12RenderPass : public IRenderPass
	{
		D3D12RenderPass(const RenderPassDesc& desc);

		RenderPassDesc m_description;
	}; 
}