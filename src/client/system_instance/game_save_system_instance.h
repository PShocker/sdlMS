#pragma once
#include "src/client/game/game_save.h"
#include <optional>
#include <string>

class game_save_system_instance {
public:
  static inline game_save save;
  static bool load_save(const std::string &login);

  static bool save_game();
};