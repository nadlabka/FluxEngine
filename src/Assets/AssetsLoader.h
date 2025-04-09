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

namespace tinygltf 
{
    class Model;
    class TinyGLTF;
}

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

        void LoadGLTFScene(const std::wstring& filePath, std::shared_ptr<RHI::ICommandBuffer> commandBuffer);

        AssetsManager<std::shared_ptr<RHI::ITexture>>::AssetId LoadTexture(const std::string& filePath, std::shared_ptr<RHI::ICommandBuffer> commandBuffer);

    private:
        AssetsLoader();

        struct GLTFMeshData
        {
            AssetsManager<StaticMesh>::AssetId meshId;
            Matrix transform;
        };

        struct GLTFNodeEntity
        {
            Core::Entity entity;
            Matrix transform;
            int meshIndex; // -1 if no mesh
            bool isLight;
            Vector3 lightColor;
            float lightIntensity;
        };

        void LoadTexturesFromGLTF(const tinygltf::Model& model, const std::wstring& basePath, std::shared_ptr<RHI::ICommandBuffer> commandBuffer);
        StaticMesh LoadMeshFromGLTF(const tinygltf::Model& model, int meshIndex, std::shared_ptr<RHI::ICommandBuffer> commandBuffer);
        void ProcessNode(const tinygltf::Model& model, int nodeIndex, const Matrix& parentTransform, Core::Entity parentEntity, std::vector<GLTFNodeEntity>& entities, std::shared_ptr<RHI::ICommandBuffer> commandBuffer);

        std::unique_ptr<tinygltf::TinyGLTF> m_gltfLoader;
        std::unordered_map<std::string, AssetsManager<std::shared_ptr<RHI::ITexture>>::AssetId> m_textureCache;
    };
}