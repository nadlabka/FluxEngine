#pragma once
#include <entt/entt.hpp>

namespace Components
{
    struct HierarchyRelationship
    {
        std::size_t children{};
        entt::entity first{ entt::null };
        entt::entity prev{ entt::null };
        entt::entity next{ entt::null };
        entt::entity parent{ entt::null };
    };
}