#include "stdafx.h"
#include "D3D12ComputePipeline.h"

RHI::D3D12ComputePipeline::D3D12ComputePipeline(RscPtr<ID3D12PipelineState> pipelineState, const ComputePipelineDescription& description)
	: m_pipelineState(pipelineState), m_description(description)
{
}

RHI::ComputePipelineDescription& RHI::D3D12ComputePipeline::GetPipelineDescription()
{
	return m_description;
}
