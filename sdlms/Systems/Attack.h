#pragma once

#include "Components/Components.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>

void attack_run();

void attacking(AttackWarp *atk);

bool mob_attack();

bool trap_attack();
