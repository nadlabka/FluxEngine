#include "stdafx.h"
#include "D3D12Texture.h"

RHI::D3D12Texture::D3D12Texture(const TextureAccessInfo& accessInfo, const ResourceDescriptorsIndices& descriptorsIndices, RscPtr<ID3D12Resource> texture)
	: m_accessInfo(accessInfo), m_descriptorsIndices(descriptorsIndices), m_texture(texture)
{
}

void RHI::D3D12Texture::SetClearColor(const std::array<float, 4>& clearValue)
{
    m_clearValue.Color[0] = clearValue[0];
    m_clearValue.Color[1] = clearValue[1];
    m_clearValue.Color[2] = clearValue[2];
    m_clearValue.Color[3] = clearValue[3];
}

void RHI::D3D12Texture::SetClearValue(float clearValueDepth, uint8_t clearValueStencil)
{
    m_clearValue.DepthStencil.Depth = clearValueDepth;
    m_clearValue.DepthStencil.Stencil = clearValueStencil;
}

DXGI_FORMAT RHI::ConvertFormatToD3D12(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::eTextureFormat_BGRA8_UNORM:  return DXGI_FORMAT_B8G8R8A8_UNORM;
    case TextureFormat::eTextureFormat_RGBA8_UNORM:  return DXGI_FORMAT_R8G8B8A8_UNORM;
    case TextureFormat::eTextureFormat_RGBA8_UINT:  return DXGI_FORMAT_R8G8B8A8_UINT;

    case TextureFormat::eTextureFormat_RGBA16_UNORM:  return DXGI_FORMAT_R16G16B16A16_UNORM;
    case TextureFormat::eTextureFormat_RGBA16_SNORM:  return DXGI_FORMAT_R16G16B16A16_SNORM;

    case TextureFormat::eTextureFormat_RGBA16_FLOAT:  return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case TextureFormat::eTextureFormat_RGBA32_FLOAT:  return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case TextureFormat::eTextureFormat_R32_UINT:  return DXGI_FORMAT_R32_UINT;
    case TextureFormat::eTextureFormat_R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;

    case TextureFormat::eTextureFormat_D32_FLOAT:  return DXGI_FORMAT_D32_FLOAT;
    case TextureFormat::eTextureFormat_D24_UNORM_S8_UINT:  return DXGI_FORMAT_D24_UNORM_S8_UINT;

    case TextureFormat::Undefined:  return DXGI_FORMAT_UNKNOWN;

    default:
        throw std::runtime_error("Unsupported texture format");
    }
}

DXGI_FORMAT RHI::GetTyplessVersionOfFormat(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_B8G8R8A8_UNORM: 
        return DXGI_FORMAT_B8G8R8A8_TYPELESS;

    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UINT: 
        return DXGI_FORMAT_R8G8B8A8_TYPELESS;

    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_FLOAT: 
        return DXGI_FORMAT_R16G16B16A16_TYPELESS;

    case DXGI_FORMAT_R32G32B32A32_FLOAT: 
        return DXGI_FORMAT_R32G32B32A32_TYPELESS;

    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_FLOAT: 
        return DXGI_FORMAT_R32_TYPELESS;

    case DXGI_FORMAT_D32_FLOAT: 
        return DXGI_FORMAT_R32_TYPELESS;

    case DXGI_FORMAT_D24_UNORM_S8_UINT: 
        return DXGI_FORMAT_R24G8_TYPELESS;

    default:
        throw std::runtime_error("Unsupported texture format");
    }
}

D3D12_RESOURCE_DIMENSION RHI::ConvertTextureTypeToD3D12(TextureType type)
{
    switch (type)
    {
    case TextureType::eTextureType_Texture2D:
        return D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    case TextureType::eTextureType_TextureCubemap:
        return D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    default:
        return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}
