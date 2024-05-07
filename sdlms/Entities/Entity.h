#pragma once

#include <unordered_map>
#include <typeindex>
#include <typeinfo>

class Component;

class Entity
{
private:
	unsigned long id;
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
		component_refs[typeid(C)] = comp;
		comp->set_owner(this);
	}

	template <typename C>
	void del_component()
	{
		C *comp = static_cast<C *>(component_refs[typeid(C)]);
		component_refs.erase(typeid(C));
		comp->set_owner(nullptr);
	}

	template <typename C>
	C *get_component();

	template <typename C>
	void add_entity(C *ent)
	{
		entity_refs[typeid(C)] = ent;
	}

	template <typename C>
	C *get_entity();

	const std::unordered_map<std::type_index, Component *> &get_components();
};

#include "Components/Component.h"

template <typename C>
C *Entity::get_component()
{
	return static_cast<C *>(component_refs[typeid(C)]);
}

template <typename C>
inline C *Entity::get_entity()
{
	return static_cast<C *>(entity_refs[typeid(C)]);
}
