#include "stdafx.h"
#include "D3D12CommandBuffer.h"
#include "D3D12CommandQueue.h"
#include <DebugMacros.h>
#include "D3D12PipelineLayout.h"
#include "D3D12Texture.h"
#include "D3D12Buffer.h"

RHI::D3D12CommandBuffer::D3D12CommandBuffer(RscPtr<ID3D12CommandAllocator> commandAllocator, RscPtr<ID3D12GraphicsCommandList> commandList) 
	: m_commandAllocator(commandAllocator), m_commandList(commandList), m_fenceValue(0u)
{
	m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

RHI::D3D12CommandBuffer::~D3D12CommandBuffer()
{
	CloseHandle(m_fenceEvent);
}

void RHI::D3D12CommandBuffer::BindRenderPipeline(std::shared_ptr<IRenderPipeline> renderPipeline)
{
	m_currentRenderPipeline = std::static_pointer_cast<D3D12RenderPipeline>(renderPipeline);
}

void RHI::D3D12CommandBuffer::SubmitToQueue(std::shared_ptr<ICommandQueue> commandQueue)
{
	auto d3d12CommandQueue = std::static_pointer_cast<D3D12CommandQueue>(commandQueue);

	ID3D12CommandList* ppCommandLists[] = { m_commandList.ptr() };
	d3d12CommandQueue->m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_fenceValue = d3d12CommandQueue->SignalFence();
}

void RHI::D3D12CommandBuffer::BeginRecording(std::shared_ptr<ICommandQueue> commandQueue)
{
	auto d3d12CommandQueue = std::static_pointer_cast<D3D12CommandQueue>(commandQueue);

	d3d12CommandQueue->WaitForFenceValue(m_fenceValue, m_fenceEvent);

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.ptr(), m_currentRenderPipeline->m_pipelineState.ptr());

	auto d3d12PipelineLayout = std::static_pointer_cast<D3D12PipelineLayout>(m_currentRenderPipeline->m_description.pipelineLayout);

	for (auto& bufferBinding : d3d12PipelineLayout->m_description.buffersBindings)
	{
		auto d3d12Buffer = std::static_pointer_cast<D3D12Buffer>(bufferBinding.first);
		auto bufferPtr = d3d12Buffer->m_buffer.ptr() ? d3d12Buffer->m_buffer.ptr() : d3d12Buffer->m_allocation->GetResource();
		auto targetState = ConvertDescriptorBindingToResourceState(bufferBinding.second);
		if (d3d12Buffer->m_resourceState != targetState)
		{
			auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(bufferPtr, d3d12Buffer->m_resourceState, targetState);
			m_commandList->ResourceBarrier(1, &transitedRT);
			d3d12Buffer->m_resourceState = targetState;
		}

	}
	for (auto& textureBinding : d3d12PipelineLayout->m_description.texturesBindings)
	{
		auto d3d12Texture = std::static_pointer_cast<D3D12Texture>(textureBinding.first);
		auto texturePtr = d3d12Texture->m_texture.ptr() ? d3d12Texture->m_texture.ptr() : d3d12Texture->m_allocation->GetResource();
		auto targetState = ConvertDescriptorBindingToResourceState(textureBinding.second);
		if (d3d12Texture->m_resourceState != targetState)
		{
			auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(texturePtr, d3d12Texture->m_resourceState, targetState);
			m_commandList->ResourceBarrier(1, &transitedRT);
			d3d12Texture->m_resourceState = targetState;
		}
	}

	m_commandList->SetGraphicsRootSignature(d3d12PipelineLayout->m_rootSignature.ptr());

	auto& descHeapsMgr = DescriptorsHeapsManager::GetInstance();
	auto rtv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto dsv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

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
		for (auto& sliceRT : d3d12RenderPass->m_colorRTs[i].slicesToInclude)
		{
			for (auto& mipRT : sliceRT.mipsToInclude)
			{
				auto rtvHandle = rtv_heap->GetCpuHandle(d3d12ColorRT->m_RTVDescriptorsIndices[mipRT]);
				colorRTsHandles.push_back(rtvHandle);
				switch (attachmentDesc.loadOp)
				{
				case LoadAccessOperation::Load:
					break;
				case LoadAccessOperation::Clear:
					m_commandList->ClearRenderTargetView(rtvHandle, attachmentDesc.clearColor.data(), 0, nullptr);
					break;
				case LoadAccessOperation::DontCare:
					D3D12_DISCARD_REGION discardRegion = {};
					discardRegion.NumRects = 0;
					discardRegion.pRects = nullptr;
					discardRegion.FirstSubresource = D3D12CalcSubresource(mipRT, sliceRT.sliceIndex, 0, d3d12ColorRT->m_dimensionsInfo.m_mipLevels, d3d12ColorRT->m_dimensionsInfo.m_arrayLayers);
					discardRegion.NumSubresources = 1;
					m_commandList->DiscardResource(texturePtr, &discardRegion);
					break;
				}
			}
		}
	}

	m_commandList->OMSetRenderTargets(colorRTsHandles.size(), colorRTsHandles.data(), FALSE, depthStencilHandlePtr);
}

