#pragma once
#include "SDL3/SDL_audio.h"
#include "src/client/game/game_equip.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_save.h"
#include <array>
#include <cstdint>
#include <optional>
class package_game_instance {
public:
  static inline std::vector<std::optional<game_equip>> equips;
  static inline std::vector<std::optional<game_item>> cosumes;
  static inline std::vector<std::optional<game_item>> etc;
  static inline std::vector<std::optional<game_item>> install;
  static inline std::vector<std::optional<game_item>> cash;

  static inline uint64_t meso;

  static void load(const character_save &cs);
};