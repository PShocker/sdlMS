#pragma once

#include "src/client/game/game_quest.h"
#include "src/client/game/game_save.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
#include <vector>

class quest_game_instance {
private:
  static std::vector<game_quest> load_npc(const std::u16string &id);

public:
  static void update_check_mob(const std::u16string &mob_id, int num);
  static void update_check_npc(const std::u16string &npc_id);
  static void update_check_item();
  static void update_check_item(const std::u16string &quest_id);

  static void accept_quest(const std::u16string &id);

  static void accept_quest(game_quest &q);
  static void complete_quest(game_quest &q);
  static void decline_quest(game_quest &q);

  static inline std::flat_map<std::u16string, game_quest> progress_quests;
  static inline std::flat_map<std::u16string, game_quest> complete_quests;
  static inline std::flat_map<std::u16string, game_quest> decline_quests;

  static std::vector<game_quest> load_npc_quest(const std::u16string &id);
  static std::optional<game_quest>
  load_avaliable_quest(const std::u16string &id);
  static std::vector<game_quest> load_avaliable_quest();
  static std::flat_map<int8_t, std::vector<game_quest>>
  load_ui_avaliable_quest();

  static std::vector<game_quest> load_progress_quest();
  static std::flat_map<int8_t, std::vector<game_quest>>
  load_ui_progress_quest();

  static std::vector<game_quest> load_complete_quest();
  static std::flat_map<int8_t, std::vector<game_quest>>
  load_ui_complete_quest();

  static std::u16string load_area_name(int area);

  static int load_quest_index(const std::u16string &id);

  static int load_quest_progress(const std::u16string &id);

  static int load_quest_area(const std::u16string &id);

  static wz::Node *load_quest_node(const std::u16string &id);

  static void load(character_save &cs);
};