#include "stdafx.h"
#include "D3D12CommandBuffer.h"
#include "D3D12CommandQueue.h"
#include <DebugMacros.h>
#include "D3D12PipelineLayout.h"
#include "D3D12Texture.h"

RHI::D3D12CommandBuffer::D3D12CommandBuffer(RscPtr<ID3D12CommandAllocator> commandAllocator, RscPtr<ID3D12GraphicsCommandList> commandList) 
	: m_commandAllocator(commandAllocator), m_commandList(commandList), m_fenceValue(0u)
{
	m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

RHI::D3D12CommandBuffer::~D3D12CommandBuffer()
{
}

void RHI::D3D12CommandBuffer::BindRenderPipeline(std::shared_ptr<IRenderPipeline> renderPipeline)
{
	m_currentRenderPipeline = std::static_pointer_cast<D3D12RenderPipeline>(renderPipeline);
}

void RHI::D3D12CommandBuffer::SubmitToQueue(std::shared_ptr<ICommandQueue> commandQueue)
{
	auto d3d12CommandQueue = std::static_pointer_cast<D3D12CommandQueue>(commandQueue);

	d3d12CommandQueue->WaitForFenceValue(m_fenceValue, m_fenceEvent);
	m_commandAllocator->Reset();

	ID3D12CommandList* ppCommandLists[] = { m_commandList.ptr() };
	d3d12CommandQueue->m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_fenceValue = d3d12CommandQueue->SignalFence();
}

void RHI::D3D12CommandBuffer::BeginRecording()
{
	m_commandList->Reset(m_commandAllocator.ptr(), m_currentRenderPipeline->m_pipelineState.ptr());

	m_commandList->SetGraphicsRootSignature(std::static_pointer_cast<D3D12PipelineLayout>(m_currentRenderPipeline->m_description.pipelineLayout)->m_rootSignature.ptr());

	auto& descHeapsMgr = DescriptorsHeapsManager::GetInstance();
	auto rtv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto dsv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	auto cbv_srv_uav_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	auto d3d12RenderPass = std::static_pointer_cast<D3D12RenderPass>(m_currentRenderPipeline->m_description.renderPass);
	auto d3d12DepthStencilRT = std::static_pointer_cast<D3D12Texture>(d3d12RenderPass->m_depthStencilRT);
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle = {};
	D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilHandlePtr = nullptr;
	if (d3d12RenderPass->m_description.depthStencilAttachment.has_value())
	{
		auto texturePtr = d3d12DepthStencilRT->m_texture.ptr() ? d3d12DepthStencilRT->m_texture.ptr() : d3d12DepthStencilRT->m_allocation->GetResource();
		auto targetState = ConvertTextureLayoutToResourceState(d3d12RenderPass->m_description.depthStencilAttachment->initialLayout);
		if (d3d12DepthStencilRT->m_resourceState != targetState)
		{
			auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(texturePtr, d3d12DepthStencilRT->m_resourceState, targetState);
			m_commandList->ResourceBarrier(1, &transitedRT);
			d3d12DepthStencilRT->m_resourceState = targetState;
		}
		depthStencilHandle = dsv_heap->GetCpuHandle(d3d12DepthStencilRT->m_DSVDescriptorIndex);
		depthStencilHandlePtr = &depthStencilHandle;
	}
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> colorRTsHandles = {};
	for (int i = 0; i < d3d12RenderPass->m_description.colorAttachments.size(); i++)
	{
		auto& attachmentDesc = d3d12RenderPass->m_description.colorAttachments[i];
		auto d3d12ColorRT = std::static_pointer_cast<D3D12Texture>(d3d12RenderPass->m_colorRTs[i].texture);
		auto texturePtr = d3d12ColorRT->m_texture.ptr() ? d3d12ColorRT->m_texture.ptr() : d3d12ColorRT->m_allocation->GetResource();

		auto targetState = ConvertTextureLayoutToResourceState(attachmentDesc.initialLayout);
		if (d3d12ColorRT->m_resourceState != targetState)
		{
			auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(texturePtr, d3d12ColorRT->m_resourceState, targetState);
			m_commandList->ResourceBarrier(1, &transitedRT);
			d3d12ColorRT->m_resourceState = targetState;
		}
		for (auto& mipRT : d3d12RenderPass->m_colorRTs[i].mipsToInclude)
		{
			colorRTsHandles.push_back(rtv_heap->GetCpuHandle(d3d12ColorRT->m_RTVDescriptorsIndices[i]));
		}
	}
	m_commandList->OMSetRenderTargets(colorRTsHandles.size(), colorRTsHandles.data(), FALSE, depthStencilHandlePtr);
}

void RHI::D3D12CommandBuffer::EndRecording()
{

}

void RHI::D3D12CommandBuffer::SetViewport(const ViewportInfo&)
{

}

void RHI::D3D12CommandBuffer::SetScissors(const ScissorsRect&)
{

}

void RHI::D3D12CommandBuffer::SetBlendConstants(const std::array<float, 4> constantsValues)
{

}

void RHI::D3D12CommandBuffer::DrawInstanced(const InstancedDrawInfo&)
{

}

void RHI::D3D12CommandBuffer::DrawIndexedInstanced(const IndexedInstancedDrawInfo&)
{

}

void RHI::D3D12CommandBuffer::CopyDataBetweenBuffers(std::shared_ptr<IBuffer> fromBuffer, std::shared_ptr<IBuffer> toBuffer)
{
}
