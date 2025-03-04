#pragma once

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

#include "Components/Components.h"

void hit_effect(Attack *atk, std::optional<SDL_FPoint> head,
                entt::entity src, entt::entity target,
                char type, std::optional<SDL_FPoint> p);

void hit_effect(Attack *atk, entt::entity src, entt::entity target,
                char type, std::optional<SDL_FPoint> p);
