#pragma once

#include <unordered_map>
#include <typeindex>
#include <typeinfo>

class Component;

class Entity
{
private:
	int id = 0;
	std::unordered_map<std::type_index, Component *> component_refs;
	std::unordered_map<std::type_index, std::unordered_map<int, Entity *>> entity_refs;

public:
	Entity();
	virtual ~Entity();

	int get_id() const;
	void set_id(int value);

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

	int add_entity(Entity *ent);
	void add_entity(Entity *ent, int index);

	template <typename C>
	void remove_entity(int index)
	{
		entity_refs[typeid(C)].erase(index);
	}
	
	void remove_entity(Entity *ent);

	template <typename C>
	C *get_entity(int index)
	{
		if (entity_refs.contains(typeid(C)) && entity_refs[typeid(C)].contains(index))
		{
			return static_cast<C *>(entity_refs[typeid(C)][index]);
		}
		else
		{
			return nullptr;
		}
	}

	template <typename C>
	std::unordered_map<int, C *> &get_entity()
	{
		return *reinterpret_cast<std::unordered_map<int, C *> *>(&entity_refs[typeid(C)]);
	}

	const std::unordered_map<std::type_index, Component *> &get_components();
};

#include "Components/Component.h"

template <typename C>
C *Entity::get_component()
{
	if (component_refs.contains(typeid(C)))
	{
		return static_cast<C *>(component_refs[typeid(C)]);
	}
	else
	{
		return nullptr;
	}
}
