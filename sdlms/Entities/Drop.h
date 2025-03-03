#pragma once

#include <string>
#include <stdint.h>
#include "entt/entt.hpp"
#include "Components/Drop.h"

void load_drops(std::vector<Drop::Info> *drops, float x, float y, int layer);
