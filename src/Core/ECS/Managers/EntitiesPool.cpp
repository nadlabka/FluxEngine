#include "stdafx.h"
#include <ECS/Entity.h>
#include "EntitiesPool.h"

Core::Entity Core::EntitiesPool::CreateEntity()
{
	return { m_registry.create() };
}

void Core::EntitiesPool::RemoveEntity(const Entity& entity)
{
	m_registry.destroy(entity.m_enityId);
}

void Core::EntitiesPool::Destroy()
{
	m_registry.clear();
}
