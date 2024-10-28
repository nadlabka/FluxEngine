#include "stdafx.h"
#include "D3D12CommandBuffer.h"
#include "D3D12CommandQueue.h"
#include <DebugMacros.h>

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