void RHI::D3D12CommandBuffer::EndRecording()
{
	auto& descHeapsMgr = DescriptorsHeapsManager::GetInstance();
	auto rtv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto dsv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	auto d3d12RenderPass = std::static_pointer_cast<D3D12RenderPass>(m_currentRenderPipeline->m_description.renderPass);

	auto d3d12DepthStencilRT = std::static_pointer_cast<D3D12Texture>(d3d12RenderPass->m_depthStencilRT);
	if (d3d12RenderPass->m_description.depthStencilAttachment.has_value())
	{
		auto& attachmentDesc = d3d12RenderPass->m_description.depthStencilAttachment;
		auto texturePtr = d3d12DepthStencilRT->m_texture.ptr() ? d3d12DepthStencilRT->m_texture.ptr() : d3d12DepthStencilRT->m_allocation->GetResource();
		auto targetState = ConvertTextureLayoutToResourceState(attachmentDesc->finalLayout);
		if (d3d12DepthStencilRT->m_resourceState != targetState)
		{
			auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(texturePtr, d3d12DepthStencilRT->m_resourceState, targetState);
			m_commandList->ResourceBarrier(1, &transitedRT);
			d3d12DepthStencilRT->m_resourceState = targetState;
		}
	}

	for (int i = 0; i < d3d12RenderPass->m_description.colorAttachments.size(); i++)
	{
		auto& attachmentDesc = d3d12RenderPass->m_description.colorAttachments[i];
		auto d3d12ColorRT = std::static_pointer_cast<D3D12Texture>(d3d12RenderPass->m_colorRTs[i].texture);
		auto texturePtr = d3d12ColorRT->m_texture.ptr() ? d3d12ColorRT->m_texture.ptr() : d3d12ColorRT->m_allocation->GetResource();

		auto targetState = ConvertTextureLayoutToResourceState(attachmentDesc.finalLayout);
		if (d3d12ColorRT->m_resourceState != targetState)
		{
			auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(texturePtr, d3d12ColorRT->m_resourceState, targetState);
			m_commandList->ResourceBarrier(1, &transitedRT);
			d3d12ColorRT->m_resourceState = targetState;
		}
	}

	m_commandList->Close();
}

void RHI::D3D12CommandBuffer::SetViewport(const ViewportInfo& viewportInfo)
{
	D3D12_VIEWPORT viewport = {};
	viewport.TopLeftX = viewportInfo.topLeftX;
	viewport.TopLeftY = viewportInfo.topLeftY;
	viewport.Width = viewportInfo.width;
	viewport.Height = viewportInfo.height;
	viewport.MinDepth = viewportInfo.minDepth;
	viewport.MaxDepth = viewportInfo.maxDepth;

	m_commandList->RSSetViewports(1, &viewport);
}

void RHI::D3D12CommandBuffer::SetScissors(const ScissorsRect& scissorsRect)
{
	D3D12_RECT scissorRectD3D = {};
	scissorRectD3D.left = scissorsRect.offsetX;
	scissorRectD3D.top = scissorsRect.offsetY;
	scissorRectD3D.right = scissorsRect.offsetX + scissorsRect.extentX;
	scissorRectD3D.bottom = scissorsRect.offsetY + scissorsRect.extentY;

	m_commandList->RSSetScissorRects(1, &scissorRectD3D);
}

void RHI::D3D12CommandBuffer::SetBlendConstants(const std::array<float, 4> constantsValues)
{
	m_commandList->OMSetBlendFactor(constantsValues.data());
}

void RHI::D3D12CommandBuffer::SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
{
	m_commandList->IASetPrimitiveTopology(ConvertPrimitiveTopologyToD3D12(primitiveTopology));
}

