#pragma once
#include <optional>
#include "Texture.h"

namespace RHI
{
    struct AttachmentDesc
    {
        TextureFormat format;

        uint32_t sampleCount = 1;

        LoadAccessOperation loadOp = LoadAccessOperation::DontCare;
        StoreAccessOperation storeOp = StoreAccessOperation::DontCare;

        LoadAccessOperation stencilLoadOp = LoadAccessOperation::DontCare;
        StoreAccessOperation stencilStoreOp = StoreAccessOperation::DontCare;

        std::array<float, 4> clearColor { 0, 0, 0, 1 };
        float clearDepth = 0.0f;
        uint32_t clearStencil = 0u;

        TextureLayout initialLayout = TextureLayout::Undefined;
        TextureLayout finalLayout = TextureLayout::Present;
    };

    struct RenderPassDesc 
    {
        std::vector<AttachmentDesc> colorAttachments;
        std::optional<AttachmentDesc> depthStencilAttachment;
    };

    struct SubResourceRTsDescription
    {
        std::shared_ptr<ITexture> texture;
        std::vector<uint32_t> mipsToInclude;
    };

	struct IRenderPass
	{
		virtual ~IRenderPass() {}

        virtual void SetAttachments(const std::vector<SubResourceRTsDescription>& colorRTs, std::shared_ptr<ITexture> depthStencilRT) = 0;
	};
}