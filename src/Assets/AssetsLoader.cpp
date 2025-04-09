#include "stdafx.h"
#include "AssetsLoader.h"
#include <DebugMacros.h>
#include <filesystem>
#include <ECS/Components/Transform.h>
#include <ECS/Components/HierarchyRelationship.h>
#include <ECS/Components/InstancedStaticMesh.h>

namespace Assets
{
    AssetsLoader::AssetsLoader()
        : m_gltfLoader(std::make_unique<tinygltf::TinyGLTF>())
    {
    }

    void AssetsLoader::LoadGLTFScene(const std::wstring& filePath, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
    {
        tinygltf::Model model;
        std::string err, warn;

        std::string utf8Path = std::filesystem::path(filePath).string();
        bool ret = m_gltfLoader->LoadASCIIFromFile(&model, &err, &warn, utf8Path);
        if (!warn.empty()) { std::cerr << "GLTF Warning: " << warn.c_str() << std::endl; }
        if (!err.empty()) { std::cerr << "GLTF Error: " << err.c_str() << std::endl; }
        ASSERT(ret, "Failed to load GLTF file");

        std::wstring basePath = std::filesystem::path(filePath).parent_path().wstring();

        LoadTexturesFromGLTF(model, basePath, commandBuffer);

        auto& entityManager = Core::EntitiesPool::GetInstance();
        auto& transformSystem = TransformSystem::GetInstance();
        auto& registry = entityManager.GetRegistry();
        std::vector<GLTFNodeEntity> entities;

        for (int nodeIndex : model.scenes[0].nodes)
        {
            ProcessNode(model, nodeIndex, Matrix::Identity, entt::null, entities, commandBuffer);
        }

        std::unordered_map<Core::Entity, std::vector<Core::Entity>> hierarchyMap;
        for (auto& nodeEntity : entities)
        {
            Core::Entity entity = nodeEntity.entity;

            // Transform
            auto& transform = registry.emplace<Components::Transform>(entity);
            transform.position = nodeEntity.transform.Translation();
            DirectX::SimpleMath::Quaternion quat;
            nodeEntity.transform.Decompose(transform.scale, quat, transform.position);
            transform.rotationAngles = quat.ToEuler();

            auto& hierarchy = registry.emplace<Components::HierarchyRelationship>(entity);
            hierarchyMap[entity] = {};

            registry.emplace<Components::TransformFlags>(entity);
            registry.emplace<Components::AccumulatedHierarchicalTransformMatrix>(entity);

            if (nodeEntity.meshIndex >= 0)
            {
                // Mesh
                auto meshId = AssetsManager<StaticMesh>::GetInstance().CreateAsset(LoadMeshFromGLTF(model, nodeEntity.meshIndex, commandBuffer));
                auto& meshComponent = registry.emplace<Components::InstancedStaticMesh>(entity, meshId);
                auto& mesh = AssetsManager<StaticMesh>::GetInstance().GetAsset(meshId);

                MeshPerInstanceData instanceData{
                    .transform = nodeEntity.transform,
                    .inverseTransposeTransform = nodeEntity.transform.Invert().Transpose()
                };
                mesh.CreatePerInstanceData(entity, instanceData);

                // Material (assuming one submesh)
                if (!mesh.m_submeshes.empty())
                {
                    mesh.CreateSubmeshPerInstanceData<Material>((Core::Entity)entity, 0u, Material{});
                }

                mesh.UpdateRHIBufferWithPerInstanceData(commandBuffer);
            }
            else if (nodeEntity.isLight)
            {
                auto& light = registry.emplace<Components::PointLight>(entity);
                light.color = nodeEntity.lightColor;
                light.intensity = nodeEntity.lightIntensity;

                auto& lightManager = LightSourcesManager::GetInstance();
                lightManager.AddPointLight(entity);
                lightManager.UpdatePointLightParams(entity, light);
            }

            transformSystem.MarkDirty(registry, entity);
        }

        // Set up hierarchy relationships
        for (const auto& nodeEntity : entities)
        {
            auto& hierarchy = registry.get<Components::HierarchyRelationship>(nodeEntity.entity);
            if (hierarchy.parent != entt::null)
            {
                hierarchyMap[hierarchy.parent].push_back(nodeEntity.entity);
            }
        }

        for (auto& [parent, children] : hierarchyMap)
        {
            if (!children.empty())
            {
                auto& hierarchy = registry.get<Components::HierarchyRelationship>(parent);
                hierarchy.children = static_cast<uint32_t>(children.size());
                hierarchy.first = children.front();
                for (size_t i = 0; i < children.size() - 1; ++i)
                {
                    auto& childHierarchy = registry.get<Components::HierarchyRelationship>(children[i]);
                    childHierarchy.next = children[i + 1];
                }
            }
        }
    }

    AssetsManager<std::shared_ptr<RHI::ITexture>>::AssetId AssetsLoader::LoadTexture(const std::string& filePath, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
    {
        auto it = m_textureCache.find(filePath);
        if (it != m_textureCache.end())
        {
            return it->second;
        }

        // Placeholder: Load image data (e.g., via stb_image)
        std::vector<unsigned char> imageData; // Load from file
        int width = 0, height = 0; // Fill these
        // imageData = loadImage(filePath, &width, &height);

        RHI::Texture2DDescription desc = {
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height),
            .format = RHI::TextureFormat::RGBA8_UNORM,
            .usage = RHI::TextureUsage::ShaderResource
        };
        auto texture = m_rhiContext->CreateTexture2D(desc);
        texture->UploadData(imageData.data(), width * height * 4, commandBuffer);

        auto& textureManager = AssetsManager<std::shared_ptr<RHI::ITexture2D>>::GetInstance();
        auto textureId = textureManager.CreateAsset(std::move(texture));
        m_textureCache[filePath] = textureId;

        return textureId;
    }

    void AssetsLoader::LoadTexturesFromGLTF(const tinygltf::Model& model, const std::wstring& basePath, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
    {
        for (const auto& texture : model.textures)
        {
            if (texture.source >= 0 && texture.source < model.images.size())
            {
                const auto& image = model.images[texture.source];
                std::string uri = image.uri;
                if (uri.empty() && !image.image.empty())
                {
                    RHI::Texture2DDescription desc = {
                        .width = static_cast<uint32_t>(image.width),
                        .height = static_cast<uint32_t>(image.height),
                        .format = RHI::TextureFormat::RGBA8_UNORM,
                        .usage = RHI::TextureUsage::ShaderResource
                    };
                    auto texture = m_rhiContext->CreateTexture2D(desc);
                    texture->UploadData(image.image.data(), image.width * image.height * 4, commandBuffer);

                    auto& textureManager = AssetsManager<std::shared_ptr<RHI::ITexture2D>>::GetInstance();
                    std::string textureName = image.name.empty() ? "texture_" + std::to_string(texture.source) : image.name;
                    m_textureCache[textureName] = textureManager.CreateAsset(std::move(texture));
                }
                else if (!uri.empty())
                {
                    std::wstring fullPath = basePath + L"/" + std::filesystem::path(uri).wstring();
                    LoadTexture(std::filesystem::path(fullPath).string(), commandBuffer);
                }
            }
        }
    }

    StaticMesh AssetsLoader::LoadMeshFromGLTF(const tinygltf::Model& model, int meshIndex, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
    {
        const auto& mesh = model.meshes[meshIndex];
        StaticMesh staticMesh;

        for (const auto& primitive : mesh.primitives)
        {
            std::vector<float> positions;
            std::vector<float> normals;
            std::vector<uint32_t> indices;

            // Positions
            const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
            const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
            const auto& posBuffer = model.buffers[posBufferView.buffer];
            positions.resize(posAccessor.count * 3);
            memcpy(positions.data(), &posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset], posAccessor.count * 3 * sizeof(float));

            // Normals (optional)
            if (primitive.attributes.count("NORMAL"))
            {
                const auto& normAccessor = model.accessors[primitive.attributes.at("NORMAL")];
                const auto& normBufferView = model.bufferViews[normAccessor.bufferView];
                const auto& normBuffer = model.buffers[normBufferView.buffer];
                normals.resize(normAccessor.count * 3);
                memcpy(normals.data(), &normBuffer.data[normBufferView.byteOffset + normAccessor.byteOffset], normAccessor.count * 3 * sizeof(float));
            }

            // Indices
            const auto& indexAccessor = model.accessors[primitive.indices];
            const auto& indexBufferView = model.bufferViews[indexAccessor.bufferView];
            const auto& indexBuffer = model.buffers[indexBufferView.buffer];
            indices.resize(indexAccessor.count);
            if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
            {
                std::vector<uint16_t> tempIndices(indexAccessor.count);
                memcpy(tempIndices.data(), &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset], indexAccessor.count * sizeof(uint16_t));
                for (size_t i = 0; i < indexAccessor.count; ++i) indices[i] = tempIndices[i];
            }
            else
            {
                memcpy(indices.data(), &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset], indexAccessor.count * sizeof(uint32_t));
            }

            // RHI buffers
            RHI::BufferDescription vertexDesc = {
                .size = static_cast<uint32_t>(positions.size() * sizeof(float)),
                .usage = RHI::BufferUsage::VertexBuffer
            };
            auto vertexBuffer = m_rhiContext->CreateBuffer(vertexDesc);
            vertexBuffer->UploadData(positions.data(), vertexDesc.size, commandBuffer);

            RHI::BufferDescription normalDesc = {
                .size = static_cast<uint32_t>(normals.size() * sizeof(float)),
                .usage = RHI::BufferUsage::VertexBuffer
            };
            auto normalBuffer = normals.empty() ? nullptr : m_rhiContext->CreateBuffer(normalDesc);
            if (normalBuffer)
                normalBuffer->UploadData(normals.data(), normalDesc.size, commandBuffer);

            RHI::BufferDescription indexDesc = {
                .size = static_cast<uint32_t>(indices.size() * sizeof(uint32_t)),
                .usage = RHI::BufferUsage::IndexBuffer
            };
            auto indexBuffer = m_rhiContext->CreateBuffer(indexDesc);
            indexBuffer->UploadData(indices.data(), indexDesc.size, commandBuffer);

            StaticSubmesh submesh(
                { vertexBuffer, 0, vertexDesc.size },
                { normalBuffer, 0, normalDesc.size },
                { indexBuffer, 0, indexDesc.size }
            );

            // Material
            if (primitive.material >= 0)
            {
                const auto& mat = model.materials[primitive.material];
                if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0)
                {
                    const auto& tex = model.textures[mat.pbrMetallicRoughness.baseColorTexture.index];
                    std::string texName = model.images[tex.source].name.empty() ? "texture_" + std::to_string(tex.source) : model.images[tex.source].name;
                    submesh.CreatePerInstanceData<Material>((Core::Entity)0, MeshPerInstanceDataHandle{ 0 }, Material{ m_textureCache[texName] });
                }
            }

            staticMesh.m_submeshes.push_back(std::move(submesh));
        }

