#pragma once

#include "src/client/game/game_quest.h"
#include "src/client/game/game_save.h"
#include <flat_map>
#include <optional>
#include <string>
#include <vector>

class quest_game_instance {
private:
  static std::vector<game_quest> load_npc(const std::u16string &id);

public:
  static inline std::vector<game_quest> quests;

  static std::vector<game_quest> load_npc_quest(const std::u16string &id);
  static std::vector<game_quest> load_avaliable_quest();
  static std::vector<game_quest> load_avaliable_quest(int area);
  static std::vector<game_quest> load_progress_quest();
  static std::vector<game_quest> load_progress_quest(int area);
  static std::vector<game_quest> load_complete_quest();
  static std::vector<game_quest> load_complete_quest(int area);

  static int load_quest_index(const std::u16string &id);

  static void load(const character_save &cs);
};