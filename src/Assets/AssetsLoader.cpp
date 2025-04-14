#include <stdafx.h>
#include "AssetsLoader.h"
#include <DebugMacros.h>
#include <filesystem>
#include <ECS/Components/Transform.h>
#include <ECS/Components/HierarchyRelationship.h>
#include <ECS/Components/InstancedStaticMesh.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <tinygltf/tiny_gltf.h>
#include <ECS/Components/MaterialParameters.h>

namespace Assets
{
    AssetsLoader::AssetsLoader()
        : m_gltfLoader()
        , m_commonUploadBufferSize(4 * 1024 * 1024) // 4MB default
    {
        auto& rhiContext = RHI::RHIContext::GetInstance();
        auto allocator = rhiContext.GetAllocator();

        RHI::BufferDescription uploadBufferDesc = {
            .elementsNum = 1,
            .elementStride = m_commonUploadBufferSize,
            .unstructuredSize = 0,
            .access = RHI::BufferAccess::Upload,
            .usage = RHI::BufferUsage::None,
            .flags = {.requiredCopyStateToInit = false }
        };
        m_commonUploadBuffer = allocator->CreateBuffer(uploadBufferDesc);
    }

    void AssetsLoader::Destroy()
    {
        m_commonUploadBuffer.reset();
    }

    void AssetsLoader::LoadGLTFScene(const std::wstring& filePath, std::shared_ptr<RHI::ICommandQueue> commandQueue, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
    {
        tinygltf::Model model;
        std::string err, warn;

        std::string utf8Path = std::filesystem::path(filePath).string();
        bool ret = m_gltfLoader.LoadASCIIFromFile(&model, &err, &warn, utf8Path);
        if (!warn.empty()) { std::cerr << "GLTF Warning: " << warn.c_str() << std::endl; }
        if (!err.empty()) { std::cerr << "GLTF Error: " << err.c_str() << std::endl; }
        ASSERT(ret, "Failed to load GLTF file");

        std::wstring basePath = std::filesystem::path(filePath).parent_path().wstring();
        auto& rhiContext = RHI::RHIContext::GetInstance();

        LoadTexturesFromGLTF(model, basePath, commandQueue, commandBuffer);

        auto& entityManager = Core::EntitiesPool::GetInstance();
        auto& transformSystem = TransformSystem::GetInstance();
        auto& registry = entityManager.GetRegistry();

        std::vector<GLTFNodeEntity> entities;

        // Process all nodes to create entities
        for (int nodeIndex : model.scenes[0].nodes)
        {
            ProcessNode(model, nodeIndex, DirectX::SimpleMath::Matrix::Identity, entt::null, entities, commandQueue, commandBuffer);
        }

        // Set up components and hierarchy
        std::unordered_map<entt::entity, std::vector<entt::entity>> hierarchyMap;
        for (auto& nodeEntity : entities)
        {
            entt::entity entity = nodeEntity.entity;

            // Transform component
            auto& transform = registry.emplace<Components::Transform>(entity);
            transform.position = nodeEntity.transform.Translation();
            DirectX::SimpleMath::Quaternion quat;
            nodeEntity.transform.Decompose(transform.scale, quat, transform.position);
            transform.rotationAngles = quat.ToEuler();

            // Hierarchy component
            auto& hierarchy = registry.get<Components::HierarchyRelationship>(entity);
            hierarchyMap[entity] = {};

            // Transform flags and accumulated matrix
            registry.emplace<Components::TransformFlags>(entity);
            registry.emplace<Components::AccumulatedHierarchicalTransformMatrix>(entity);

            if (nodeEntity.meshIndex >= 0)
            {
                auto meshId = AssetsManager<StaticMesh>::GetInstance().CreateAsset(LoadMeshFromGLTF(model, nodeEntity.meshIndex, commandQueue, commandBuffer));
                registry.emplace<Components::InstancedStaticMesh>(entity, meshId);
                auto& mesh = AssetsManager<StaticMesh>::GetInstance().GetAsset(meshId);

                MeshPerInstanceData instanceData{
                    .transform = nodeEntity.transform,
                    .inverseTransposeTransform = nodeEntity.transform.Invert().Transpose()
                };
                // Convert to Core::Entity only where required by the API
                mesh.CreatePerInstanceData(Core::Entity{ entity }, instanceData);

                if (!mesh.m_submeshes.empty())
                {
                    MaterialParameters::PBRMaterial material{};
                    if (model.meshes[nodeEntity.meshIndex].primitives[0].material >= 0)
                    {
                        const auto& mat = model.materials[model.meshes[nodeEntity.meshIndex].primitives[0].material];
                        if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0)
                        {
                            const auto& tex = model.textures[mat.pbrMetallicRoughness.baseColorTexture.index];
                            std::string texName = model.images[tex.source].name.empty() ? "texture_" + std::to_string(tex.source) : model.images[tex.source].name;
                            auto& textureManager = AssetsManager<std::shared_ptr<RHI::ITexture>>::GetInstance();
                            material.albedoIndex = -1;
                        }
                    }
                    mesh.CreateSubmeshPerInstanceData<MaterialParameters::PBRMaterial>(Core::Entity{ entity }, 0u, material);
                }

                mesh.UpdateRHIBufferWithPerInstanceData(commandBuffer);
            }
            else if (nodeEntity.isLight)
            {
                auto& light = registry.emplace<Components::PointLight>(entity);
                light.color = nodeEntity.lightColor;
                light.intensity = nodeEntity.lightIntensity;

                auto& lightManager = LightSourcesManager::GetInstance();
                lightManager.AddPointLight(Core::Entity{ entity }); // Convert to Core::Entity for LightSourcesManager
                lightManager.UpdatePointLightParams(Core::Entity{ entity }, light);
            }

            transformSystem.MarkDirty(registry, entity);
        }

        // Build hierarchy relationships
        for (const auto& nodeEntity : entities)
        {
            auto& hierarchy = registry.get<Components::HierarchyRelationship>(nodeEntity.entity);
            if (hierarchy.parent != entt::null)
            {
                hierarchyMap[static_cast<entt::entity>(hierarchy.parent)].push_back(nodeEntity.entity);
            }
        }

        for (auto& [parent, children] : hierarchyMap)
        {
            if (!children.empty())
            {
                auto& parentHierarchy = registry.get<Components::HierarchyRelationship>(parent);
                parentHierarchy.children = children.size();
                parentHierarchy.first = children.front();

                for (size_t i = 0; i < children.size(); ++i)
                {
                    auto& childHierarchy = registry.get<Components::HierarchyRelationship>(children[i]);
                    childHierarchy.prev = (i > 0) ? children[i - 1] : entt::null;
                    childHierarchy.next = (i < children.size() - 1) ? children[i + 1] : entt::null;
                }
            }
        }
    }

