#include "stdafx.h"
#include "D3D12Sampler.h"

D3D12_SAMPLER_DESC RHI::ConvertSamplerDescriptionToD3D12(const SamplerDescription& desc)
{
    D3D12_SAMPLER_DESC d3dDesc = {};

    switch (desc.Filter)
    {
    case FilterMode::Nearest:
        d3dDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        break;
    case FilterMode::Linear:
        d3dDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        break;
    case FilterMode::Anisotropic:
        d3dDesc.Filter = D3D12_FILTER_ANISOTROPIC;
        break;
    }

    auto mapAddressMode = [](AddressMode mode) -> D3D12_TEXTURE_ADDRESS_MODE
        {
            switch (mode)
            {
            case AddressMode::Wrap: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case AddressMode::Mirror: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            case AddressMode::Clamp: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case AddressMode::Border: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            case AddressMode::MirrorOnce: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
            default: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            }
        };
    d3dDesc.AddressU = mapAddressMode(desc.AddressU);
    d3dDesc.AddressV = mapAddressMode(desc.AddressV);
    d3dDesc.AddressW = mapAddressMode(desc.AddressW);

    switch (desc.ComparisonFunc)
    {
    case SamplerComparisonFunc::Never: d3dDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; break;
    case SamplerComparisonFunc::Less: d3dDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS; break;
    case SamplerComparisonFunc::Equal: d3dDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_EQUAL; break;
    case SamplerComparisonFunc::LessEqual: d3dDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; break;
    case SamplerComparisonFunc::Greater: d3dDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER; break;
    case SamplerComparisonFunc::NotEqual: d3dDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL; break;
    case SamplerComparisonFunc::GreaterEqual: d3dDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL; break;
    case SamplerComparisonFunc::Always: d3dDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS; break;
    }

    d3dDesc.MipLODBias = desc.MipLODBias;
    d3dDesc.MaxAnisotropy = desc.MaxAnisotropy;
    d3dDesc.MinLOD = desc.MinLOD;
    d3dDesc.MaxLOD = desc.MaxLOD;
    std::memcpy(d3dDesc.BorderColor, &desc.BorderColor, sizeof(float) * 4);

    return d3dDesc;
}
