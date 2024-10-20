#pragma once
#include "../RenderPipeline.h"

namespace RHI
{
    DXGI_FORMAT ConvertVertexAttributeFormatToDXGI(VertexAttributeFormat format);
    D3D12_INPUT_CLASSIFICATION ConvertBindingInputRateToD3D12(BindingInputRate rate);
    D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertPrimitiveTopologyToD3D12(PrimitiveTopology topology);
    D3D12_FILL_MODE ConvertPolygonFillModeToD3D12(PolygonFillMode mode);
    D3D12_CULL_MODE ConvertCullModeToD3D12(CullMode mode);
    D3D12_BLEND ConvertBlendFactorToD3D12(BlendFactor factor);
    D3D12_BLEND_OP ConvertBlendOperationToD3D12(BlendOperation operation);
    D3D12_LOGIC_OP ConvertLogicalOperationToD3D12(LogicalOperation op);

	struct D3D12RenderPipeline : public IRenderPipeline
	{
		virtual ~D3D12RenderPipeline() {}
	};
}