        return staticMesh;
    }

    void AssetsLoader::ProcessNode(const tinygltf::Model& model, int nodeIndex, const Matrix& parentTransform, Core::Entity parentEntity, std::vector<GLTFNodeEntity>& entities, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
    {
        const auto& node = model.nodes[nodeIndex];
        auto& entityManager = Core::EntitiesPool::GetInstance();
        Core::Entity entity = entityManager.CreateEntity();

        // Compute transform
        Matrix localTransform = Matrix::Identity;
        if (node.matrix.size() == 16)
        {
            localTransform = Matrix(
                node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
                node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
                node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
                node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]
            );
        }
        else
        {
            Vector3 translation = node.translation.size() == 3 ? Vector3(node.translation[0], node.translation[1], node.translation[2]) : Vector3::Zero;
            Quaternion rotation = node.rotation.size() == 4 ? Quaternion(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]) : Quaternion::Identity;
            Vector3 scale = node.scale.size() == 3 ? Vector3(node.scale[0], node.scale[1], node.scale[2]) : Vector3::One;
            localTransform = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(translation);
        }

        Matrix worldTransform = localTransform * parentTransform;

        // Create node entity
        GLTFNodeEntity nodeEntity{
            .entity = entity,
            .transform = worldTransform,
            .meshIndex = node.mesh,
            .isLight = false // Placeholder; extend with GLTF light extension
        };

        // Detect lights (using name as a simple heuristic; extend with KHR_lights_punctual if needed)
        if (node.name.find("Light") != std::string::npos)
        {
            nodeEntity.isLight = true;
            nodeEntity.lightColor = Vector3(1.0f, 1.0f, 1.0f); // Default white; parse from extensions
            nodeEntity.lightIntensity = 1.0f;
        }

        entities.push_back(nodeEntity);

        // Process children
        for (int childIndex : node.children)
        {
            ProcessNode(model, childIndex, worldTransform, entity, entities, commandBuffer);
            auto& childHierarchy = entityManager.GetRegistry().get<Components::HierarchyRelationship>(entities.back().entity);
            childHierarchy.parent = entity;
        }
    }
}