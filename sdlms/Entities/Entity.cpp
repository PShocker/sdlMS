#include "Entity.h"

Entity::Entity() {}

Entity::~Entity() {}

unsigned long Entity::get_id() const
{
	return id;
}

void Entity::set_id(unsigned long value)
{
	id = value;
}

void Entity::add_entity(Entity *ent)
{
	auto index = entity_refs[typeid(*ent)].size();
	add_entity(ent, index);
}

void Entity::add_entity(Entity *ent, int index)
{
	entity_refs[typeid(*ent)][index] = ent;
}

const std::unordered_map<std::type_index, Component *> &Entity::get_components()
{
	return component_refs;
}
