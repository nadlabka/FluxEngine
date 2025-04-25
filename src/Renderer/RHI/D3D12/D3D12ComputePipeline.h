#pragma once
#include "../PipelineCommon.h"

namespace RHI
{
    struct D3D12ComputePipeline : public IComputePipeline
    {
        D3D12ComputePipeline(RscPtr<ID3D12PipelineState> pipelineState, const ComputePipelineDescription& description);

        ComputePipelineDescription& GetPipelineDescription();

        RscPtr<ID3D12PipelineState> m_pipelineState;
        ComputePipelineDescription m_description; // change to d3d12 specific struct to avoid further conversions during usage
    };
}