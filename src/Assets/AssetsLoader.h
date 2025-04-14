#pragma once
#include <string>
#include <memory>
#include <vector>
#include "AssetsManager.h"
#include "../Renderer/RHI/RHIContext.h"
#include "Mesh.h"
#include "Material.h"
#include <ECS/Entity.h>
#include <ECS/Managers/EntitiesPool.h>
#include "../Core/ECS/Managers/TransformSystem.h"
#include "../Renderer/Managers/LightSourcesManager.h"
#include <tinygltf/tiny_gltf.h>


namespace Assets
{
    class AssetsLoader
    {
    public:
        static AssetsLoader& GetInstance()
        {
            static AssetsLoader instance;
            return instance;
        }

        AssetsLoader(const AssetsLoader&) = delete;
        AssetsLoader& operator=(const AssetsLoader&) = delete;

        void Destroy();

        void LoadGLTFScene(const std::wstring& filePath, std::shared_ptr<RHI::ICommandQueue> commandQueue, std::shared_ptr<RHI::ICommandBuffer> commandBuffer);

    private:
        AssetsLoader();

        struct GLTFNodeEntity
        {
            entt::entity entity; 
            DirectX::SimpleMath::Matrix transform;
            int meshIndex; // -1 if no mesh
            bool isLight;
            DirectX::SimpleMath::Vector3 lightColor;
            float lightIntensity;
        };

        void LoadTexturesFromGLTF(const tinygltf::Model& model, const std::wstring& basePath, std::shared_ptr<RHI::ICommandQueue> commandQueue, std::shared_ptr<RHI::ICommandBuffer> commandBuffer);
        StaticMesh LoadMeshFromGLTF(const tinygltf::Model& model, int meshIndex, std::shared_ptr<RHI::ICommandQueue> commandQueue, std::shared_ptr<RHI::ICommandBuffer> commandBuffer);
        void ProcessNode(const tinygltf::Model& model, int nodeIndex, const DirectX::SimpleMath::Matrix& parentTransform, entt::entity parentEntity, std::vector<GLTFNodeEntity>& entities, std::shared_ptr<RHI::ICommandQueue> commandQueue, std::shared_ptr<RHI::ICommandBuffer> commandBuffer);

        tinygltf::TinyGLTF m_gltfLoader;
        std::shared_ptr<RHI::IBuffer> m_commonUploadBuffer;
        uint32_t m_commonUploadBufferSize;
    };
}