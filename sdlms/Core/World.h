#pragma once

#include "Entities/Entity.h"
#include "Components/Component.h"
#include "Systems/System.h"
#include "Resource/Resource.h"

#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <cstdint>

#include "Core/Window.h"
#include <vector>
#include <SDL2/SDL.h>

class World
{
private:
	static unsigned long EntityCounter;

	struct ComponentStore
	{
		std::type_index type;
		Component *component;
	};

	std::vector<Entity *> entity_list;
	std::unordered_map<std::type_index, std::vector<Component *>> component_map;
	std::vector<System *> system_list;

	SDL_Event event_handler;

	std::uint64_t dt_now;
	std::uint64_t dt_last;
	double delta_time_;

	Window *window;

	bool quit;

	std::unordered_map<std::type_index, Resource *> resource_map;

public:
	World();
	~World();

	void add_entity(Entity *ent);

	void add_component(Component *comp);

	void add_system(System *system);

	void add_resource(std::type_index type, Resource *r);

	void destroy_entity(Entity *ent, bool destroy_components = true, bool remove_from_list = true);

	void destroy_component(Component *comp, bool remove_from_map = true);

	inline const auto &get_entity_list() const { return entity_list; }
	inline const auto &get_component_map() const { return component_map; }
	inline const auto &get_system_list() const { return system_list; }

	template <typename C>
	bool components_exist_of_type()
	{
		return component_map.find(typeid(C)) != component_map.end();
	}

	template <typename C>
	const std::vector<C *> &get_components()
	{
		return *reinterpret_cast<std::vector<C *> *>(&component_map.at(typeid(C)));
	}

	template <typename C>
	const C &get_resource()
	{
		return *reinterpret_cast<C *>(resource_map.at(typeid(C)));
	}

	void initialize_entities();
	void poll_events();
	void tick_delta_time();

	void process_systems();

	double delta_time() const;

	Window *get_window() const;
	void set_window(Window *value);

	bool is_game_quit() const;

	void cleanup();
};
