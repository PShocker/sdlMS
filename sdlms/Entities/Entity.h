#pragma once

#include <unordered_map>
#include <typeindex>
#include <typeinfo>

class Component;

class Entity {
private:
	unsigned long id;
	std::unordered_map<std::type_index, Component*> component_refs;

public:
	Entity();
	~Entity();

	unsigned long get_id() const;
	void set_id(unsigned long value);

	template <typename C>
	void add_component(C* comp) {
		component_refs[typeid(C)] = comp;
		comp->set_owner(this);
	}

	template <typename C>
	C* get_component();

	const std::unordered_map<std::type_index, Component*>& get_components();
};


#include "Components/Component.h"

template <typename C>
C* Entity::get_component() {
	return static_cast<C*>(component_refs[typeid(C)]);
}
