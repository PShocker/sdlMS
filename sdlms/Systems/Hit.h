#pragma once

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

#include "Components/Components.h"

void hit_hit(Attack *atk, entt::entity src,
             entt::entity target,
             std::optional<SDL_FPoint> p
             );