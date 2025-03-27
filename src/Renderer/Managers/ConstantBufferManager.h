#pragma once
#include <entt/entt.hpp>
#include "../Renderer/RHI/RHIContext.h"
#include "../DataTypes/BuffersPair.h"

class ConstantBufferManager
{
public:
    static ConstantBufferManager& GetInstance()
    {
        static ConstantBufferManager instance;
        return instance;
    }

    ConstantBufferManager(const ConstantBufferManager& arg) = delete;
    ConstantBufferManager& operator=(const ConstantBufferManager& arg) = delete;

    void Destroy();

    template <typename T>
    void RegisterBuffer(const std::string& name, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
    {
        ASSERT(m_nameToRHIBuffers.find(name) == m_nameToRHIBuffers.end(), "Buffer already registered");

        m_nameToRHIBuffers[name].Resize(1, sizeof(T), commandBuffer, RHI::BufferUsage::UniformBuffer);
        m_cpuBuffers[name] = std::vector<uint8_t>(sizeof(T), 0);
    }

    template <typename T>
    T& GetCpuBuffer(const std::string& name)
    {
        auto it = m_cpuBuffers.find(name);
        ASSERT(it != m_cpuBuffers.end(), "Buffer not registered");
        auto& cpuBuffer = it->second;
        ASSERT(cpuBuffer.size() >= sizeof(T), "Buffer size mismatch");
        return *reinterpret_cast<T*>(cpuBuffer.data());
    }

    template <typename T>
    void SetCpuBuffer(const std::string& name, const T& data)
    {
        auto it = m_cpuBuffers.find(name);
        ASSERT(it != m_cpuBuffers.end(), "Buffer not registered");
        auto& cpuBuffer = it->second;
        ASSERT(cpuBuffer.size() >= sizeof(T), "Buffer size mismatch");
        std::memcpy(cpuBuffer.data(), &data, sizeof(T));
    }

    template <typename T>
    void UpdateBuffer(std::shared_ptr<RHI::ICommandQueue> commandQueue,
        std::shared_ptr<RHI::ICommandBuffer> commandBuffer,
        const std::string& name)
    {
        auto it = m_nameToRHIBuffers.find(name);
        ASSERT(it != m_nameToRHIBuffers.end(), "Buffer not registered");

        auto& buffersPair = it->second;
        auto& cpuBuffer = m_cpuBuffers[name];
        ASSERT(cpuBuffer.size() > 0, "Buffer not registered");

        buffersPair.uploadBuffer->UploadData(cpuBuffer.data(), { .srcOffset = 0, .destOffset = 0, .width = (uint32_t)cpuBuffer.size() });

        commandBuffer->BeginRecording(commandQueue);
        commandBuffer->CopyDataBetweenBuffers(buffersPair.uploadBuffer, buffersPair.dataBuffer, { .srcOffset = 0, .destOffset = 0, .width = (uint32_t)cpuBuffer.size() });
        commandBuffer->EndRecording();
        commandBuffer->SubmitToQueue(commandQueue);
    }

    std::shared_ptr<RHI::IBuffer> GetDataBufferByName(const std::string& name)
    {
        auto it = m_nameToRHIBuffers.find(name);
        ASSERT(it != m_nameToRHIBuffers.end(), "Buffer not registered");
        return it->second.dataBuffer;
    }

private:
    ConstantBufferManager() {}

    std::unordered_map<std::string, PrivateUploadBuffersPair> m_nameToRHIBuffers;
    std::unordered_map<std::string, std::vector<uint8_t>> m_cpuBuffers;
};