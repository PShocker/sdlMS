#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_debuff.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
class server_debuff {
public:
  uint64_t client_id;
  game_debuff db;
};