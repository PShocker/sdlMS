#pragma once
#include "SDL3/SDL_audio.h"
#include "src/client/game/game_equip.h"
#include "src/client/game/game_item.h"
#include <array>
#include <optional>
class package_game_instance {
public:
  static inline std::array<std::optional<game_equip>, 64> equips;
  static inline std::array<std::optional<game_item>, 64> cosumes;
  static inline std::array<std::optional<game_item>, 64> etc;
  static inline std::array<std::optional<game_item>, 64> install;
  static inline std::array<std::optional<game_item>, 64> cash;
};