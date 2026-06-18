#pragma once
#include "src/client/game/game_save.h"
#include <string>

class game_save_system_instance {
public:
  static game_save load_save(const std::string &login);
};