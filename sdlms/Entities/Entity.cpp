#include "Entity.h"

Entity::Entity() {}

Entity::~Entity() {}

int Entity::get_id() const
{
	return id;
}

void Entity::set_id(int value)
{
	id = value;
}

int Entity::add_entity(Entity *ent)
{
	auto index = entity_refs[typeid(*ent)].size();
	add_entity(ent, index);
	return index;
}

void Entity::add_entity(Entity *ent, int index)
{
	entity_refs[typeid(*ent)][index] = ent;
}

void Entity::remove_entity(Entity *ent)
{
	auto &target_map = entity_refs[typeid(*ent)];
	for (auto it = target_map.begin(); it != target_map.end();)
	{
		[[unlikely]]
		if (it->second == ent)
		{
			it = target_map.erase(it); // 删除匹配值的元素，并返回指向下一个元素的迭代器
		}
		else
		{
			++it;
		}
	}
}

const std::unordered_map<std::type_index, Component *> &Entity::get_components()
{
	return component_refs;
}
