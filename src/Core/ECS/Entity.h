#pragma once
#include <entt/entt.hpp>
#include "./Managers/EntitiesPool.h"

namespace Core
{
	struct Entity
	{
		template<typename ComponentT, typename... Args>
		ComponentT& AddComponent(Args&&... args)
		{
			return EntitiesPool::GetInstance().m_registry.emplace<ComponentT>(m_enityId, std::forward<Args>(args)...);
		}

		template<typename ComponentT>
		void RemoveComponent()
		{
			EntitiesPool::GetInstance().m_registry.remove<ComponentT>(m_enityId);
		}

		template<typename ComponentT>
		const ComponentT& GetComponent() const
		{
			return EntitiesPool::GetInstance().m_registry.get<ComponentT>(m_enityId);
		}

		template<typename ComponentT>
		ComponentT& GetComponent()
		{
			return EntitiesPool::GetInstance().m_registry.get<ComponentT>(m_enityId);
		}

		template<typename... ComponentTs>
		bool HasComponent() const
		{
			return EntitiesPool::GetInstance().m_registry.all_of<ComponentTs...>(m_enityId);
		}

		operator entt::entity() const { return m_enityId; }

		bool operator==(const Entity& other) const { return m_enityId == other.m_enityId; }

		entt::entity m_enityId;
	};
}