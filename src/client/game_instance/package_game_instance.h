#pragma once
#include "SDL3/SDL_audio.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_save.h"
#include <array>
#include <cstdint>
#include <optional>
class package_game_instance {
public:
  // enum class package_enum { equip, consume, etc, install, cash };
  static inline std::array<std::vector<std::optional<game_item>>, 5> data;

  static inline uint64_t meso;

  static void load(const character_save &cs);
};