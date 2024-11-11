#pragma once
#include <optional>
#include "Texture.h"

namespace RHI
{
    struct AttachmentDesc
    {
        TextureFormat format = TextureFormat::RGBA8_UNORM;

        uint32_t sampleCount = 1;

        LoadAccessOperation loadOp = LoadAccessOperation::Clear;
        StoreAccessOperation storeOp = StoreAccessOperation::Store;

        LoadAccessOperation stencilLoadOp = LoadAccessOperation::Clear;
        StoreAccessOperation stencilStoreOp = StoreAccessOperation::Store;

        std::array<float, 4> clearColor{ 0, 0, 0, 1 };
        float clearDepth = 0.0f;
        uint32_t clearStencil = 0u;

        TextureLayout initialLayout = TextureLayout::ColorAttachmentOptimal;
        TextureLayout finalLayout = TextureLayout::Present;
    };

    struct RenderPassDesc 
    {
        std::vector<AttachmentDesc> colorAttachments;
        std::optional<AttachmentDesc> depthStencilAttachment;
    };

    struct SubResourceRTsDescription
    {
        struct TextureArraySliceToInclude
        {
            uint32_t sliceIndex = 0;
            std::vector<uint32_t> mipsToInclude = { 0 };
        };

        std::shared_ptr<ITexture> texture;
        std::vector<TextureArraySliceToInclude> slicesToInclude;
    };

	struct IRenderPass
	{
		virtual ~IRenderPass() {}

        virtual void SetAttachments(const std::vector<SubResourceRTsDescription>& colorRTs, std::shared_ptr<ITexture> depthStencilRT) = 0;
	};
}