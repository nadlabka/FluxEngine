#pragma once
#include <entt/entt.hpp>
#include "../Renderer/RHI/RHIContext.h"

class ConstantBufferManager
{
public:
    struct UploadAndPrivateBuffersPair
    {
        std::shared_ptr<RHI::IBuffer> uploadBuffer;
        std::shared_ptr<RHI::IBuffer> dataBuffer;
    };

    static ConstantBufferManager& GetInstance()
    {
        static ConstantBufferManager instance;
        return instance;
    }

    ConstantBufferManager(const ConstantBufferManager& arg) = delete;
    ConstantBufferManager& operator=(const ConstantBufferManager& arg) = delete;

    void Destroy();

    template <typename T>
    void RegisterBuffer(const std::string& name)
    {
        ASSERT(m_nameToEntity.find(name) != m_nameToEntity.end(), "Buffer has already been registered");

        auto& rhiContext = RHI::RHIContext::GetInstance();
        auto allocator = rhiContext.GetAllocator();

        RHI::BufferDescription uploadBufferDesc = {};
        uploadBufferDesc.access = RHI::BufferAccess::Upload;
        uploadBufferDesc.unstructuredSize = sizeof(T);
        uploadBufferDesc.flags = { .requiredCopyStateToInit = false };
        uploadBufferDesc.usage = RHI::BufferUsage::None;
        auto uploadBuffer = allocator->CreateBuffer(uploadBufferDesc);

        RHI::BufferDescription dataBufferDesc = {};
        dataBufferDesc.access = RHI::BufferAccess::DefaultPrivate;
        dataBufferDesc.unstructuredSize = sizeof(T);
        dataBufferDesc.flags = { .requiredCopyStateToInit = true };
        dataBufferDesc.usage = RHI::BufferUsage::UniformBuffer;
        auto dataBuffer = allocator->CreateBuffer(dataBufferDesc);

        m_nameToEntity[name] = { uploadBuffer, dataBuffer };
    }

    template <typename T>
    void UpdateBuffer(std::shared_ptr<RHI::ICommandQueue> commandQueue, std::shared_ptr<RHI::ICommandBuffer> commandBuffer, const std::string& name, const T& data)
    {
        auto& buffersPair = m_nameToEntity[name];

        buffersPair.uploadBuffer->UploadData((void*)&data, {.srcOffset = 0, .destOffset = 0, .width = sizeof(T)});

        commandBuffer->BeginRecording(commandQueue);
        commandBuffer->CopyDataBetweenBuffers(buffersPair.uploadBuffer, buffersPair.dataBuffer, { .srcOffset = 0, .destOffset = 0, .width = sizeof(T) });
        commandBuffer->EndRecording();
        commandBuffer->SubmitToQueue(commandQueue);

        commandBuffer->ForceWaitUntilFinished(commandQueue);
    }

    std::shared_ptr<RHI::IBuffer> GetDataBufferByName(const std::string& name)
    {
        return m_nameToEntity[name].dataBuffer;
    }

private:
    ConstantBufferManager() {}

    std::unordered_map<std::string, UploadAndPrivateBuffersPair> m_nameToEntity;
};