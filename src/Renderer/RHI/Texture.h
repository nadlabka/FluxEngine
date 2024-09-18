#pragma once
#include <stdafx.h>

namespace RHI
{
	enum TextureUsage : uint8_t
	{
		eTextureUsage_TransferSource = 0,
		eTextureUsage_TransferDestination = 1 << 0,
		eTextureUsage_Sampled = 1 << 1,
		eTextureUsage_Storage = 1 << 2,
		eTextureUsage_ColorAttachment = 1 << 3,
		eTextureUsage_DepthStencilAttachment = 1 << 4,
		eTextureUsage_TransientAttachment = 1 << 5,
		eTextureUsage_InputAttachment = 1 << 6
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
		eTextureAspect_HasColor = 0,
		eTextureAspect_HasDepth = 1 << 0,
		eTextureAspect_HasStencil = 1 << 1,
		eTextureAspect_HasMetadata = 1 << 2,
	};

	enum class TextureFormat : uint8_t
	{
		Undefined,
		eTextureFormat_BGRA8_UNORM,
		eTextureFormat_RGBA8_UINT,
		eTextureFormat_RGBA8_UNORM,
		eTextureFormat_RGBA16_UNORM,
		eTextureFormat_RGBA16_SNORM,
		eTextureFormat_RGBA16_FLOAT,
		eTextureFormat_RGBA32_FLOAT,

		eTextureFormat_R32_UINT,
		eTextureFormat_R32_FLOAT,

		eTextureFormat_D32_FLOAT,
		eTextureFormat_D24_UNORM_S8_UINT
	};

	enum class TextureType : uint8_t 
	{
		eTextureType_Texture2D,
		eTextureType_TextureCubemap
	};

	enum class PostInitOp : uint8_t
	{
		ePostInitOp_Clear,
		ePostInitOp_Discard
	};

	struct TexturePropertiesDesc
	{
		TextureUsage usage;
		TextureAspect aspect;
		TextureFormat format;
		TextureType type;
		TextureLayout layout;
		uint32_t width = 0, height = 0, depth = 1;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		Vector4 clearValue{ 0, 0, 0, 0 };
	};

	struct ITexture
	{
		TexturePropertiesDesc textureProperties;

		virtual ~ITexture() {}
	};
}