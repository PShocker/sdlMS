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

	int get_id()
	{
		return id;
	};
	void set_id(int value)
	{
		id = value;
	};

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

	int add_entity(Entity *ent)
	{
		auto index = entity_refs[typeid(*ent)].size();
		add_entity(ent, index);
		return index;
	};

	void add_entity(Entity *ent, int index)
	{
		entity_refs[typeid(*ent)][index] = ent;
	};

	template <typename C>
	void remove_entity(int index)
	{
		entity_refs[typeid(C)].erase(index);
	}

	void remove_entity(Entity *ent)
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
	};

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

	const std::unordered_map<std::type_index, Component *> &get_components()
	{
		return component_refs;
	};
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
