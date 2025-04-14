#pragma once

#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Entities/Entities.h"
#include "Commons/Commons.h"

// 独立函数：寻找最近的可攻击怪物
entt::entity find_closest_attackable_mob(
    int flip,
    const SDL_FPoint &origin,
    const std::unordered_set<entt::entity> &hit_entities,
    float max_x_distance,
    float max_y_distance);

// 独立函数：寻找最近的可攻击怪物
entt::entity find_closest_attackable_mob(
    Transform &origin,
    const Triangle &tri);

void mob_special_effect(entt::entity ent, int id, AnimatedSprite::Wrap *asprw, unsigned int time);

std::pair<bool, bool> dizzy_call_back(entt::entity ent, std::any &data);

std::pair<bool, bool> poison_call_back(entt::entity ent, std::any &data);

std::pair<bool, bool> flame_call_back(entt::entity ent, std::any &data);

std::pair<bool, bool> frozen_call_back(entt::entity ent, std::any &data);
