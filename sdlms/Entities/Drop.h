#pragma once

#include <string>
#include <stdint.h>
#include "entt/entt.hpp"
#include "Components/Drop.h"

void load_drops(std::vector<std::pair<std::u16string,unsigned int>> &drops, float x, float y, int layer);
