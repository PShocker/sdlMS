#include "World.h"
#include "Input.h"
#include "ECSSystem.h"
#include <SDL2/SDL_timer.h>

#include <algorithm>

SDL_FPoint operator+(const SDL_FPoint &a, const SDL_FPoint &b)
{
	return {a.x + b.x, a.y + b.y};
}

// 重载 SDL_Point 的减法运算符
SDL_FPoint operator-(const SDL_FPoint &a, const SDL_FPoint &b)
{
	return {a.x - b.x, a.y - b.y};
}

unsigned long World::EntityCounter = 0;

World::World() : dt_now{SDL_GetTicks()}, dt_last{0}, delta_time_{0}, quit{false} {}
World::~World() {}

void World::add_entity(Entity *ent, int index)
{
	entity_map[typeid(*ent)].insert({index, ent});
	auto s = typeid(*ent).name();
	ent->set_id(EntityCounter++);

	DEBUG_PRINT("Added a new entity (ID %lu)", EntityCounter - 1);
}

void World::add_component(Component *comp)
{
	component_map[typeid(*comp)].insert({component_map[typeid(*comp)].size(), comp});
	auto s = typeid(*comp).name();

	DEBUG_PRINT("Added a new component (%s)", typeid(*comp).name());
}

void World::add_component(Component *comp, int index)
{
	component_map[typeid(*comp)].insert({index, comp});

	DEBUG_PRINT("Added a new component (%s)", typeid(*comp).name());
}

void World::add_unique_component(Component *comp)
{
	for (const auto &[key, value] : component_map[typeid(*comp)])
	{
		if (value == comp)
		{
			// 避免重复添加到world
			return;
		}
	}
	add_component(comp);
}

void World::add_system(System *sys)
{
	system_list.push_back(sys);

	DEBUG_PRINT("Added a new system (%s)", typeid(*sys).name());
}

void World::add_resource(Resource *r)
{
	resource_map[typeid(*r)] = r;

	DEBUG_PRINT("Added a new system (%s)", typeid(*sys).name());
}

void World::destroy_entity(Entity *ent, bool destroy_components, bool remove)
{
	DEBUG_PRINT("Destroying entity ID %lu", ent->get_id());

	if (destroy_components)
	{
		for (auto &pair : ent->get_components())
			destroy_component(pair.second);
	}

	if (remove)
		entity_map.erase(typeid(*ent));

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

	// for (Entity *ent : entity_map)
	// 	destroy_entity(ent, false, false);
	for (auto &[key, val] : entity_map)
	{
		for (auto &[_, ent] : val)
		{
			destroy_entity(ent, false, false);
		}
	}
}
