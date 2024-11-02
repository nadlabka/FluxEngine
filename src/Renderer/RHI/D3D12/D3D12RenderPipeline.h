#pragma once
#include "../PipelineCommon.h"

namespace RHI
{
    DXGI_FORMAT ConvertVertexAttributeFormatToDXGI(VertexAttributeFormat format);
    D3D12_INPUT_CLASSIFICATION ConvertBindingInputRateToD3D12(BindingInputRate rate);
    D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertPrimitiveTopologyToD3D12TopologyType(PrimitiveTopology topology);
    D3D12_PRIMITIVE_TOPOLOGY ConvertPrimitiveTopologyToD3D12(PrimitiveTopology topology);
    D3D12_FILL_MODE ConvertPolygonFillModeToD3D12(PolygonFillMode mode);
    D3D12_CULL_MODE ConvertCullModeToD3D12(CullMode mode);
    D3D12_BLEND ConvertBlendFactorToD3D12(BlendFactor factor);
    D3D12_BLEND_OP ConvertBlendOperationToD3D12(BlendOperation operation);
    D3D12_LOGIC_OP ConvertLogicalOperationToD3D12(LogicalOperation op);
    D3D12_COMPARISON_FUNC ConvertDepthStencilCompareOperationToD3D12(DepthStencilCompareOperation operation);
    D3D12_STENCIL_OP ConvertStencilOperationToD3D12(StencilOperation operation);

	struct D3D12RenderPipeline : public IRenderPipeline
	{
        D3D12RenderPipeline(RscPtr<ID3D12PipelineState> pipelineState, const RenderPipelineDescription& description);

        RscPtr<ID3D12PipelineState> m_pipelineState;
        RenderPipelineDescription m_description; // change to d3d12 specific struct to avoid further conversions during usage
	};
}