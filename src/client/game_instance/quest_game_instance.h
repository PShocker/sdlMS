#pragma once

#include "src/client/game/game_quest.h"
#include "src/client/game/game_save.h"

#include <wz/Node.h>

#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

class QuestGameInstance {
 private:
  static std::vector<GameQuest> LoadNpc(const std::u16string& id);

  // 二转试炼：凑齐试炼道具后自动把玩家送回教官所在的城镇。
  static void CheckJobTestReturn();

 public:
  // 二转试炼场信息：200X2(Test of Qualification) 对应的试炼场地图，
  // 以及发任务的教官所在的地图。
  struct JobTestArea {
    uint32_t arena_map;
    uint32_t instructor_map;
  };
  static std::optional<JobTestArea> LoadJobTestArea(
      const std::u16string& quest_id);

  // 二转试炼失败（在试炼场里死亡）：运行该任务的 Check/1/failscript。
  // 当前地图不是试炼场、或没有进行中的试炼时返回 false。
  static bool RunJobTestFailScript();

  // 二转试炼进行中且还没达标时，把玩家送回试炼场（重新进场）。
  // 返回 true 表示已处理（调用方不要再显示"任务未完成"对白）。
  static bool EnterJobTestArena(const std::u16string& quest_id);

  static void UpdateCheckMob(const std::u16string& mob_id, int num);
  static void UpdateCheckItem();
  static void UpdateCheckItem(const std::u16string& quest_id);
  static void UpdateAcceptQuestTip(const std::u16string& quest_id);

  static void AcceptQuest(GameQuest& q);
  static void CompleteQuest(GameQuest& q);
  static void DeclineQuest(GameQuest& q);

  static void AcceptQuest(const std::u16string& quest_id);
  static void CompleteQuest(const std::u16string& quest_id);
  static void DeclineQuest(const std::u16string& quest_id);

  static inline std::flat_map<std::u16string, GameQuest> progress_quests_;
  static inline std::flat_map<std::u16string, GameQuest> complete_quests_;
  static inline std::flat_map<std::u16string, GameQuest> decline_quests_;

  static std::vector<GameQuest> LoadNpcQuest(const std::u16string& id);
  static std::optional<GameQuest> LoadAvaliableQuest(const std::u16string& id);
  static std::vector<GameQuest> LoadAvaliableQuest();
  static std::flat_map<int8_t, std::vector<GameQuest>> LoadUiAvaliableQuest();

  static std::vector<GameQuest> LoadProgressQuest();
  static std::flat_map<int8_t, std::vector<GameQuest>> LoadUiProgressQuest();

  static std::vector<GameQuest> LoadCompleteQuest();
  static std::flat_map<int8_t, std::vector<GameQuest>> LoadUiCompleteQuest();

  static std::u16string LoadAreaName(int area);

  static int LoadQuestIndex(const std::u16string& id);

  static int LoadQuestProgress(const std::u16string& id);

  static int LoadQuestArea(const std::u16string& id);

  static wz::Node* LoadQuestNode(const std::u16string& id);

  static std::flat_map<std::u16string, int> LoadQuestActItem(
      const std::u16string& id);
  // Act/0 中与 Check/1 需求对应的起始任务道具（如 1001 的镜子）
  static std::flat_map<std::u16string, int> LoadQuestStartItem(
      const std::u16string& id);
  static std::flat_map<std::u16string, int> LoadQuestActRandomItem(
      const std::u16string& id);
  static uint32_t LoadQuestActExp(const std::u16string& id);
  static int32_t LoadQuestActMeso(const std::u16string& id);
  // Act/<idx>/pop：人气值奖励。
  static int32_t LoadQuestActPop(const std::u16string& id);
  // Act/<idx>/skill：授予技能（key = 技能 id，value = 等级）。
  static std::flat_map<uint32_t, uint8_t> LoadQuestActSkill(
      const std::u16string& id);

  // 交任务需求校验，对应 095 CQuestMan::CheckCompleteDemand 的失败原因。
  // 返回 "level" / "item" / "mob"，空串表示全部满足。
  // 依据：Check/1/lvmin(89) / item(215) / mob(63)。
  static std::u16string CheckCompleteDemand(const std::u16string& id);

  static void Load(CharacterSaveData& cs);
};

}  // namespace sdlms
