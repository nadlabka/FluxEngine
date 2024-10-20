#include "stdafx.h"
#include "D3D12RenderPipeline.h"

DXGI_FORMAT RHI::ConvertVertexAttributeFormatToDXGI(VertexAttributeFormat format)
{
    switch (format)
    {
    case VertexAttributeFormat::Undefined:                return DXGI_FORMAT_UNKNOWN;
    case VertexAttributeFormat::R32_Uint:                 return DXGI_FORMAT_R32_UINT;
    case VertexAttributeFormat::R32G32_SignedFloat:       return DXGI_FORMAT_R32G32_FLOAT;
    case VertexAttributeFormat::R32G32B32_SignedFloat:    return DXGI_FORMAT_R32G32B32_FLOAT;
    case VertexAttributeFormat::R32G32B32A32_SignedFloat: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    default:                                              return DXGI_FORMAT_UNKNOWN;
    }
}

D3D12_INPUT_CLASSIFICATION RHI::ConvertBindingInputRateToD3D12(BindingInputRate rate)
{
    switch (rate)
    {
    case BindingInputRate::PerVertex:
        return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    case BindingInputRate::PerInstance:
        return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    default:
        return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    }
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE RHI::ConvertPrimitiveTopologyToD3D12(PrimitiveTopology topology)
{
    switch (topology)
    {
    case PrimitiveTopology::PointList:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case PrimitiveTopology::LineList:
    case PrimitiveTopology::LineStrip:
    case PrimitiveTopology::LineListAdjacency:
    case PrimitiveTopology::LineStripAdjacency:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case PrimitiveTopology::TriangleList:
    case PrimitiveTopology::TriangleStrip:
    case PrimitiveTopology::TriangleFan:
    case PrimitiveTopology::TriangleListAdjacency:
    case PrimitiveTopology::TriangleStripAdjacency:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case PrimitiveTopology::PatchList:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    default:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    }
}

D3D12_FILL_MODE RHI::ConvertPolygonFillModeToD3D12(PolygonFillMode mode)
{
    switch (mode)
    {
    case PolygonFillMode::Fill:  return D3D12_FILL_MODE_SOLID;
    case PolygonFillMode::Line:  return D3D12_FILL_MODE_WIREFRAME;
    case PolygonFillMode::Point: return D3D12_FILL_MODE_SOLID; // D3D12 does not support point fill mode directly
    default:                     return D3D12_FILL_MODE_SOLID;
    }
}

D3D12_CULL_MODE RHI::ConvertCullModeToD3D12(CullMode mode)
{
    switch (mode)
    {
    case CullMode::None:  return D3D12_CULL_MODE_NONE;
    case CullMode::Front: return D3D12_CULL_MODE_FRONT;
    case CullMode::Back:  return D3D12_CULL_MODE_BACK;
    default:              return D3D12_CULL_MODE_NONE;
    }
}

D3D12_BLEND RHI::ConvertBlendFactorToD3D12(BlendFactor factor)
{
    switch (factor)
    {
    case BlendFactor::Zero:                  return D3D12_BLEND_ZERO;
    case BlendFactor::One:                   return D3D12_BLEND_ONE;
    case BlendFactor::SourceColor:           return D3D12_BLEND_SRC_COLOR;
    case BlendFactor::OneMinusSourceColor:   return D3D12_BLEND_INV_SRC_COLOR;
    case BlendFactor::DestColor:             return D3D12_BLEND_DEST_COLOR;
    case BlendFactor::OneMinusDestColor:     return D3D12_BLEND_INV_DEST_COLOR;
    case BlendFactor::SourceAlpha:           return D3D12_BLEND_SRC_ALPHA;
    case BlendFactor::OneMinusSourceAlpha:   return D3D12_BLEND_INV_SRC_ALPHA;
    case BlendFactor::DestAlpha:             return D3D12_BLEND_DEST_ALPHA;
    case BlendFactor::OneMinusDestAlpha:     return D3D12_BLEND_INV_DEST_ALPHA;
    case BlendFactor::ConstantColor:         return D3D12_BLEND_BLEND_FACTOR;
    case BlendFactor::OneMinusConstantColor: return D3D12_BLEND_INV_BLEND_FACTOR;
    case BlendFactor::ConstantAlpha:         return D3D12_BLEND_BLEND_FACTOR;
    case BlendFactor::OneMinusConstantAlpha: return D3D12_BLEND_INV_BLEND_FACTOR;
    case BlendFactor::SourceAlphaSaturate:   return D3D12_BLEND_SRC_ALPHA_SAT;
    case BlendFactor::Source1Color:          return D3D12_BLEND_SRC1_COLOR;
    case BlendFactor::OneMinusSource1Color:  return D3D12_BLEND_INV_SRC1_COLOR;
    case BlendFactor::Source1Alpha:          return D3D12_BLEND_SRC1_ALPHA;
    case BlendFactor::OneMinusSource1Alpha:  return D3D12_BLEND_INV_SRC1_ALPHA;
    default:                                 return D3D12_BLEND_ZERO;
    }
}

D3D12_BLEND_OP RHI::ConvertBlendOperationToD3D12(BlendOperation operation)
{
    switch (operation)
    {
    case BlendOperation::Add:            return D3D12_BLEND_OP_ADD;
    case BlendOperation::Subtract:       return D3D12_BLEND_OP_SUBTRACT;
    case BlendOperation::ReverseSubtract:return D3D12_BLEND_OP_REV_SUBTRACT;
    case BlendOperation::Min:            return D3D12_BLEND_OP_MIN;
    case BlendOperation::Max:            return D3D12_BLEND_OP_MAX;
    default:                             return D3D12_BLEND_OP_ADD;
    }
}

D3D12_LOGIC_OP RHI::ConvertLogicalOperationToD3D12(LogicalOperation op)
{
    switch (op)
    {
    case LogicalOperation::Clear:         return D3D12_LOGIC_OP_CLEAR;
    case LogicalOperation::AND:           return D3D12_LOGIC_OP_AND;
    case LogicalOperation::ANDReverse:    return D3D12_LOGIC_OP_AND_REVERSE;
    case LogicalOperation::Copy:          return D3D12_LOGIC_OP_COPY;
    case LogicalOperation::ANDInverted:   return D3D12_LOGIC_OP_AND_INVERTED;
    case LogicalOperation::NoOp:          return D3D12_LOGIC_OP_NOOP;
    case LogicalOperation::XOR:           return D3D12_LOGIC_OP_XOR;
    case LogicalOperation::OR:            return D3D12_LOGIC_OP_OR;
    case LogicalOperation::NOR:           return D3D12_LOGIC_OP_NOR;
    case LogicalOperation::Equivalent:    return D3D12_LOGIC_OP_EQUIV;
    case LogicalOperation::Invert:        return D3D12_LOGIC_OP_INVERT;
    case LogicalOperation::ORReverse:     return D3D12_LOGIC_OP_OR_REVERSE;
    case LogicalOperation::CopyInverted:  return D3D12_LOGIC_OP_COPY_INVERTED;
    case LogicalOperation::ORInverted:    return D3D12_LOGIC_OP_OR_INVERTED;
    case LogicalOperation::NAND:          return D3D12_LOGIC_OP_NAND;
    case LogicalOperation::Set:           return D3D12_LOGIC_OP_SET;
    default:                              return D3D12_LOGIC_OP_NOOP;
    }
}

