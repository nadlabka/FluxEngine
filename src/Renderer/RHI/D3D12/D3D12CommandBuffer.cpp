#include "stdafx.h"
#include "D3D12CommandBuffer.h"
#include "D3D12CommandQueue.h"
#include <DebugMacros.h>
#include "D3D12PipelineLayout.h"
#include "D3D12Texture.h"
#include "D3D12Buffer.h"
#include "D3D12Sampler.h"

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
	m_currentComputePipeline = nullptr;
}

std::shared_ptr<RHI::IRenderPipeline> RHI::D3D12CommandBuffer::GetCurrentRenderPipeline()
{
	return m_currentRenderPipeline;
}

void RHI::D3D12CommandBuffer::BindComputePipeline(std::shared_ptr<IComputePipeline> computePipeline)
{
	m_currentComputePipeline = std::static_pointer_cast<D3D12ComputePipeline>(computePipeline);
	m_currentRenderPipeline = nullptr;
}

std::shared_ptr<RHI::IComputePipeline> RHI::D3D12CommandBuffer::GetCurrentComputePipeline()
{
	return m_currentComputePipeline;
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
    if (!m_currentRenderPipeline && !m_currentComputePipeline)
    {
        m_commandList->Reset(m_commandAllocator.ptr(), nullptr);
        return;
    }
    m_commandList->Reset(m_commandAllocator.ptr(), m_currentRenderPipeline ? m_currentRenderPipeline->m_pipelineState.ptr() : m_currentComputePipeline->m_pipelineState.ptr());

	auto& descHeapsMgr = DescriptorsHeapsManager::GetInstance();

	auto sampler_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)->Heap();
	auto cbv_uav_srv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->Heap();

	ID3D12DescriptorHeap* heaps[] = { cbv_uav_srv_heap, sampler_heap };
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	auto pipelineLayout = m_currentRenderPipeline ? m_currentRenderPipeline->m_description.pipelineLayout : m_currentComputePipeline->m_description.pipelineLayout;
	auto d3d12PipelineLayout = std::static_pointer_cast<D3D12PipelineLayout>(pipelineLayout);
	m_commandList->SetGraphicsRootSignature(d3d12PipelineLayout->m_rootSignature.ptr());
}

void RHI::D3D12CommandBuffer::EndRecording()
{
	m_commandList->Close();
}