void RHI::D3D12CommandBuffer::SetVertexBuffer(std::shared_ptr<IBuffer> buffer, uint32_t slot, const BufferBindDescription& bufferBindDesc)
{
	auto& inputAssemblerLayout = m_currentRenderPipeline->m_description.inputAssemblerLayout;
	auto d3d12Buffer = std::static_pointer_cast<D3D12Buffer>(buffer);
	auto bufferPtr = d3d12Buffer->m_buffer.ptr() ? d3d12Buffer->m_buffer.ptr() : d3d12Buffer->m_allocation->GetResource();

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {}; 
	vertexBufferView.BufferLocation = bufferPtr->GetGPUVirtualAddress() + bufferBindDesc.offset;
	vertexBufferView.StrideInBytes = inputAssemblerLayout.vertexBindings[slot].stride;
	vertexBufferView.SizeInBytes = bufferBindDesc.size;

	m_commandList->IASetVertexBuffers(slot, 1, &vertexBufferView);
}

void RHI::D3D12CommandBuffer::SetIndexBuffer(std::shared_ptr<IBuffer> buffer, const BufferBindDescription& bufferBindDesc)
{
	auto& inputAssemblerLayout = m_currentRenderPipeline->m_description.inputAssemblerLayout;
	auto d3d12Buffer = std::static_pointer_cast<D3D12Buffer>(buffer);
	auto bufferPtr = d3d12Buffer->m_buffer.ptr() ? d3d12Buffer->m_buffer.ptr() : d3d12Buffer->m_allocation->GetResource();

	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.BufferLocation = bufferPtr->GetGPUVirtualAddress() + bufferBindDesc.offset;
	indexBufferView.SizeInBytes = bufferBindDesc.size;
	indexBufferView.Format = d3d12Buffer->m_elementStride == 4 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	m_commandList->IASetIndexBuffer(&indexBufferView);
}

void RHI::D3D12CommandBuffer::DrawInstanced(const InstancedDrawInfo& instancedDrawInfo)
{
	m_commandList->DrawInstanced(
		instancedDrawInfo.verticesPerInstanceNum,
		instancedDrawInfo.instancesNum,
		instancedDrawInfo.startVertex,
		instancedDrawInfo.firstInstance);
}

void RHI::D3D12CommandBuffer::DrawIndexedInstanced(const IndexedInstancedDrawInfo& indexedInstancedDrawInfo)
{
	m_commandList->DrawIndexedInstanced(
		indexedInstancedDrawInfo.indicesPerInstanceNum,
		indexedInstancedDrawInfo.instancesNum,
		indexedInstancedDrawInfo.firstIndex,
		indexedInstancedDrawInfo.startVertex,
		indexedInstancedDrawInfo.firstInstance);
}

void RHI::D3D12CommandBuffer::CopyDataBetweenBuffers(std::shared_ptr<IBuffer> fromBuffer, std::shared_ptr<IBuffer> toBuffer, const BufferRegionCopyDescription& regionCopyDesc)
{
	auto fromD3D12Buffer = std::static_pointer_cast<D3D12Buffer>(fromBuffer);
	auto toD3D12Buffer = std::static_pointer_cast<D3D12Buffer>(toBuffer);

	auto fromBufferPtr = fromD3D12Buffer->m_buffer.ptr() ? fromD3D12Buffer->m_buffer.ptr() : fromD3D12Buffer->m_allocation->GetResource();
	if (fromD3D12Buffer->m_resourceState != D3D12_RESOURCE_STATE_COPY_SOURCE)
	{
		auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(fromBufferPtr, fromD3D12Buffer->m_resourceState, D3D12_RESOURCE_STATE_COPY_SOURCE);
		m_commandList->ResourceBarrier(1, &transitedRT);
		fromD3D12Buffer->m_resourceState = D3D12_RESOURCE_STATE_COPY_SOURCE;
	}

	auto toBufferPtr = toD3D12Buffer->m_buffer.ptr() ? toD3D12Buffer->m_buffer.ptr() : toD3D12Buffer->m_allocation->GetResource();
	if (toD3D12Buffer->m_resourceState != D3D12_RESOURCE_STATE_COPY_DEST)
	{
		auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(toBufferPtr, toD3D12Buffer->m_resourceState, D3D12_RESOURCE_STATE_COPY_DEST);
		m_commandList->ResourceBarrier(1, &transitedRT);
		toD3D12Buffer->m_resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	m_commandList->CopyBufferRegion(
		toD3D12Buffer->m_buffer.ptr(),
		regionCopyDesc.destOffset,
		fromD3D12Buffer->m_buffer.ptr(),
		regionCopyDesc.srcOffset,
		regionCopyDesc.width
	);
}
