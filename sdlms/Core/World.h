#pragma once

#include "Entities/Entity.h"
#include "Components/Component.h"
#include "Systems/System.h"
#include "Resource/Resource.h"

#include <unordered_map>
#include <map>

#include <typeinfo>
#include <typeindex>
#include <cstdint>

#include "Core/Window.h"
#include <vector>
#include <SDL2/SDL.h>

// 重载 SDL_Point 的加法运算符
SDL_FPoint operator+(const SDL_FPoint &m, const SDL_FPoint &n);
// 重载 SDL_Point 的减法运算符
SDL_FPoint operator-(const SDL_FPoint &m, const SDL_FPoint &n);

class World
{
private:
	static unsigned long EntityCounter;

	struct ComponentStore
	{
		std::type_index type;
		Component *component;
	};

	std::unordered_map<std::type_index, std::multimap<int, Entity *>> entity_map;
	std::unordered_map<std::type_index, std::multimap<int, Component *>> component_map;
	std::vector<System *> system_list;

	SDL_Event event_handler;

	std::uint64_t dt_now;
	std::uint64_t dt_last;
	int delta_time;

	Window *window;

	bool quit;

	std::unordered_map<std::type_index, Resource *> resource_map;

	static inline World *world = nullptr;

public:
	World();
	~World();

	static World *get_world()
	{
		return world;
	};

	void add_entity(Entity *ent);
	void add_entity(Entity *ent, int index);

	void add_component(Component *comp);
	void add_component(Component *comp, int index);
	void add_unique_component(Component *comp);

	void add_system(System *system);

	void add_resource(Resource *r);

	void destroy_entity(Entity *ent, bool destroy_components = true);

	template <typename C>
	void clear_entity()
	{
		auto &target_map = *reinterpret_cast<std::multimap<int, C *> *>(&entity_map[typeid(C)]);
		for (auto &[key, val] : target_map)
		{
			delete val;
		}
		target_map.clear();
		entity_map.erase(typeid(C));
	};

	void remove_entity(Entity *ent);

	void destroy_component(Component *comp, bool delete_component = true);

	inline const auto &get_entity_map() const { return entity_map; }
	inline const auto &get_component_map() const { return component_map; }
	inline const auto &get_system_list() const { return system_list; }

	template <typename C>
	bool components_exist_of_type()
	{
		return component_map.contains(typeid(C));
	}

	template <typename C>
	std::multimap<int, C *> &get_components()
	{
		return *reinterpret_cast<std::multimap<int, C *> *>(&component_map[typeid(C)]);
	}

	template <typename C>
	bool entity_exist_of_type()
	{
		return entity_map.contains(typeid(C));
	}

	template <typename C>
	std::multimap<int, C *> &get_entitys()
	{
		return *reinterpret_cast<std::multimap<int, C *> *>(&entity_map[typeid(C)]);
	}

	template <typename C>
	C &get_resource()
	{
		return *reinterpret_cast<C *>(resource_map.at(typeid(C)));
	}

	void initialize_entities();
	void poll_events();
	void tick_delta_time();

	void process_systems();

	int get_delta_time() const;
	void set_delta_time(int value);

	Window *get_window() const;
	void set_window(Window *value);

	bool is_game_quit() const;

	void clean_up();
};
