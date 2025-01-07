#pragma once
#include <entt/entt.hpp>

namespace Core
{
	struct Entity;

	class EntitiesPool
	{
	public:
		static EntitiesPool& GetInstance()
		{
			static EntitiesPool instance;
			return instance;
		}

		EntitiesPool(const EntitiesPool& arg) = delete;
		EntitiesPool& operator=(const EntitiesPool& arg) = delete;

		Entity CreateEntity();

		void RemoveEntity(const Entity& entity);

		entt::registry& GetRegistry() { return m_registry; }

		void Destroy();

	private:
		EntitiesPool() {}

		friend struct Entity;

		entt::registry m_registry;
	};
}
