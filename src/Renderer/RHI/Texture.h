#pragma once
#include <stdafx.h>

namespace RHI
{
	enum TextureUsage : uint8_t
	{
		eTextureUsage_TransferSource = 1 << 0,
		eTextureUsage_TransferDestination = 1 << 1,
		eTextureUsage_Sampled = 1 << 2,
		eTextureUsage_Storage = 1 << 3,
		eTextureUsage_ColorAttachment = 1 << 4,
		eTextureUsage_DepthStencilAttachment = 1 << 5,
		eTextureUsage_TransientAttachment = 1 << 6,
		eTextureUsage_InputAttachment = 1 << 7
	};

	enum class TextureLayout : uint8_t 
	{
		Undefined,
		General,
		ColorAttachmentOptimal,
		DepthStencilAttachmentOptimal,
		DepthStencilReadOnlyOptimal,
		ShaderReadOnlyOptimal,
		TransferSourceOptimal,
		TransferDestinationOptimal,
		Reinitialized,
		DepthReadOnlyStencilAttachmentOptimal,
		DepthAttachmentStencilReadOnlyOptimal,
		DepthAttachmentOptimal,
		DepthReadOnlyOptimal,
		StencilAttachmentOptimal,
		StencilReadOnlyOptimal,
		ReadOnlyOptimal,
		AttachmentOptimal,
		Present
	};

	enum TextureAspect : uint8_t
	{
		eTextureAspect_HasColor = 1 << 0,
		eTextureAspect_HasDepth = 1 << 1,
		eTextureAspect_HasStencil = 1 << 2,
		eTextureAspect_HasMetadata = 1 << 3,
	};

	enum class TextureFormat : uint8_t
	{
		Undefined,
		BGRA8_UNORM,
		RGBA8_UINT,
		RGBA8_UNORM,
		RGBA16_UNORM,
		RGBA16_SNORM,
		RGBA16_FLOAT,
		RGBA32_FLOAT,

		R32_UINT,
		R32_FLOAT,

		D32_FLOAT,
		D24_UNORM_S8_UINT
	};

	enum class TextureType : uint8_t 
	{
		Texture1D,
		Texture1DArray,
		Texture2D,
		Texture2DArray,
		Texture3D,
		TextureCubemap,
		TextureCubemapArray
	};

	enum class LoadAccessOperation : uint8_t 
	{
		Load,
		Clear,
		DontCare,
		NotAccessed
	};

	enum class StoreAccessOperation : uint8_t 
	{
		Store,
		DontCare,
		None,
	};

	struct TextureDescription
	{
		TextureUsage usage;
		TextureAspect aspect;
		TextureFormat format;
		TextureType type;
		TextureLayout layout = TextureLayout::Undefined;
		uint32_t width = 0, height = 0, depth = 1;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		std::array<float, 4> clearColor {0.0f, 0.0f, 0.0f, 0.0f};
		float clearDepthValue = 0.0f;
		uint8_t clearStencilValue = 0u;
	};

	struct ITexture
	{
		virtual ~ITexture() {}
	};
}