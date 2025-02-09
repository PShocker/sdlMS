#pragma once

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

#include "Components/Components.h"

void hit_effect(AttackWarp *atkw, std::optional<SDL_FPoint> head,
                entt::entity ent, char type, SDL_FPoint *p);

void hit_effect(AttackWarp *atkw, entt::entity ent, char type, SDL_FPoint *p);
