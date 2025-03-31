#include "stdafx.h"
#include "TransformSystem.h"
#include <ECS/Components/HierarchyRelationship.h>
#include <ECS/Components/Transform.h>
#include <DebugMacros.h>
#include <queue>
#include <ECS/Components/InstancedStaticMesh.h>
#include <ECS/Components/LightSources.h>
#include "../../../Renderer/Managers/LightSourcesManager.h"

void TransformSystem::MarkDirty(entt::registry& registry, entt::entity entity)
{
    ASSERT(registry.all_of<Components::TransformFlags>(entity), "Entity HAS TO have TransformFlags component");

    auto& transformFlag = registry.get<Components::TransformFlags>(entity);
    if (!transformFlag.isDirty)
    {
        transformFlag.isDirty = true;

        if (m_dirtyRootsMap.find(entity) == m_dirtyRootsMap.end())
        {
            m_dirtyRootsMap[entity] = m_dirtyRoots.size();
            m_dirtyRoots.push_back(entity);
        }

        PropagateDirty(registry, entity);
    }
}

void TransformSystem::UpdateMarkedTransforms(entt::registry& registry)
{
    for (auto rootEntity : m_dirtyRoots)
    {
        // Queue for BFS traversal
        std::queue<entt::entity> queue;
        queue.push(rootEntity);

        auto& hierarchy = registry.get<Components::HierarchyRelationship>(rootEntity);
        auto& localTransform = registry.get<Components::Transform>(rootEntity);
        Matrix localMatrix = Matrix::CreateScale(localTransform.scale) *
            Matrix::CreateFromYawPitchRoll(localTransform.rotationAngles) *
            Matrix::CreateTranslation(localTransform.position);
        auto& accumulatedTransform = registry.get<Components::AccumulatedHierarchicalTransformMatrix>(rootEntity);
        if (hierarchy.parent != entt::null)
        {
            auto& parentAccumulatedTransform = registry.get<Components::AccumulatedHierarchicalTransformMatrix>(hierarchy.parent);
            accumulatedTransform.matrix = localMatrix * parentAccumulatedTransform.matrix;
        }
        else
        {
            accumulatedTransform.matrix = localMatrix;
        }

        while (!queue.empty())
        {
            entt::entity current = queue.front();
            queue.pop();

            auto& transformFlag = registry.get<Components::TransformFlags>(current);
            transformFlag.isDirty = false;

            auto& localTransform = registry.get<Components::Transform>(current);
            auto& accumulatedTransform = registry.get<Components::AccumulatedHierarchicalTransformMatrix>(current);

            Matrix localMatrix = Matrix::CreateScale(localTransform.scale) *
                Matrix::CreateFromYawPitchRoll(localTransform.rotationAngles) *
                Matrix::CreateTranslation(localTransform.position);

            auto& hierarchy = registry.get<Components::HierarchyRelationship>(current);
            entt::entity child = hierarchy.first;
            while (child != entt::null)
            {
                auto& childLocalTransform = registry.get<Components::Transform>(child);
                auto& childAccumulatedTransform = registry.get<Components::AccumulatedHierarchicalTransformMatrix>(child);

                Matrix childLocalMatrix = Matrix::CreateScale(childLocalTransform.scale) *
                    Matrix::CreateFromYawPitchRoll(childLocalTransform.rotationAngles) *
                    Matrix::CreateTranslation(childLocalTransform.position);

                childAccumulatedTransform.matrix = childLocalMatrix * accumulatedTransform.matrix;

                queue.push(child);

                auto& childHierarchy = registry.get<Components::HierarchyRelationship>(child);
                child = childHierarchy.next;
            }

            if (registry.all_of<Components::InstancedStaticMesh>(current))
            {
                auto& meshComponent = registry.get<Components::InstancedStaticMesh>(current);
                Assets::AssetsManager<Assets::StaticMesh>::GetInstance().GetAsset(meshComponent.staticMesh).UpdatePerInstanceData((Core::Entity)current, { accumulatedTransform.matrix });
            }

            if (registry.all_of<Components::PointLight>(current))
            {
                LightSourcesManager::GetInstance().UpdateDirectionalLightTransform((Core::Entity)current, accumulatedTransform.matrix);
            }
            else if (registry.all_of<Components::SpotLight>(current))
            {
                LightSourcesManager::GetInstance().UpdateDirectionalLightTransform((Core::Entity)current, accumulatedTransform.matrix);
            }
            else if (registry.all_of<Components::DirectionalLight>(current))
            {
                LightSourcesManager::GetInstance().UpdateDirectionalLightTransform((Core::Entity)current, accumulatedTransform.matrix);
            }
        }
    }

    m_dirtyRoots.clear();
    m_dirtyRootsMap.clear();
}

void TransformSystem::PropagateDirty(entt::registry& registry, entt::entity entity)
{
    ASSERT(registry.all_of<Components::HierarchyRelationship>(entity), "Entity HAS TO have HierarchyRelationship component");

    // Queue for BFS traversal
    std::queue<entt::entity, std::deque<entt::entity>> queue;
    queue.push(entity);

    while (!queue.empty())
    {
        entt::entity current = queue.front();
        queue.pop();

        auto& hierarchy = registry.get<Components::HierarchyRelationship>(current);

        entt::entity child = hierarchy.first;
        while (child != entt::null)
        {
            auto& childTransformFlag = registry.get<Components::TransformFlags>(child);
            if (!childTransformFlag.isDirty)
            {
                childTransformFlag.isDirty = true;
                queue.push(child);
            }
            else
            {
                RemoveDirtyRoot(child);
            }

            auto& childHierarchy = registry.get<Components::HierarchyRelationship>(child);
            child = childHierarchy.next;
        }
    }
}

void TransformSystem::RemoveDirtyRoot(entt::entity entity)
{
    auto it = m_dirtyRootsMap.find(entity);
    ASSERT(it != m_dirtyRootsMap.end(), "Trying to remove dirty transform root, that was not previously marked as dirty root");

    // Swap the entity with the last element in the vector
    std::size_t index = it->second;
    entt::entity lastEntity = m_dirtyRoots.back();
    m_dirtyRoots[index] = lastEntity;
    m_dirtyRoots.pop_back();

    // Update the map for the swapped entity
    m_dirtyRootsMap[lastEntity] = index;
    m_dirtyRootsMap.erase(it);
}

