#pragma once
#include <vector>
#include <unordered_map>
#include "../Entity.h"

class TransformSystem
{
public:
    static TransformSystem& GetInstance()
    {
        static TransformSystem instance;
        return instance;
    }

    TransformSystem(const TransformSystem&) = delete;
    TransformSystem& operator=(const TransformSystem&) = delete;

    void MarkDirty(entt::registry& registry, entt::entity entity);
    void UpdateMarkedTransforms(entt::registry& registry);

private:
    TransformSystem() {}
    ~TransformSystem() {}

    void PropagateDirty(entt::registry& registry, entt::entity entity);
    void RemoveDirtyRoot(entt::entity entity);
    
    std::vector<entt::entity> m_dirtyRoots;
    std::unordered_map<entt::entity, std::size_t> m_dirtyRootsMap;
};