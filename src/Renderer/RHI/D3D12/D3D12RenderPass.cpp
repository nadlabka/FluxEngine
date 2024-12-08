#include "stdafx.h"
#include "D3D12RenderPass.h"

RHI::D3D12RenderPass::D3D12RenderPass(const RenderPassDesc& desc) : m_description(desc)
{

}

void RHI::D3D12RenderPass::SetAttachments(const std::vector<SubResourceRTsDescription>& colorRTs, const SubResourceRTsDescription& depthStencilRT)
{
	m_colorRTs = colorRTs;
	m_depthStencilRT = depthStencilRT;
}
