#include "stdafx.h"
#include "BuffersPair.h"

void PrivateUploadBuffersPair::Resize(uint32_t newElementCount, uint32_t elementStride, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
{
    auto& rhiContext = RHI::RHIContext::GetInstance();
    auto allocator = rhiContext.GetAllocator();

    if (!dataBuffer || !uploadBuffer)
    {
        RHI::BufferDescription desc{};
        desc.elementsNum = newElementCount;
        desc.elementStride = elementStride;
        desc.usage = RHI::BufferUsage::DataReadOnlyBuffer;

        desc.access = RHI::BufferAccess::Upload;
        desc.usage = RHI::BufferUsage::None;
        desc.flags = { .requiredCopyStateToInit = false };
        uploadBuffer = allocator->CreateBuffer(desc);

        desc.access = RHI::BufferAccess::DefaultPrivate;
        desc.usage = RHI::BufferUsage::DataReadOnlyBuffer;
        desc.flags = { .requiredCopyStateToInit = true };
        dataBuffer = allocator->CreateBuffer(desc);
        return;
    }

    RHI::BufferDescription desc{};
    desc.elementsNum = newElementCount;
    desc.elementStride = elementStride;
    desc.flags = { .requiredCopyStateToInit = true };
    desc.usage = RHI::BufferUsage::DataReadOnlyBuffer;

    desc.access = RHI::BufferAccess::Upload;
    desc.usage = RHI::BufferUsage::None;
    desc.flags = { .requiredCopyStateToInit = false };
    auto newUploadBuffer = allocator->CreateBuffer(desc);

    desc.access = RHI::BufferAccess::DefaultPrivate;
    desc.usage = RHI::BufferUsage::DataReadOnlyBuffer;
    desc.flags = { .requiredCopyStateToInit = true };
    auto newDataBuffer = allocator->CreateBuffer(desc);


    uint32_t oldElementCount = dataBuffer->GetStructuredElementsNum();

    RHI::BufferRegionCopyDescription copyDesc
    {
        .srcOffset = 0,
        .destOffset = 0,
        .width = oldElementCount * elementStride
    };

    commandBuffer->CopyDataBetweenBuffers(dataBuffer, newDataBuffer, copyDesc);
    commandBuffer->CopyDataBetweenBuffers(uploadBuffer, newUploadBuffer, copyDesc);

    dataBuffer = std::move(newDataBuffer);
    uploadBuffer = std::move(newUploadBuffer);
}
