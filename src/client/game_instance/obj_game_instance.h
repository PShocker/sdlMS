#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_obj.h"
#include "wz/Node.h"
#include <array>
#include <cstdint>
#include <flat_map>
#include <vector>
class ObjGameInstance {
public:
  static void Load(wz::Node *image);
  static void Load(uint32_t map_id);
  static inline std::array<std::flat_multimap<int64_t, GameObj>, 8> data_;
};