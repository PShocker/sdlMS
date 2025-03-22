#pragma once

#include "wz/Property.hpp"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <vector>

entt::entity load_chatballoon(std::vector<std::u16string> strs, SDL_Color color, entt::entity owner);
