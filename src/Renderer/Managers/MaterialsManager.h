#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include "../RHI/PipelineCommon.h"

class MaterialsManager
{
public:
    static MaterialsManager& GetInstance()
    {
        static MaterialsManager instance;
        return instance;
    }

    MaterialsManager(const MaterialsManager& arg) = delete;
    MaterialsManager& operator=(const MaterialsManager& arg) = delete;

    void Destroy();

    std::shared_ptr<RHI::IRenderPipeline> GetRenderPipeline(const std::string& name);
    std::shared_ptr<RHI::IComputePipeline> GetComputePipeline(const std::string& name);

    void SetRenderPipeline(const std::string& name, std::shared_ptr<RHI::IRenderPipeline> pipeline);
    void SetComputePipeline(const std::string& name, std::shared_ptr<RHI::IComputePipeline> pipeline);

    void RemoveRenderPipeline(const std::string& name);
    void RemoveComputePipeline(const std::string& name);

    static std::shared_ptr<RHI::IRenderPipeline> CreateForwardPBRPipeline();
    static std::shared_ptr<RHI::IRenderPipeline> CreateForwardMaskedPBRPipeline();
    static std::shared_ptr<RHI::IRenderPipeline> CreateOpaqueDepthOnlyPipeline();
    static std::shared_ptr<RHI::IComputePipeline> CreateMipGenerationPipeline();
    static std::shared_ptr<RHI::IRenderPipeline> CreatePostProcessPipeline();
private:
    MaterialsManager() {}

    std::unordered_map<std::string, std::shared_ptr<RHI::IRenderPipeline>> m_nameToRenderPipeline;
    std::unordered_map<std::string, std::shared_ptr<RHI::IComputePipeline>> m_nameToComputePipeline;
};