#pragma once

#include <SDL2/SDL_surface.h>

#include <unordered_map>
#include <typeinfo>
#include <typeindex>

#include "Core/World.h"

class Entity;

class Component
{
public:
	Component();
	virtual ~Component();
	int id = 0;

protected:
	Entity *owner = nullptr;

public:
	int get_id() const
	{
		return id;
	};

	void set_id(int value)
	{
		id = value;
	};

	Entity *get_owner() const;
	void set_owner(Entity *value);

	template <typename C>
	C *get_owner()
	{
		return dynamic_cast<C *>(owner);
	}

	template <typename C>
	C *get_owner_component();
};

#include "Entities/Entity.h"

template <typename C>
C *Component::get_owner_component()
{
	return static_cast<C *>(owner->get_component<C>());
}
