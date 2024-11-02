#pragma once
#include "../RenderPass.h"

namespace RHI
{
	struct D3D12RenderPass : public IRenderPass
	{
		D3D12RenderPass(const RenderPassDesc& desc);

		void SetAttachments(const std::vector<SubResourceRTsDescription>& colorRTs, std::shared_ptr<ITexture> depthStencilRT);

		RenderPassDesc m_description;
		std::vector<SubResourceRTsDescription> m_colorRTs;
		std::shared_ptr<ITexture> m_depthStencilRT;
	}; 
}