    void AssetsLoader::LoadTexturesFromGLTF(const tinygltf::Model& model, const std::wstring& basePath, std::shared_ptr<RHI::ICommandQueue> commandQueue, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
    {
        RHI::BufferRegionCopyDescription copyDesc;
        auto& rhiContext = RHI::RHIContext::GetInstance();
        auto allocator = rhiContext.GetAllocator();

        for (const auto& texture : model.textures)
        {
            if (texture.source >= 0 && texture.source < model.images.size())
            {
                auto& textureManager = AssetsManager<std::shared_ptr<RHI::ITexture>>::GetInstance();
                const auto& image = model.images[texture.source];
                if (textureManager.IsAssetNameRegistered(image.name))
                {
                    continue;
                }
                std::string uri = image.uri;

                std::vector<unsigned char> imageData;
                int width = 0, height = 0, channels = 0;

                if (uri.empty() && !image.image.empty())
                {
                    width = image.width;
                    height = image.height;
                    channels = image.component;
                    imageData.assign(image.image.begin(), image.image.end());
                }
                else if (!uri.empty())
                {
                    std::wstring fullPath = basePath + L"/" + std::filesystem::path(uri).wstring();
                    std::string utf8Path = std::filesystem::path(fullPath).string();
                    unsigned char* loadedData = stbi_load(utf8Path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
                    ASSERT(loadedData, "Failed to load texture file: %s", utf8Path.c_str());

                    uint32_t dataSize = width * height * 4;
                    imageData.resize(dataSize);
                    memcpy(imageData.data(), loadedData, dataSize);
                    stbi_image_free(loadedData);
                }
                else
                {
                    std::cerr << "Invalid texture skipped during texture loading" << std::endl;
                    continue;
                }

                RHI::TextureDescription desc = {
                    .usage = static_cast<RHI::TextureUsage>(RHI::TextureUsage::eTextureUsage_Sampled | RHI::TextureUsage::eTextureUsage_TransferDestination),
                    .aspect = RHI::TextureAspect::eTextureAspect_HasColor,
                    .format = RHI::TextureFormat::RGBA8_UNORM,
                    .type = RHI::TextureType::Texture2D,
                    .layout = RHI::TextureLayout::Undefined,
                    .width = static_cast<uint32_t>(width),
                    .height = static_cast<uint32_t>(height),
                    .depth = 1,
                    .mipLevels = 1,
                    .arrayLayers = 1
                };
                auto texture = allocator->CreateTexture(desc);

                uint32_t textureSize = width * height * 4;
                if (textureSize > m_commonUploadBufferSize)
                {
                    m_commonUploadBufferSize = textureSize * 2;
                    RHI::BufferDescription newDesc = {
                        .elementsNum = 1,
                        .elementStride = m_commonUploadBufferSize,
                        .unstructuredSize = 0,
                        .access = RHI::BufferAccess::Upload,
                        .usage = RHI::BufferUsage::None,
                        .flags = {.requiredCopyStateToInit = false }
                    };
                    m_commonUploadBuffer = allocator->CreateBuffer(newDesc);
                }

                copyDesc.srcOffset = 0;
                copyDesc.destOffset = 0;
                copyDesc.width = textureSize;
                m_commonUploadBuffer->UploadData(imageData.data(), copyDesc);

                commandBuffer->BeginRecording(commandQueue);
                commandBuffer->CopyDataFromBufferToTexture(m_commonUploadBuffer, texture, { .srcOffset = 0, .destOffset = 0, .width = textureSize });
                commandBuffer->EndRecording();
                commandBuffer->SubmitToQueue(commandQueue);
                commandBuffer->ForceWaitUntilFinished(commandQueue);

                ASSERT(!image.name.empty(), "TEXTURE DOESN'T HAVE A NAME");
                std::string textureName = image.name;
                auto textureAssetId = textureManager.CreateAsset(std::move(texture));
                textureManager.AssignName(textureAssetId, textureName);
            }
        }
    }

    StaticMesh AssetsLoader::LoadMeshFromGLTF(const tinygltf::Model& model, int meshIndex, std::shared_ptr<RHI::ICommandQueue> commandQueue, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
    {
        const auto& mesh = model.meshes[meshIndex];
        StaticMesh staticMesh;
        RHI::BufferRegionCopyDescription copyDesc;
        auto& rhiContext = RHI::RHIContext::GetInstance();
        auto allocator = rhiContext.GetAllocator();

        for (int i = 0; i < mesh.primitives.size(); i++)
        {
            const auto& primitive = mesh.primitives[i];
            std::vector<float> positions;
            std::vector<float> normals;
            std::vector<uint32_t> indices;

            // Positions
            const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
            const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
            const auto& posBuffer = model.buffers[posBufferView.buffer];
            positions.resize(posAccessor.count * 3);
            memcpy(positions.data(), &posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset], posAccessor.count * 3 * sizeof(float));

            tinygltf::Accessor normAccessor;
            tinygltf::BufferView normBufferView;

            // Normals (optional)
            if (primitive.attributes.count("NORMAL"))
            {
                normAccessor = model.accessors[primitive.attributes.at("NORMAL")];
                normBufferView = model.bufferViews[normAccessor.bufferView];
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
                .elementsNum = static_cast<uint32_t>(posAccessor.count),
                .elementStride = sizeof(float) * 3,
                .unstructuredSize = 0,
                .access = RHI::BufferAccess::DefaultPrivate,
                .usage = RHI::BufferUsage::VertexBuffer,
                .flags = {.requiredCopyStateToInit = true }
            };
            auto vertexBuffer = allocator->CreateBuffer(vertexDesc);
            RHI::BufferWithRegionDescription vertexData = {
                .buffer = vertexBuffer,
                .regionDescription = {.offset = 0, .size = vertexDesc.elementsNum * vertexDesc.elementStride }
            };

            if (vertexData.regionDescription.size > m_commonUploadBufferSize)
            {
                m_commonUploadBufferSize = vertexData.regionDescription.size * 2;
                RHI::BufferDescription newDesc = {
                    .elementsNum = 1,
                    .elementStride = m_commonUploadBufferSize,
                    .unstructuredSize = 0,
                    .access = RHI::BufferAccess::Upload,
                    .usage = RHI::BufferUsage::None,
                    .flags = {.requiredCopyStateToInit = false }
                };
                m_commonUploadBuffer = allocator->CreateBuffer(newDesc);
            }

            copyDesc.srcOffset = vertexData.regionDescription.offset;
            copyDesc.destOffset = 0;
            copyDesc.width = vertexData.regionDescription.size;
            m_commonUploadBuffer->UploadData(positions.data(), copyDesc);

            commandBuffer->BeginRecording(commandQueue);
            commandBuffer->CopyDataBetweenBuffers(m_commonUploadBuffer, vertexData.buffer, { .srcOffset = 0, .destOffset = 0, .width = copyDesc.width });
            commandBuffer->EndRecording();
            commandBuffer->SubmitToQueue(commandQueue);
            commandBuffer->ForceWaitUntilFinished(commandQueue);

            RHI::BufferDescription normalDesc = {
                .elementsNum = normals.empty() ? 0 : static_cast<uint32_t>(normAccessor.count),
                .elementStride = sizeof(float) * 3,
                .unstructuredSize = 0,
                .access = RHI::BufferAccess::DefaultPrivate,
                .usage = RHI::BufferUsage::VertexBuffer,
                .flags = {.requiredCopyStateToInit = true }
            };
            auto normalBuffer = normals.empty() ? nullptr : allocator->CreateBuffer(normalDesc);
            RHI::BufferWithRegionDescription normalData = {
                .buffer = normalBuffer,
                .regionDescription = {.offset = 0, .size = normalDesc.elementsNum * normalDesc.elementStride }
            };
            if (normalBuffer)
            {
                if (normalData.regionDescription.size > m_commonUploadBufferSize)
                {
                    m_commonUploadBufferSize = normalData.regionDescription.size * 2;
                    RHI::BufferDescription newDesc = {
                        .elementsNum = 1,
                        .elementStride = m_commonUploadBufferSize,
                        .unstructuredSize = 0,
                        .access = RHI::BufferAccess::Upload,
                        .usage = RHI::BufferUsage::None,
                        .flags = {.requiredCopyStateToInit = false }
                    };
                    m_commonUploadBuffer = allocator->CreateBuffer(newDesc);
                }

                copyDesc.srcOffset = normalData.regionDescription.offset;
                copyDesc.destOffset = 0;
                copyDesc.width = normalData.regionDescription.size;
                m_commonUploadBuffer->UploadData(normals.data(), copyDesc);

                commandBuffer->BeginRecording(commandQueue);
                commandBuffer->CopyDataBetweenBuffers(m_commonUploadBuffer, normalData.buffer, { .srcOffset = 0, .destOffset = 0, .width = copyDesc.width });
                commandBuffer->EndRecording();
                commandBuffer->SubmitToQueue(commandQueue);
                commandBuffer->ForceWaitUntilFinished(commandQueue);
            }

            RHI::BufferDescription indexDesc = {
                .elementsNum = static_cast<uint32_t>(indexAccessor.count),
                .elementStride = sizeof(uint32_t),
                .unstructuredSize = 0,
                .access = RHI::BufferAccess::DefaultPrivate,
                .usage = RHI::BufferUsage::IndexBuffer,
                .flags = {.requiredCopyStateToInit = true }
            };
            auto rhiIndexBuffer = allocator->CreateBuffer(indexDesc);
            RHI::BufferWithRegionDescription indexData = {
                .buffer = rhiIndexBuffer,
                .regionDescription = {.offset = 0, .size = indexDesc.elementsNum * indexDesc.elementStride }
            };

            if (indexData.regionDescription.size > m_commonUploadBufferSize)
            {
                m_commonUploadBufferSize = indexData.regionDescription.size * 2;
                RHI::BufferDescription newDesc = {
                    .elementsNum = 1,
                    .elementStride = m_commonUploadBufferSize,
                    .unstructuredSize = 0,
                    .access = RHI::BufferAccess::Upload,
                    .usage = RHI::BufferUsage::None,
                    .flags = {.requiredCopyStateToInit = false }
                };
                m_commonUploadBuffer = allocator->CreateBuffer(newDesc);
            }

            copyDesc.srcOffset = indexData.regionDescription.offset;
            copyDesc.destOffset = 0;
            copyDesc.width = indexData.regionDescription.size;
            m_commonUploadBuffer->UploadData(indices.data(), copyDesc);

            commandBuffer->BeginRecording(commandQueue);
            commandBuffer->CopyDataBetweenBuffers(m_commonUploadBuffer, indexData.buffer, { .srcOffset = 0, .destOffset = 0, .width = copyDesc.width });
            commandBuffer->EndRecording();
            commandBuffer->SubmitToQueue(commandQueue);
            commandBuffer->ForceWaitUntilFinished(commandQueue);

            StaticSubmesh submesh(vertexData, normalData, indexData);
            staticMesh.m_submeshes.push_back(std::move(submesh));
        }

        return staticMesh;
    }

    void AssetsLoader::ProcessNode(const tinygltf::Model& model, int nodeIndex, const DirectX::SimpleMath::Matrix& parentTransform, entt::entity parentEntity, std::vector<GLTFNodeEntity>& entities, std::shared_ptr<RHI::ICommandQueue> commandQueue, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
    {
        const auto& node = model.nodes[nodeIndex];
        auto& entityManager = Core::EntitiesPool::GetInstance();
        entt::entity entity = entityManager.GetRegistry().create();

        DirectX::SimpleMath::Matrix localTransform = DirectX::SimpleMath::Matrix::Identity;
        if (node.matrix.size() == 16)
        {
            localTransform = DirectX::SimpleMath::Matrix(
                static_cast<float>(node.matrix[0]), static_cast<float>(node.matrix[1]), static_cast<float>(node.matrix[2]), static_cast<float>(node.matrix[3]),
                static_cast<float>(node.matrix[4]), static_cast<float>(node.matrix[5]), static_cast<float>(node.matrix[6]), static_cast<float>(node.matrix[7]),
                static_cast<float>(node.matrix[8]), static_cast<float>(node.matrix[9]), static_cast<float>(node.matrix[10]), static_cast<float>(node.matrix[11]),
                static_cast<float>(node.matrix[12]), static_cast<float>(node.matrix[13]), static_cast<float>(node.matrix[14]), static_cast<float>(node.matrix[15])
            );
        }
        else
        {
            DirectX::SimpleMath::Vector3 translation = node.translation.size() == 3 ? DirectX::SimpleMath::Vector3(node.translation[0], node.translation[1], node.translation[2]) : DirectX::SimpleMath::Vector3::Zero;
            DirectX::SimpleMath::Quaternion rotation = node.rotation.size() == 4 ? DirectX::SimpleMath::Quaternion(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]) : DirectX::SimpleMath::Quaternion::Identity;
            DirectX::SimpleMath::Vector3 scale = node.scale.size() == 3 ? DirectX::SimpleMath::Vector3(node.scale[0], node.scale[1], node.scale[2]) : DirectX::SimpleMath::Vector3::One;
            localTransform = DirectX::SimpleMath::Matrix::CreateScale(scale) * DirectX::SimpleMath::Matrix::CreateFromQuaternion(rotation) * DirectX::SimpleMath::Matrix::CreateTranslation(translation);
        }

        DirectX::SimpleMath::Matrix worldTransform = localTransform * parentTransform;

        GLTFNodeEntity nodeEntity{
            .entity = entity,
            .transform = worldTransform,
            .meshIndex = node.mesh,
            .isLight = false,
            .lightColor = DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f),
            .lightIntensity = 1.0f
        };

        if (node.name.find("Light") != std::string::npos)
        {
            nodeEntity.isLight = true;
        }

        entities.push_back(nodeEntity);

        auto& hierarchy = entityManager.GetRegistry().emplace<Components::HierarchyRelationship>(entity);
        hierarchy.parent = parentEntity;

        for (int childIndex : node.children)
        {
            ProcessNode(model, childIndex, worldTransform, entity, entities, commandQueue, commandBuffer);
        }
    }
}