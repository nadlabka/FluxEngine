#pragma once
#include "../RenderPass.h"

namespace RHI
{
	struct D3D12RenderPass : public IRenderPass
	{
		D3D12RenderPass(const RenderPassDesc& desc);

		void SetAttachments(const std::vector<SubResourceRTsDescription>& colorRTs, const SubResourceRTsDescription& depthStencilRT);

		RenderPassDesc m_description;
		std::vector<SubResourceRTsDescription> m_colorRTs;
		SubResourceRTsDescription m_depthStencilRT;
	}; 
}