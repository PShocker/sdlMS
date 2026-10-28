#pragma once

#include "src/client/game/game_quest.h"
#include "src/client/game/game_save.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
#include <vector>

class QuestGameInstance {
private:
  static std::vector<GameQuest> load_npc(const std::u16string &id);

public:
  static void update_check_mob(const std::u16string &mob_id, int num);
  static void update_check_item();
  static void update_check_item(const std::u16string &quest_id);
  static void update_accept_quest_tip(const std::u16string &quest_id);

  static void accept_quest(GameQuest &q);
  static void complete_quest(GameQuest &q);
  static void decline_quest(GameQuest &q);

  static void accept_quest(const std::u16string &quest_id);
  static void complete_quest(const std::u16string &quest_id);
  static void decline_quest(const std::u16string &quest_id);

  static inline std::flat_map<std::u16string, GameQuest> progress_quests;
  static inline std::flat_map<std::u16string, GameQuest> complete_quests;
  static inline std::flat_map<std::u16string, GameQuest> decline_quests;

  static std::vector<GameQuest> load_npc_quest(const std::u16string &id);
  static std::optional<GameQuest>
  load_avaliable_quest(const std::u16string &id);
  static std::vector<GameQuest> load_avaliable_quest();
  static std::flat_map<int8_t, std::vector<GameQuest>>
  load_ui_avaliable_quest();

  static std::vector<GameQuest> load_progress_quest();
  static std::flat_map<int8_t, std::vector<GameQuest>>
  load_ui_progress_quest();

  static std::vector<GameQuest> load_complete_quest();
  static std::flat_map<int8_t, std::vector<GameQuest>>
  load_ui_complete_quest();

  static std::u16string load_area_name(int area);

  static int load_quest_index(const std::u16string &id);

  static int load_quest_progress(const std::u16string &id);

  static int load_quest_area(const std::u16string &id);

  static wz::Node *load_quest_node(const std::u16string &id);

  static std::flat_map<std::u16string, int>
  load_quest_act_item(const std::u16string &id);
  static std::flat_map<std::u16string, int>
  load_quest_act_random_item(const std::u16string &id);
  static uint32_t load_quest_act_exp(const std::u16string &id);
  static int32_t load_quest_act_meso(const std::u16string &id);

  static void load(CharacterSaveData &cs);
};