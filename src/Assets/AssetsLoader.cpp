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
#include "../Renderer/RHI/D3D12/D3D12Texture.h"
#include <DirectXTex.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

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
                mesh.CreatePerInstanceData(Core::Entity{ entity }, instanceData);

                if (!mesh.m_submeshes.empty())
                {   
                    auto processTexture = [&](int32_t texIndex, uint32_t& outIndex) 
                    {
                        if (texIndex >= 0)
                        {
                            auto& textureManager = AssetsManager<std::shared_ptr<RHI::ITexture>>::GetInstance();

                            const auto& tex = model.textures[texIndex];
                            std::string texName = model.images[tex.source].name.empty() ? model.images[tex.source].uri : model.images[tex.source].name;

                            std::shared_ptr<RHI::ITexture> texture = textureManager.GetAssetByName(texName);
                            auto rhiTexture = textureManager.GetAssetByName(texName);
                            outIndex = std::static_pointer_cast<RHI::D3D12Texture>(rhiTexture)->m_SRVDescriptorIndex;
                        }
                    };

                    for (int submeshIndex = 0; submeshIndex < mesh.m_submeshes.size(); submeshIndex++)
                    {
                        MaterialParameters::PBRMaterial material{};
                        if (model.meshes[nodeEntity.meshIndex].primitives[submeshIndex].material >= 0)
                        {
                            const auto& mat = model.materials[model.meshes[nodeEntity.meshIndex].primitives[submeshIndex].material];
                            processTexture(mat.pbrMetallicRoughness.baseColorTexture.index, material.albedoIndex);
                            processTexture(mat.normalTexture.index, material.normalIndex);
                            processTexture(mat.pbrMetallicRoughness.metallicRoughnessTexture.index, material.metallicRoughnessIndex);
                            processTexture(mat.occlusionTexture.index, material.aoIndex);
                            processTexture(mat.emissiveTexture.index, material.emissiveIndex);
                        }
                        mesh.CreateSubmeshPerInstanceData<MaterialParameters::PBRMaterial>(Core::Entity{ entity }, submeshIndex, material);
                    }
                }
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

    void AssetsLoader::LoadTexturesFromGLTF(const tinygltf::Model& model, const std::wstring& basePath,
        std::shared_ptr<RHI::ICommandQueue> commandQueue, std::shared_ptr<RHI::ICommandBuffer> commandBuffer)
    {
        RHI::BufferRegionCopyDescription copyDesc;
        auto& rhiContext = RHI::RHIContext::GetInstance();
        auto allocator = rhiContext.GetAllocator();
        auto& textureManager = AssetsManager<std::shared_ptr<RHI::ITexture>>::GetInstance();

        const std::unordered_map<int, RHI::TextureFormat> formatMap = {
            {1, RHI::TextureFormat::R8_UNORM},
            {2, RHI::TextureFormat::RG8_UNORM},
            {3, RHI::TextureFormat::RGBA8_UNORM},
            {4, RHI::TextureFormat::RGBA8_UNORM}
        };

        for (const auto& texture : model.textures)
        {
            if (texture.source < 0 || texture.source >= model.images.size())
                continue;

            const auto& image = model.images[texture.source];
            if (textureManager.IsAssetNameRegistered(image.name))
                continue;

            std::vector<unsigned char> imageData;
            int width = 0, height = 0, channels = 0;
            bool isValidTexture = false;

            if (image.uri.empty() && !image.image.empty())
            {
                width = image.width;
                height = image.height;
                channels = image.component;
                if (width > 0 && height > 0 && channels > 0)
                {
                    if (channels == 3)
                    {
                        imageData.resize(width * height * 4);
                        for (int i = 0, j = 0; i < image.image.size(); i += 3, j += 4)
                        {
                            imageData[j] = image.image[i];
                            imageData[j + 1] = image.image[i + 1];
                            imageData[j + 2] = image.image[i + 2];
                            imageData[j + 3] = 255;
                        }
                        channels = 4;
                    }
                    else
                    {
                        imageData.assign(image.image.begin(), image.image.end());
                    }
                    isValidTexture = true;
                }
            }
            else if (!image.uri.empty())
            {
                std::wstring fullPath = basePath + L"/" + std::filesystem::path(image.uri).wstring();
                std::string utf8Path = std::filesystem::path(fullPath).string();

                int reqChannels = (image.component >= 3) ? STBI_rgb_alpha : 0;
                unsigned char* loadedData = stbi_load(utf8Path.c_str(), &width, &height, &channels, reqChannels);

                if (loadedData && width > 0 && height > 0 && channels > 0)
                {
                    if (reqChannels == STBI_rgb_alpha)
                    {
                        channels = 4;
                    }

                    uint32_t dataSize = width * height * channels;
                    imageData.resize(dataSize);
                    memcpy(imageData.data(), loadedData, dataSize);
                    stbi_image_free(loadedData);

                    isValidTexture = true;
                }
                else
                {
                    std::cerr << "Failed to load texture file: " << utf8Path << std::endl;
                }
            }

            if (!isValidTexture)
            {
                std::cerr << "Invalid texture skipped: " << (image.name.empty() ? image.uri : image.name) << std::endl;
                continue;
            }

            auto formatIt = formatMap.find(channels);
            RHI::TextureFormat textureFormat = (formatIt != formatMap.end())
                ? formatIt->second
                : RHI::TextureFormat::RGBA8_UNORM;

            uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

            RHI::TextureDescription desc = {
                .usage = static_cast<RHI::TextureUsage>(RHI::TextureUsage::eTextureUsage_Sampled |
                                                      RHI::TextureUsage::eTextureUsage_TransferDestination),
                .aspect = RHI::TextureAspect::eTextureAspect_HasColor,
                .format = textureFormat,
                .type = RHI::TextureType::Texture2D,
                .layout = RHI::TextureLayout::Undefined,
                .width = static_cast<uint32_t>(width),
                .height = static_cast<uint32_t>(height),
                .depth = 1,
                .mipLevels = mipLevels,
                .arrayLayers = 1
            };

            auto texture = allocator->CreateTexture(desc);
            if (!texture)
            {
                std::cerr << "Failed to create texture: " << (image.name.empty() ? image.uri : image.name) << std::endl;
                continue;
            }

            DirectX::ScratchImage scratchImage;
            DirectX::ScratchImage mipChain;
            DXGI_FORMAT dxgiFormat = RHI::ConvertFormatToD3D12(textureFormat);

            HRESULT hr = scratchImage.Initialize2D(dxgiFormat, width, height, 1, 1);
            if (FAILED(hr))
            {
                std::cerr << "Failed to initialize scratch image for texture: " << (image.name.empty() ? image.uri : image.name) << std::endl;
                continue;
            }

            memcpy(scratchImage.GetPixels(), imageData.data(), imageData.size());

            hr = DirectX::GenerateMipMaps(*scratchImage.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, mipChain);
            if (FAILED(hr))
            {
                std::cerr << "Failed to generate mipmaps for texture: " << (image.name.empty() ? image.uri : image.name) << std::endl;
                scratchImage.Release();
                continue;
            }

            std::vector<uint8_t> uploadData;
            size_t totalSize = 0;
            for (size_t i = 0; i < mipChain.GetImageCount(); ++i)
            {
                const DirectX::Image* mipImage = mipChain.GetImage(i, 0, 0);
                size_t mipSize = mipImage->rowPitch * mipImage->height;
                totalSize += mipSize;
            }

            uploadData.resize(totalSize);
            size_t currentOffset = 0;
            for (size_t i = 0; i < mipChain.GetImageCount(); ++i)
            {
                const DirectX::Image* mipImage = mipChain.GetImage(i, 0, 0);
                size_t mipSize = mipImage->rowPitch * mipImage->height;
                memcpy(uploadData.data() + currentOffset, mipImage->pixels, mipSize);
                currentOffset += mipSize;
            }

            if (totalSize > m_commonUploadBufferSize)
            {
                m_commonUploadBufferSize = totalSize * 2;
                RHI::BufferDescription bufferDesc = {
                    .elementsNum = 1,
                    .elementStride = static_cast<uint32_t>(m_commonUploadBufferSize),
                    .unstructuredSize = 0,
                    .access = RHI::BufferAccess::Upload,
                    .usage = RHI::BufferUsage::None,
                    .flags = {.requiredCopyStateToInit = false }
                };
                m_commonUploadBuffer = allocator->CreateBuffer(bufferDesc);
            }

            copyDesc.srcOffset = 0;
            copyDesc.destOffset = 0;
            copyDesc.width = static_cast<uint32_t>(totalSize);
            m_commonUploadBuffer->UploadData(uploadData.data(), copyDesc);

            commandBuffer->BeginRecording(commandQueue);
            RHI::TextureRegionCopyDescription regionCopyDesc = {
                .srcOffset = 0
            };
            commandBuffer->CopyDataFromBufferToTexture(m_commonUploadBuffer, texture, regionCopyDesc);
            commandBuffer->EndRecording();
            commandBuffer->SubmitToQueue(commandQueue);
            commandBuffer->ForceWaitUntilFinished(commandQueue);

            scratchImage.Release();
            mipChain.Release();

            std::string textureName = image.name.empty() ? image.uri : image.name;

            auto textureAssetId = textureManager.CreateAsset(std::move(texture));
            textureManager.AssignName(textureAssetId, textureName);
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
            std::vector<Assets::VertexPrimaryAttributes> primaryAttributes;
            std::vector<Assets::VertexSecondaryAttributes> secondaryAttributes;
            std::vector<uint32_t> indices;

            // Load primary attributes
            const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
            primaryAttributes.resize(posAccessor.count);
            const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
            const auto& posBuffer = model.buffers[posBufferView.buffer];
            const float* posData = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);
            for (size_t j = 0; j < posAccessor.count; ++j)
            {
                primaryAttributes[j].position = DirectX::SimpleMath::Vector3(posData[j * 3 + 0], posData[j * 3 + 1], posData[j * 3 + 2]);
            }

            if (primitive.attributes.count("NORMAL"))
            {
                const auto& normAccessor = model.accessors[primitive.attributes.at("NORMAL")];
                const auto& normBufferView = model.bufferViews[normAccessor.bufferView];
                const auto& normBuffer = model.buffers[normBufferView.buffer];
                const float* normData = reinterpret_cast<const float*>(&normBuffer.data[normBufferView.byteOffset + normAccessor.byteOffset]);
                for (size_t j = 0; j < normAccessor.count; ++j)
                {
                    primaryAttributes[j].normals = DirectX::SimpleMath::Vector3(normData[j * 3 + 0], normData[j * 3 + 1], normData[j * 3 + 2]);
                }
            }

            if (primitive.attributes.count("TEXCOORD_0"))
            {
                const auto& texAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
                const auto& texBufferView = model.bufferViews[texAccessor.bufferView];
                const auto& texBuffer = model.buffers[texBufferView.buffer];
                const float* texData = reinterpret_cast<const float*>(&texBuffer.data[texBufferView.byteOffset + texAccessor.byteOffset]);
                for (size_t j = 0; j < texAccessor.count; ++j)
                {
                    primaryAttributes[j].texCoords = DirectX::SimpleMath::Vector2(texData[j * 2 + 0], texData[j * 2 + 1]);
                }
            }

            // Load or generate secondary attributes
            bool hasSecondary = primitive.attributes.count("TANGENT");
            secondaryAttributes.resize(posAccessor.count);
            if (hasSecondary)
            {
                const auto& tangentAccessor = model.accessors[primitive.attributes.at("TANGENT")];
                const auto& tangentBufferView = model.bufferViews[tangentAccessor.bufferView];
                const auto& tangentBuffer = model.buffers[tangentBufferView.buffer];
                const float* tangentData = reinterpret_cast<const float*>(&tangentBuffer.data[tangentBufferView.byteOffset + tangentAccessor.byteOffset]);
                for (size_t j = 0; j < tangentAccessor.count; ++j)
                {
                    secondaryAttributes[j].Tangent = DirectX::SimpleMath::Vector3(tangentData[j * 4 + 0], tangentData[j * 4 + 1], tangentData[j * 4 + 2]);
                    secondaryAttributes[j].Bitangent = primaryAttributes[j].normals.Cross(secondaryAttributes[j].Tangent);
                }
            }
            else if (primitive.attributes.count("TEXCOORD_0") && primitive.attributes.count("NORMAL"))
            {
                // Generate tangents and bitangents
                std::vector<DirectX::SimpleMath::Vector3> tangents(posAccessor.count, DirectX::SimpleMath::Vector3::Zero);
                std::vector<DirectX::SimpleMath::Vector3> bitangents(posAccessor.count, DirectX::SimpleMath::Vector3::Zero);
                std::vector<uint32_t> counts(posAccessor.count, 0);

                for (size_t j = 0; j < indices.size(); j += 3)
                {
                    uint32_t i0 = indices[j + 0];
                    uint32_t i1 = indices[j + 1];
                    uint32_t i2 = indices[j + 2];

                    const auto& p0 = primaryAttributes[i0].position;
                    const auto& p1 = primaryAttributes[i1].position;
                    const auto& p2 = primaryAttributes[i2].position;

                    const auto& uv0 = primaryAttributes[i0].texCoords;
                    const auto& uv1 = primaryAttributes[i1].texCoords;
                    const auto& uv2 = primaryAttributes[i2].texCoords;

                    DirectX::SimpleMath::Vector3 e1 = p1 - p0;
                    DirectX::SimpleMath::Vector3 e2 = p2 - p0;
                    DirectX::SimpleMath::Vector2 duv1 = uv1 - uv0;
                    DirectX::SimpleMath::Vector2 duv2 = uv2 - uv0;

                    float r = 1.0f / (duv1.x * duv2.y - duv2.x * duv1.y + 0.0001f); // Avoid division by zero
                    DirectX::SimpleMath::Vector3 tangent = (e1 * duv2.y - e2 * duv1.y) * r;
                    DirectX::SimpleMath::Vector3 bitangent = (e2 * duv1.x - e1 * duv2.x) * r;

                    // Accumulate for averaging
                    tangents[i0] += tangent;
                    tangents[i1] += tangent;
                    tangents[i2] += tangent;
                    bitangents[i0] += bitangent;
                    bitangents[i1] += bitangent;
                    bitangents[i2] += bitangent;
                    counts[i0]++;
                    counts[i1]++;
                    counts[i2]++;
                }

                // Average and orthogonalize
                for (size_t j = 0; j < posAccessor.count; ++j)
                {
                    if (counts[j] > 0)
                    {
                        secondaryAttributes[j].Tangent = tangents[j] / static_cast<float>(counts[j]);
                        secondaryAttributes[j].Bitangent = bitangents[j] / static_cast<float>(counts[j]);

                        // Orthogonalize tangent against normal
                        const auto& normal = primaryAttributes[j].normals;
                        secondaryAttributes[j].Tangent = (secondaryAttributes[j].Tangent - normal * normal.Dot(secondaryAttributes[j].Tangent));
                        secondaryAttributes[j].Tangent.Normalize();
                        secondaryAttributes[j].Bitangent = normal.Cross(secondaryAttributes[j].Tangent);
                    }
                    else
                    {
                        // Fallback for unreferenced vertices
                        secondaryAttributes[j].Tangent = DirectX::SimpleMath::Vector3(1.0f, 0.0f, 0.0f);
                        secondaryAttributes[j].Bitangent = DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
                    }
                }
            }
            else
            {
                // No texCoords or normals; use defaults
                for (size_t j = 0; j < posAccessor.count; ++j)
                {
                    secondaryAttributes[j].Tangent = DirectX::SimpleMath::Vector3(1.0f, 0.0f, 0.0f);
                    secondaryAttributes[j].Bitangent = DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
                }
            }

            // Load indices
            const auto& indexAccessor = model.accessors[primitive.indices];
            indices.resize(indexAccessor.count);
            const auto& indexBufferView = model.bufferViews[indexAccessor.bufferView];
            const auto& indexBuffer = model.buffers[indexBufferView.buffer];
            if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
            {
                const uint16_t* indexData = reinterpret_cast<const uint16_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
                for (size_t j = 0; j < indexAccessor.count; ++j)
                {
                    indices[j] = static_cast<uint32_t>(indexData[j]);
                }
            }
            else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
            {
                memcpy(indices.data(), &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset], indexAccessor.count * sizeof(uint32_t));
            }
            else
            {
                ASSERT(false, "Unsupported index component type");
            }

            // Create primary vertex buffer
            RHI::BufferDescription vertexDesc = {
                .elementsNum = static_cast<uint32_t>(posAccessor.count),
                .elementStride = sizeof(Assets::VertexPrimaryAttributes),
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

            copyDesc.srcOffset = 0;
            copyDesc.destOffset = 0;
            copyDesc.width = vertexData.regionDescription.size;
            m_commonUploadBuffer->UploadData(primaryAttributes.data(), copyDesc);

            commandBuffer->BeginRecording(commandQueue);
            commandBuffer->CopyDataBetweenBuffers(m_commonUploadBuffer, vertexData.buffer, { .srcOffset = 0, .destOffset = 0, .width = copyDesc.width });
            commandBuffer->EndRecording();
            commandBuffer->SubmitToQueue(commandQueue);
            commandBuffer->ForceWaitUntilFinished(commandQueue);

            // Create secondary vertex buffer
            RHI::BufferDescription secondaryDesc = {
                .elementsNum = static_cast<uint32_t>(posAccessor.count),
                .elementStride = sizeof(Assets::VertexSecondaryAttributes),
                .unstructuredSize = 0,
                .access = RHI::BufferAccess::DefaultPrivate,
                .usage = RHI::BufferUsage::VertexBuffer,
                .flags = {.requiredCopyStateToInit = true }
            };
            auto secondaryBuffer = allocator->CreateBuffer(secondaryDesc);
            RHI::BufferWithRegionDescription secondaryData = {
                .buffer = secondaryBuffer,
                .regionDescription = {.offset = 0, .size = secondaryDesc.elementsNum * secondaryDesc.elementStride }
            };

            if (secondaryData.regionDescription.size > m_commonUploadBufferSize)
            {
                m_commonUploadBufferSize = secondaryData.regionDescription.size * 2;
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
            copyDesc.width = secondaryData.regionDescription.size;
            m_commonUploadBuffer->UploadData(secondaryAttributes.data(), copyDesc);

            commandBuffer->BeginRecording(commandQueue);
            commandBuffer->CopyDataBetweenBuffers(m_commonUploadBuffer, secondaryData.buffer, { .srcOffset = 0, .destOffset = 0, .width = copyDesc.width });
            commandBuffer->EndRecording();
            commandBuffer->SubmitToQueue(commandQueue);
            commandBuffer->ForceWaitUntilFinished(commandQueue);

            // Create index buffer
            RHI::BufferDescription indexDesc = {
                .elementsNum = static_cast<uint32_t>(indexAccessor.count),
                .elementStride = sizeof(uint32_t),
                .unstructuredSize = 0,
                .access = RHI::BufferAccess::DefaultPrivate,
                .usage = RHI::BufferUsage::IndexBuffer,
                .flags = {.requiredCopyStateToInit = true }
            };
            auto indexRHIBuffer = allocator->CreateBuffer(indexDesc);
            RHI::BufferWithRegionDescription indexData = {
                .buffer = indexRHIBuffer,
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

            copyDesc.srcOffset = 0;
            copyDesc.destOffset = 0;
            copyDesc.width = indexData.regionDescription.size;
            m_commonUploadBuffer->UploadData(indices.data(), copyDesc);

            commandBuffer->BeginRecording(commandQueue);
            commandBuffer->CopyDataBetweenBuffers(m_commonUploadBuffer, indexData.buffer, { .srcOffset = 0, .destOffset = 0, .width = copyDesc.width });
            commandBuffer->EndRecording();
            commandBuffer->SubmitToQueue(commandQueue);
            commandBuffer->ForceWaitUntilFinished(commandQueue);

            // Create submesh
            StaticSubmesh submesh(vertexData, secondaryData, indexData);
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