#include "World.h"
#include "Input.h"
#include "ECSSystem.h"

#include <algorithm>

SDL_FPoint operator+(const SDL_FPoint &m, const SDL_FPoint &n)
{
	return {m.x + n.x, m.y + n.y};
}

// 重载 SDL_Point 的减法运算符
SDL_FPoint operator-(const SDL_FPoint &m, const SDL_FPoint &n)
{
	return {m.x - n.x, m.y - n.y};
}

World::World() : dt_now{SDL_GetTicks()}, dt_last{0}, delta_time{0}, quit{false}
{
	world = this;
}
World::~World() {}

void World::add_entity(Entity *ent)
{
	auto index = entity_map[typeid(*ent)].size();
	add_entity(ent, index);
}

void World::add_entity(Entity *ent, int index)
{
	entity_map[typeid(*ent)].emplace(index, ent);
	ent->set_id(index);
}

void World::add_component(Component *comp)
{
	auto index = component_map[typeid(*comp)].size();
	add_component(comp, index);
}

void World::add_component(Component *comp, int index)
{
	component_map[typeid(*comp)].emplace(index, comp);
	comp->set_id(index);
}

void World::add_unique_component(Component *comp)
{
	for (const auto &[key, value] : component_map[typeid(*comp)])
	{
		[[unlikely]]
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

void World::destroy_entity(Entity *ent, bool destroy_components)
{
	if (destroy_components)
	{
		for (auto &[key, val] : ent->get_components())
		{
			[[likely]]
			if (val != nullptr)
			{
				destroy_component(val, true);
			}
		}
	}
	auto &target_map = entity_map[typeid(*ent)];
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
	if (target_map.size() == 0)
	{
		entity_map.erase(typeid(*ent));
	}
	delete ent;
}

void World::remove_entity(Entity *ent)
{
	auto &target_map = entity_map[typeid(*ent)];
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
}

void World::destroy_component(Component *comp, bool delete_component)
{
	if (comp != nullptr)
	{
		auto &target_map = component_map[typeid(*comp)];

		for (auto it = target_map.equal_range(comp->get_id()).first; it != target_map.equal_range(comp->get_id()).second;)
		{
			if (it->second == comp)
			{
				it = target_map.erase(it);
			}
			else
			{
				++it;
			}
		}
		if (target_map.size() == 0)
		{
			component_map.erase(typeid(*comp));
		}
		if (delete_component)
		{
			delete comp;
		}
	}
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
	delta_time = dt_now - dt_last;
}

void World::process_systems()
{
	for (System *sys : system_list)
	{
		if (sys)
			sys->run(*this);
	}
}

int World::get_delta_time() const
{
	return delta_time;
}

void World::set_delta_time(int value)
{
	delta_time = value;
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

void World::clean_up()
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
		{
			destroy_component(comp, true);
		}
	}

	for (auto &[key, val] : entity_map)
	{
		for (auto &[id, ent] : val)
		{
			destroy_entity(ent, true);
		}
	}
}
