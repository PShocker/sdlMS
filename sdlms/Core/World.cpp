#include "World.h"
#include "Input.h"
#include "ECSSystem.h"
#include <SDL2/SDL_timer.h>

#include <algorithm>

unsigned long World::EntityCounter = 0;

World::World() : dt_now{SDL_GetTicks()}, dt_last{0}, delta_time_{0}, quit{false} {}
World::~World() {}

void World::add_entity(Entity *ent)
{
	entity_list.push_back(ent);
	ent->set_id(EntityCounter++);

	DEBUG_PRINT("Added a new entity (ID %lu)", EntityCounter - 1);
}

void World::add_component(Component *comp)
{
	// component_map[typeid(*comp)][component_map[typeid(*comp)].size()] = comp;
	component_map[typeid(*comp)].insert({component_map[typeid(*comp)].size(), comp});

	DEBUG_PRINT("Added a new component (%s)", typeid(*comp).name());
}

void World::add_component(Component *comp, int index)
{

	// component_map[typeid(*comp)][index] = comp;
	component_map[typeid(*comp)].insert({index, comp});

	DEBUG_PRINT("Added a new component (%s)", typeid(*comp).name());
}

void World::add_system(System *sys)
{
	system_list.push_back(sys);

	DEBUG_PRINT("Added a new system (%s)", typeid(*sys).name());
}

void World::add_resource(std::type_index type, Resource *r)
{
	resource_map[type] = r;

	DEBUG_PRINT("Added a new system (%s)", typeid(*sys).name());
}

void World::destroy_entity(Entity *ent, bool destroy_components, bool remove_from_list)
{
	DEBUG_PRINT("Destroying entity ID %lu", ent->get_id());

	if (destroy_components)
	{
		for (auto &pair : ent->get_components())
			destroy_component(pair.second);
	}

	if (remove_from_list)
		entity_list.erase(std::remove(entity_list.begin(), entity_list.end(), ent));

	delete ent;
}

void World::destroy_component(Component *comp, bool remove_from_map)
{
	auto &target_map = component_map[typeid(*comp)];

	for (auto it = target_map.begin(); it != target_map.end();)
	{
		if (it->second == comp)
		{
			it = target_map.erase(it); // 删除匹配值的元素，并返回指向下一个元素的迭代器
		}
		else
		{
			++it;
		}
	}
	delete comp;
}

void World::poll_events()
{
	Input::update_momentary_keys();

	while (SDL_PollEvent(&event_handler) != 0)
	{
		switch (event_handler.type)
		{
		case SDL_EventType::SDL_QUIT:
		{
			quit = true;
		}
		break;

		case SDL_EventType::SDL_KEYDOWN:
		{
			Input::process_input_event(Input::EventType::KeyPressed, event_handler);
		}
		break;

		case SDL_EventType::SDL_KEYUP:
		{
			Input::process_input_event(Input::EventType::KeyReleased, event_handler);
		}
		break;

		case SDL_EventType::SDL_MOUSEBUTTONDOWN:
		{
			Input::process_input_event(Input::EventType::MousePressed, event_handler);
		}
		break;

		case SDL_EventType::SDL_MOUSEBUTTONUP:
		{
			Input::process_input_event(Input::EventType::MouseReleased, event_handler);
		}
		break;
		}
	}
}

void World::tick_delta_time()
{
	dt_last = dt_now;
	dt_now = SDL_GetTicks();
	delta_time_ = dt_now - dt_last;
}

void World::process_systems()
{
	for (System *sys : system_list)
	{
		if (sys)
			sys->run(*this);
	}
}

int World::delta_time() const
{
	return delta_time_;
}

Window *World::get_window() const
{
	return window;
}

void World::set_window(Window *value)
{
	window = value;
}

bool World::is_game_quit() const
{
	return quit;
}

void World::cleanup()
{
#ifdef ECS_DEBUG
	size_t ccount = 0;
	for (auto &pair : component_map)
		ccount += pair.second.size();

	ECS_PRINT("DESTROYING EVERYTHING (%lu entities and %lu components)", entity_list.size(), ccount);
#endif

	for (auto &pair : component_map)
	{
		for (auto &[index, comp] : pair.second)
			destroy_component(comp, false);
	}

	for (Entity *ent : entity_list)
		destroy_entity(ent, false, false);
}
