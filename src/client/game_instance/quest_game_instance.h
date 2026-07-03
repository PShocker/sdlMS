#pragma once

#include "src/client/game/game_quest.h"
#include <optional>
#include <vector>

class quest_game_instance {
public:
  static std::vector<game_quest> load_quest();
  static std::optional<game_quest> load_npc_quest(const std::u16string &id);
};