#pragma once

#include "src/client/game/game_quest.h"
#include "src/client/game/game_save.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
#include <vector>
namespace sdlms {


class QuestGameInstance {
private:
  static std::vector<GameQuest> LoadNpc(const std::u16string &id);

public:
  static void UpdateCheckMob(const std::u16string &mob_id, int num);
  static void UpdateCheckItem();
  static void UpdateCheckItem(const std::u16string &quest_id);
  static void UpdateAcceptQuestTip(const std::u16string &quest_id);

  static void AcceptQuest(GameQuest &q);
  static void CompleteQuest(GameQuest &q);
  static void DeclineQuest(GameQuest &q);

  static void AcceptQuest(const std::u16string &quest_id);
  static void CompleteQuest(const std::u16string &quest_id);
  static void DeclineQuest(const std::u16string &quest_id);

  static inline std::flat_map<std::u16string, GameQuest> progress_quests_;
  static inline std::flat_map<std::u16string, GameQuest> complete_quests_;
  static inline std::flat_map<std::u16string, GameQuest> decline_quests_;

  static std::vector<GameQuest> LoadNpcQuest(const std::u16string &id);
  static std::optional<GameQuest>
  LoadAvaliableQuest(const std::u16string &id);
  static std::vector<GameQuest> LoadAvaliableQuest();
  static std::flat_map<int8_t, std::vector<GameQuest>>
  LoadUiAvaliableQuest();

  static std::vector<GameQuest> LoadProgressQuest();
  static std::flat_map<int8_t, std::vector<GameQuest>>
  LoadUiProgressQuest();

  static std::vector<GameQuest> LoadCompleteQuest();
  static std::flat_map<int8_t, std::vector<GameQuest>>
  LoadUiCompleteQuest();

  static std::u16string LoadAreaName(int area);

  static int LoadQuestIndex(const std::u16string &id);

  static int LoadQuestProgress(const std::u16string &id);

  static int LoadQuestArea(const std::u16string &id);

  static wz::Node *LoadQuestNode(const std::u16string &id);

  static std::flat_map<std::u16string, int>
  LoadQuestActItem(const std::u16string &id);
  static std::flat_map<std::u16string, int>
  LoadQuestActRandomItem(const std::u16string &id);
  static uint32_t LoadQuestActExp(const std::u16string &id);
  static int32_t LoadQuestActMeso(const std::u16string &id);

  static void Load(CharacterSaveData &cs);
};

}  // namespace sdlms
