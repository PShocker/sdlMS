#pragma once

#include "Components/Components.h"
#include <vector>
#include "entt/entt.hpp"

entt::entity load_ball(entt::entity owner, SDL_FPoint point, int speed, Skill *ski = nullptr);

extern const std::unordered_set<int> track_no_skill;