void RHI::D3D12CommandBuffer::BindPipelineResources()
{
	auto d3d12PipelineLayout = std::static_pointer_cast<D3D12PipelineLayout>(m_currentRenderPipeline ? m_currentRenderPipeline->m_description.pipelineLayout : m_currentComputePipeline->m_description.pipelineLayout);

	for (const auto& constBufferName : d3d12PipelineLayout->m_pipelineLayoutBindings.m_BoundConstantBuffers.GetConstantBuffersBindingsNames())
	{
		if (constBufferName != BoundConstantBuffers::boundResourcesBufferName)
		{
			auto& bufferBinding = d3d12PipelineLayout->m_pipelineLayoutBindings.m_BoundConstantBuffers.GetConstantBufferBinding(constBufferName);
			auto d3d12Buffer = std::static_pointer_cast<D3D12Buffer>(bufferBinding.buffer);
			auto bufferPtr = d3d12Buffer->m_buffer.ptr() ? d3d12Buffer->m_buffer.ptr() : d3d12Buffer->m_allocation->GetResource();
			auto targetState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

			if (bufferBinding.visibility & BindingVisibility::Fragment)
			{
				targetState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			}
			if (bufferBinding.visibility & ~BindingVisibility::Fragment)
			{
				targetState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			}

			if (d3d12Buffer->m_resourceState != targetState)
			{
				auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(bufferPtr, d3d12Buffer->m_resourceState, targetState);
				m_commandList->ResourceBarrier(1, &transitedRT);
				d3d12Buffer->m_resourceState = targetState;
			}

			auto& descHeapsMgr = DescriptorsHeapsManager::GetInstance();
			auto cbv_uav_srv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			m_commandList->SetGraphicsRootConstantBufferView(bufferBinding.parameterIndex, bufferPtr->GetGPUVirtualAddress());
		}
	}

	auto boundResourcesBufferIndex = d3d12PipelineLayout->m_pipelineLayoutBindings.m_BoundConstantBuffers.GetConstantBufferBinding(BoundConstantBuffers::boundResourcesBufferName).parameterIndex;

	// Bind buffers
	for (const auto& bufferBindingName : d3d12PipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources.GetBuffersBindingsNames())
	{
		auto& bufferBinding = d3d12PipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources.GetBufferBindingMapping(bufferBindingName);
		auto bindingIndex = d3d12PipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources.GetParameterIndex(bufferBindingName);
		auto d3d12Buffer = std::static_pointer_cast<D3D12Buffer>(bufferBinding.first);
		auto bufferPtr = d3d12Buffer->m_buffer.ptr() ? d3d12Buffer->m_buffer.ptr() : d3d12Buffer->m_allocation->GetResource();
		auto targetState = ConvertDescriptorBindingToResourceState(bufferBinding.second);

		if (d3d12Buffer->m_resourceState != targetState)
		{
			auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(bufferPtr, d3d12Buffer->m_resourceState, targetState);
			m_commandList->ResourceBarrier(1, &transitedRT);
			d3d12Buffer->m_resourceState = targetState;
		}

		switch (bufferBinding.second.type)
		{
		case DescriptorResourceType::StorageBuffer:
			m_commandList->SetGraphicsRoot32BitConstant(boundResourcesBufferIndex, d3d12Buffer->m_UAVDescriptorIndex, bindingIndex);
			break;
		case DescriptorResourceType::UniformBuffer:
			m_commandList->SetGraphicsRoot32BitConstant(boundResourcesBufferIndex, d3d12Buffer->m_CBVDescriptorIndex, bindingIndex);
			break;
		case DescriptorResourceType::DataReadOnlyBuffer:
			m_commandList->SetGraphicsRoot32BitConstant(boundResourcesBufferIndex, d3d12Buffer->m_SRVDescriptorIndex, bindingIndex);
			break;
		}
	}

	// Bind textures
	for (const auto& textureBindingName : d3d12PipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources.GetTexturesBindingsNames())
	{
		const auto& textureBinding = d3d12PipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources.GetTextureBindingMapping(textureBindingName);
		auto bindingIndex = d3d12PipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources.GetParameterIndex(textureBindingName);
		auto d3d12Texture = std::static_pointer_cast<D3D12Texture>(textureBinding.first);
		auto texturePtr = d3d12Texture->m_texture.ptr() ? d3d12Texture->m_texture.ptr() : d3d12Texture->m_allocation->GetResource();
		auto targetState = ConvertDescriptorBindingToResourceState(textureBinding.second.resourceBindingInfo);

		if (d3d12Texture->m_resourceState != targetState)
		{
			auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(texturePtr, d3d12Texture->m_resourceState, targetState);
			m_commandList->ResourceBarrier(1, &transitedRT);
			d3d12Texture->m_resourceState = targetState;
		}

		switch (textureBinding.second.resourceBindingInfo.type)
		{
		case DescriptorResourceType::SampledImage:
			m_commandList->SetGraphicsRoot32BitConstant(boundResourcesBufferIndex, d3d12Texture->m_SRVDescriptorIndex, bindingIndex);
			break;
		case DescriptorResourceType::StorageImage:
			uint32_t descriptorIndexInArray = d3d12Texture->m_dimensionsInfo.m_mipLevels * textureBinding.second.subresourceInfo.slice +
				textureBinding.second.subresourceInfo.mip;
			m_commandList->SetGraphicsRoot32BitConstant(boundResourcesBufferIndex, d3d12Texture->m_UAVDescriptorsIndices[descriptorIndexInArray], bindingIndex);
			break;
		}
	}

	// Bind samplers
	for (const auto& samplerBindingName : d3d12PipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources.GetSamplersBindingsNames())
	{
		const auto& sampler = d3d12PipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources.GetSamplerOfBinding(samplerBindingName);
		auto bindingIndex = d3d12PipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources.GetParameterIndex(samplerBindingName);
		auto d3d12Sampler = std::static_pointer_cast<D3D12Sampler>(sampler);
		m_commandList->SetGraphicsRoot32BitConstant(boundResourcesBufferIndex, d3d12Sampler->m_descriptorIndex, bindingIndex);
	}
}

