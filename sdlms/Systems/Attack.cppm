module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>

export module systems:attack;

import components;
import core;

export void attack_run();

void attacking(AttackWarp *atk);

bool mob_attack();

bool trap_attack();
