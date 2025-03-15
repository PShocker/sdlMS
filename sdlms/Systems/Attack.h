#pragma once

#include "Components/Components.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>

void attack_mob(Attack *atk, entt::entity ent);

void attack_reactor(Attack *atk);

bool attack_mob(Attack *atk, entt::entity src,
                entt::entity target,
                std::optional<SDL_FPoint> p);

bool attack_character(Attack *atk, entt::entity src,
                      entt::entity target,
                      std::optional<SDL_FPoint> p);