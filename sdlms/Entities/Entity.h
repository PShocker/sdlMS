#pragma once

#include <unordered_map>
#include <typeindex>
#include <typeinfo>

class Component;

class Entity
{
private:
	unsigned long id = 0;
	std::unordered_map<std::type_index, Component *> component_refs;
	std::unordered_map<std::type_index, Entity *> entity_refs;

public:
	Entity();
	virtual ~Entity();

	unsigned long get_id() const;
	void set_id(unsigned long value);

	template <typename C>
	void add_component(C *comp)
	{
		remove_component<C>();
		component_refs[typeid(C)] = comp;
		if (comp != nullptr)
		{
			comp->set_owner(this);
		}
	}

	template <typename C>
	void remove_component()
	{
		if (component_refs.contains(typeid(C)))
		{
			C *comp = static_cast<C *>(component_refs[typeid(C)]);
			comp->set_owner(nullptr);
		}
		component_refs.erase(typeid(C));
	}

	template <typename C>
	C *get_component();

	template <typename C>
	void add_entity(C *ent)
	{
		entity_refs[typeid(C)] = ent;
	}
	template <typename C>
	void remove_entity()
	{
		entity_refs.erase(typeid(C));
	}

	template <typename C>
	C *get_entity();

	const std::unordered_map<std::type_index, Component *> &get_components();
};

#include "Components/Component.h"

template <typename C>
C *Entity::get_component()
{
	[[likely]]
	if (component_refs.contains(typeid(C)))
	{
		return static_cast<C *>(component_refs[typeid(C)]);
	}
	else
	{
		return nullptr;
	}
}

template <typename C>
inline C *Entity::get_entity()
{
	if (entity_refs.contains(typeid(C)))
	{
		return static_cast<C *>(entity_refs[typeid(C)]);
	}
	else
	{
		return nullptr;
	}
}