void RHI::D3D12CommandBuffer::BindRenderTargets(bool isToClear)
{
	auto d3d12RenderPass = std::static_pointer_cast<D3D12RenderPass>(m_currentRenderPipeline->m_description.renderPass);
	auto& descHeapsMgr = DescriptorsHeapsManager::GetInstance();
	auto rtv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto dsv_heap = descHeapsMgr.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// Depth/Stencil binding
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle = {};
	D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilHandlePtr = nullptr;
	if (d3d12RenderPass->m_description.depthStencilAttachment.has_value())
	{
		auto d3d12DepthStencilRT = std::static_pointer_cast<D3D12Texture>(d3d12RenderPass->m_depthStencilRT.texture);
		ASSERT(d3d12DepthStencilRT, "No depth-stencil texture was provided or it's nullptr");
		ASSERT(d3d12RenderPass->m_depthStencilRT.slicesToInclude.size() <= 1, "Multiple depth targets are not allowed");
		ASSERT(d3d12RenderPass->m_depthStencilRT.slicesToInclude[0].mipsToInclude.size() <= 1, "Multiple depth targets are not allowed");

		auto texturePtr = d3d12DepthStencilRT->m_texture.ptr() ? d3d12DepthStencilRT->m_texture.ptr() : d3d12DepthStencilRT->m_allocation->GetResource();
		auto targetState = ConvertTextureLayoutToResourceState(d3d12RenderPass->m_description.depthStencilAttachment->initialLayout);
		if (d3d12DepthStencilRT->m_resourceState != targetState)
		{
			auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(texturePtr, d3d12DepthStencilRT->m_resourceState, targetState);
			m_commandList->ResourceBarrier(1, &transitedRT);
			d3d12DepthStencilRT->m_resourceState = targetState;
		}

		uint32_t sliceIndex = d3d12RenderPass->m_depthStencilRT.slicesToInclude[0].sliceIndex;
		uint32_t mipIndex = d3d12RenderPass->m_depthStencilRT.slicesToInclude[0].mipsToInclude[0];
		depthStencilHandle = dsv_heap->GetCpuHandle(d3d12DepthStencilRT->m_DSVDescriptorIndices[sliceIndex * d3d12DepthStencilRT->m_dimensionsInfo.m_mipLevels + mipIndex]);
		depthStencilHandlePtr = &depthStencilHandle;

		if (isToClear)
		{
			switch (d3d12RenderPass->m_description.depthStencilAttachment->loadOp)
			{
			case LoadAccessOperation::Load:
				break;
			case LoadAccessOperation::Clear:
				m_commandList->ClearDepthStencilView(depthStencilHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
					d3d12RenderPass->m_description.depthStencilAttachment->clearDepth, d3d12RenderPass->m_description.depthStencilAttachment->clearStencil, 0, nullptr);
				break;
			case LoadAccessOperation::DontCare:
				D3D12_DISCARD_REGION discardRegion = {};
				discardRegion.NumRects = 0;
				discardRegion.pRects = nullptr;
				discardRegion.FirstSubresource = D3D12CalcSubresource(mipIndex, sliceIndex, 0, d3d12DepthStencilRT->m_dimensionsInfo.m_mipLevels, d3d12DepthStencilRT->m_dimensionsInfo.m_arrayLayers);
				discardRegion.NumSubresources = 1;
				m_commandList->DiscardResource(texturePtr, &discardRegion);
				break;
			}
		}
	}

	// Color render targets binding
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> colorRTsHandles = {};
	for (int i = 0; i < d3d12RenderPass->m_description.colorAttachments.size(); i++)
	{
		auto& attachmentDesc = d3d12RenderPass->m_description.colorAttachments[i];
		auto d3d12ColorRT = std::static_pointer_cast<D3D12Texture>(d3d12RenderPass->m_colorRTs[i].texture);

		ASSERT(d3d12ColorRT, "No render target texture was provided or it's nullptr");

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
				auto rtvHandle = rtv_heap->GetCpuHandle(d3d12ColorRT->m_RTVDescriptorsIndices[sliceRT.sliceIndex * d3d12ColorRT->m_dimensionsInfo.m_mipLevels + mipRT]);
				colorRTsHandles.push_back(rtvHandle);
				if (isToClear)
				{
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
	}

	m_commandList->OMSetRenderTargets(colorRTsHandles.size(), colorRTsHandles.data(), FALSE, depthStencilHandlePtr);
}

void RHI::D3D12CommandBuffer::FinishRenderTargets()
{
	auto d3d12RenderPass = std::static_pointer_cast<D3D12RenderPass>(m_currentRenderPipeline->m_description.renderPass);

	auto d3d12DepthStencilRT = std::static_pointer_cast<D3D12Texture>(d3d12RenderPass->m_depthStencilRT.texture);
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
}

void RHI::D3D12CommandBuffer::ForceWaitUntilFinished(std::shared_ptr<ICommandQueue> commandQueue)
{
	auto d3d12CommandQueue = std::static_pointer_cast<D3D12CommandQueue>(commandQueue);
	d3d12CommandQueue->WaitForFenceValue(m_fenceValue, m_fenceEvent);
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

void RHI::D3D12CommandBuffer::SetBindingResource(const std::string& bindingName, std::shared_ptr<IBuffer> buffer)
{
	auto& currentPipelineLayout = m_currentRenderPipeline ? m_currentRenderPipeline->GetPipelineDescription().pipelineLayout : m_currentComputePipeline->GetPipelineDescription().pipelineLayout;
	auto& dynamicallyBoundResources = currentPipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources;
	dynamicallyBoundResources.SetBufferBindingResource(bindingName, buffer);
}

void RHI::D3D12CommandBuffer::SetBindingResource(const std::string& bindingName, std::shared_ptr<ITexture> texture)
{
	auto& currentPipelineLayout = m_currentRenderPipeline ? m_currentRenderPipeline->GetPipelineDescription().pipelineLayout : m_currentComputePipeline->GetPipelineDescription().pipelineLayout;
	auto& dynamicallyBoundResources = currentPipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources;
	dynamicallyBoundResources.SetTextureBindingResource(bindingName, texture);
}

void RHI::D3D12CommandBuffer::SetBindingResource(const std::string& bindingName, std::shared_ptr<ISampler> sampler)
{
	auto& currentPipelineLayout = m_currentRenderPipeline ? m_currentRenderPipeline->GetPipelineDescription().pipelineLayout : m_currentComputePipeline->GetPipelineDescription().pipelineLayout;
	auto& dynamicallyBoundResources = currentPipelineLayout->m_pipelineLayoutBindings.m_dynamicallyBoundResources;
	dynamicallyBoundResources.SetSamplerToBinding(bindingName, sampler);
}

void RHI::D3D12CommandBuffer::SetConstantBufferBindingMapping(const std::string& bindingName, std::shared_ptr<IBuffer> buffer)
{
	auto& currentPipelineLayout = m_currentRenderPipeline ? m_currentRenderPipeline->GetPipelineDescription().pipelineLayout : m_currentComputePipeline->GetPipelineDescription().pipelineLayout;
	auto& boundConstantBuffers = currentPipelineLayout->m_pipelineLayoutBindings.m_BoundConstantBuffers;
	boundConstantBuffers.SetConstantBufferBindingMapping(bindingName, buffer);
}

void RHI::D3D12CommandBuffer::SetVertexBuffer(std::shared_ptr<IBuffer> buffer, uint32_t slot, const BufferRegionDescription& bufferBindDesc)
{
	auto& inputAssemblerLayout = m_currentRenderPipeline->m_description.inputAssemblerLayout;
	auto d3d12Buffer = std::static_pointer_cast<D3D12Buffer>(buffer);
	auto bufferPtr = d3d12Buffer->m_buffer.ptr() ? d3d12Buffer->m_buffer.ptr() : d3d12Buffer->m_allocation->GetResource();

	if (d3d12Buffer->m_resourceState != D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
	{
		auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(bufferPtr, d3d12Buffer->m_resourceState, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		m_commandList->ResourceBarrier(1, &transitedRT);
		d3d12Buffer->m_resourceState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	}

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {}; 
	vertexBufferView.BufferLocation = bufferPtr->GetGPUVirtualAddress() + bufferBindDesc.offset;
	vertexBufferView.StrideInBytes = inputAssemblerLayout.vertexBindings[slot].stride;
	vertexBufferView.SizeInBytes = bufferBindDesc.size;

	m_commandList->IASetVertexBuffers(slot, 1, &vertexBufferView);
}

void RHI::D3D12CommandBuffer::SetIndexBuffer(std::shared_ptr<IBuffer> buffer, const BufferRegionDescription& bufferBindDesc)
{
	auto& inputAssemblerLayout = m_currentRenderPipeline->m_description.inputAssemblerLayout;
	auto d3d12Buffer = std::static_pointer_cast<D3D12Buffer>(buffer);
	auto bufferPtr = d3d12Buffer->m_buffer.ptr() ? d3d12Buffer->m_buffer.ptr() : d3d12Buffer->m_allocation->GetResource();

	if (d3d12Buffer->m_resourceState != D3D12_RESOURCE_STATE_INDEX_BUFFER)
	{
		auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(bufferPtr, d3d12Buffer->m_resourceState, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		m_commandList->ResourceBarrier(1, &transitedRT);
		d3d12Buffer->m_resourceState = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	}

	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.BufferLocation = bufferPtr->GetGPUVirtualAddress() + bufferBindDesc.offset;
	indexBufferView.SizeInBytes = bufferBindDesc.size;
	indexBufferView.Format = d3d12Buffer->m_elementStride == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;

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
		toBufferPtr,
		regionCopyDesc.destOffset,
		fromBufferPtr,
		regionCopyDesc.srcOffset,
		regionCopyDesc.width
	);
}

// i'm 99.9% sure it shouldn't issue multiple drawcalls, but I have no idea how to do it properly with respect to offset in an upload buffer. FIX LATER
void RHI::D3D12CommandBuffer::CopyDataFromBufferToTexture(std::shared_ptr<IBuffer> fromBuffer, std::shared_ptr<ITexture> toTexture, const TextureRegionCopyDescription& regionCopyDesc)
{
	auto fromD3D12Buffer = std::static_pointer_cast<D3D12Buffer>(fromBuffer);
	auto toD3D12Texture = std::static_pointer_cast<D3D12Texture>(toTexture);

	auto fromBufferPtr = fromD3D12Buffer->m_buffer.ptr() ? fromD3D12Buffer->m_buffer.ptr() : fromD3D12Buffer->m_allocation->GetResource();
	auto toTexturePtr = toD3D12Texture->m_texture.ptr() ? toD3D12Texture->m_texture.ptr() : toD3D12Texture->m_allocation->GetResource();

	if (fromD3D12Buffer->m_resourceState != D3D12_RESOURCE_STATE_COPY_SOURCE)
	{
		auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(fromBufferPtr, fromD3D12Buffer->m_resourceState, D3D12_RESOURCE_STATE_COPY_SOURCE);
		m_commandList->ResourceBarrier(1, &transitedRT);
		fromD3D12Buffer->m_resourceState = D3D12_RESOURCE_STATE_COPY_SOURCE;
	}

	if (toD3D12Texture->m_resourceState != D3D12_RESOURCE_STATE_COPY_DEST)
	{
		auto transitedRT = CD3DX12_RESOURCE_BARRIER::Transition(toTexturePtr, toD3D12Texture->m_resourceState, D3D12_RESOURCE_STATE_COPY_DEST);
		m_commandList->ResourceBarrier(1, &transitedRT);
		toD3D12Texture->m_resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	D3D12_RESOURCE_DESC textureDesc = toTexturePtr->GetDesc();
	UINT numMipLevels = textureDesc.MipLevels;
	UINT64 currentSrcOffset = regionCopyDesc.srcOffset;
	UINT numArraySlices = textureDesc.DepthOrArraySize;

	bool is3DTexture = textureDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D;

	for (UINT arraySlice = 0; arraySlice < numArraySlices; ++arraySlice)
	{
		for (UINT mipLevel = 0; mipLevel < numMipLevels; ++mipLevel)
		{
			UINT mipWidth = static_cast<UINT>(textureDesc.Width) >> mipLevel;
			UINT mipHeight = static_cast<UINT>(textureDesc.Height) >> mipLevel;
			UINT mipDepth = is3DTexture ? static_cast<UINT>(textureDesc.DepthOrArraySize) >> mipLevel : 1U;

			UINT rowPitch = mipWidth * toD3D12Texture->m_dimensionsInfo.m_formatPixelSizeBytes;

			if (mipWidth * toD3D12Texture->m_dimensionsInfo.m_formatPixelSizeBytes >= D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
			{
				rowPitch = (rowPitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
			}

			D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
			srcLocation.pResource = fromBufferPtr;
			srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			srcLocation.PlacedFootprint.Offset = currentSrcOffset;
			srcLocation.PlacedFootprint.Footprint.Format = textureDesc.Format;
			srcLocation.PlacedFootprint.Footprint.Width = mipWidth;
			srcLocation.PlacedFootprint.Footprint.Height = mipHeight;
			srcLocation.PlacedFootprint.Footprint.Depth = mipDepth;
			srcLocation.PlacedFootprint.Footprint.RowPitch = rowPitch;

			UINT subresourceIndex = D3D12CalcSubresource(mipLevel, arraySlice, 0, numMipLevels, numArraySlices);

			D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
			dstLocation.pResource = toTexturePtr;
			dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dstLocation.SubresourceIndex = subresourceIndex;

			D3D12_BOX srcBox = {};
			srcBox.left = 0;
			srcBox.top = 0;
			srcBox.front = 0;
			srcBox.right = mipWidth;
			srcBox.bottom = mipHeight;
			srcBox.back = mipDepth;

			m_commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &srcBox);

			UINT64 totalBytes = rowPitch * mipHeight * mipDepth;
			currentSrcOffset += totalBytes;
		}
	}
}
