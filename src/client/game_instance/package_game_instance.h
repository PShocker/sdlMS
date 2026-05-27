#pragma once
#include "SDL3/SDL_audio.h"
#include "src/client/game/game_equip.h"
#include "src/client/game/game_item.h"
#include <array>
#include <optional>
class package_game_instance {
public:
  static inline std::array<std::optional<game_equip>, 96> equips;
  static inline std::array<std::optional<game_item>, 96> cosumes;
  static inline std::array<std::optional<game_item>, 96> etc;
  static inline std::array<std::optional<game_item>, 96> install;
  static inline std::array<std::optional<game_item>, 96> cash;

  static void load();
};