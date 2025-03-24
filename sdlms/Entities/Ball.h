#pragma once

#include "Components/Components.h"
#include <vector>
#include "entt/entt.hpp"

entt::entity load_ball(entt::entity owner, SDL_FPoint point = SDL_FPoint{0, -30}, int speed = 550, Skill *ski = nullptr);

extern const std::unordered_set<std::u16string> track_no_skill;
