#include "src/client/game_instance/quest_game_instance.h"

#include "src/client/game/game_quest.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/system/ui/quest_alarm_ui_system.h"
#include "src/client/system/ui/quest_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/script/script.h"
#include "src/common/wz/wz_resource.h"

#include <wz/Property.h>

#include <algorithm>
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <format>
#include <optional>
#include <string>

namespace sdlms {

namespace {

// wzlib 的三参构造在 -Werror=reorder 下会报警，这里改用 set() 构造属性。
template <typename T>
wz::Property<T>* MakeProp(wz::Type type, T value) {
  auto* prop = new wz::Property<T>(type, nullptr);
  prop->set(std::move(value));
  return prop;
}

// 本数据包没有一转任务数据：在 QuestData 里补一份虚拟任务，
// 让一转教官出现在 QuestInfo 面板（含 NPC 头像与任务说明），
// 仅对初心者（job=0）且等级达标时可见。
void EnsureJobIntroQuests() {
  static bool done = false;
  if (done) {
    return;
  }
  done = true;

  auto* data = WzResource::quest_->find(u"QuestData");
  if (data == nullptr) {
    return;
  }

  struct JobQuest {
    const char16_t* id;
    const char16_t* name;
    const char16_t* desc;
    int category;
    int level;
    int npc;
  };
  const JobQuest quests[] = {
      {u"job100", u"Warrior's Path",
       u"Dances with Balrog in Perion can grant you the power of a Warrior.",
       100, 10, 511},
      {u"job200", u"Magician's Path",
       u"Grendel in Ellinia can grant you the power of a Magician.", 101, 8,
       313},
      {u"job300", u"Bowman's Path",
       u"Athena Pierce in Henesys can grant you the power of a Bowman.", 102,
       10, 221},
      {u"job400", u"Thief's Path",
       u"The Dark Lord in Kerning City can grant you the power of a Thief.",
       103, 10, 411},
  };

  for (const auto& quest : quests) {
    if (data->find(quest.id) != nullptr) {
      continue;
    }
    auto* node = new wz::Node(wz::Type::SubProperty, nullptr);

    auto* info = new wz::Node(wz::Type::SubProperty, nullptr);
    info->appendChild(u"name", MakeProp<std::u16string>(wz::Type::String,
                                                       quest.name));
    info->appendChild(u"area",
                      MakeProp<int32_t>(wz::Type::Int, quest.category));
    info->appendChild(u"0", MakeProp<std::u16string>(wz::Type::String,
                                                    quest.desc));
    node->appendChild(u"QuestInfo", info);

    auto* check = new wz::Node(wz::Type::SubProperty, nullptr);
    auto* check0 = new wz::Node(wz::Type::SubProperty, nullptr);
    check0->appendChild(u"lvmin",
                        MakeProp<int32_t>(wz::Type::Int, quest.level));
    check0->appendChild(u"npc", MakeProp<int32_t>(wz::Type::Int, quest.npc));
    auto* job = new wz::Node(wz::Type::SubProperty, nullptr);
    job->appendChild(u"0", MakeProp<int32_t>(wz::Type::Int, 0));
    check0->appendChild(u"job", job);
    check->appendChild(u"0", check0);
    node->appendChild(u"Check", check);
    node->appendChild(u"Act", new wz::Node(wz::Type::SubProperty, nullptr));

    data->appendChild(quest.id, node);
  }
}

}  // namespace

std::vector<GameQuest> QuestGameInstance::LoadNpc(const std::u16string& id) {
  EnsureJobIntroQuests();
  std::vector<GameQuest> q;
  auto node = WzResource::quest_->find(u"QuestData");
  for (auto [k, v] : *node) {
    auto npc = node->find(k + u"/Check/0/npc");
    if (npc != nullptr) {
      auto npc_i = static_cast<wz::Property<int>*>(npc)->get();
      auto tmp = std::format("{:07d}", npc_i);
      std::u16string npc_id{tmp.begin(), tmp.end()};
      if (npc_id == id) {
        GameQuest quest{
            .quest_id = k,
        };
        q.push_back(quest);
      }
    }
  }
  return q;
}

std::vector<GameQuest> QuestGameInstance::LoadNpcQuest(
    const std::u16string& id) {
  static std::flat_map<std::u16string, std::vector<GameQuest>> cache;
  if (!cache.contains(id)) {
    cache[id] = LoadNpc(id);
  }
  return cache.at(id);
}

std::optional<GameQuest> QuestGameInstance::LoadAvaliableQuest(
    const std::u16string& id) {
  struct KeyCache {
    std::u16string id;
    int level;
    std::u16string job;

    auto operator<=>(const KeyCache&) const = default;
  };
  static std::flat_map<KeyCache, std::optional<GameQuest>> cache;
  auto& sf = CharacterGameInstance::self_;
  auto node = LoadQuestNode(id);
  KeyCache kc{id, sf.level, sf.job};
  if (!cache.contains(kc)) {
    if (auto n = node->find(u"Check/0/lvmin"); n != nullptr) {
      auto lv = static_cast<wz::Property<int>*>(n)->get();
      if (kc.level < lv) {
        cache.emplace(kc, std::nullopt);
        return std::nullopt;
      }
    }
    if (auto n = node->find(u"Check/0/job"); n != nullptr) {
      int sf_job = 0;
      if (!sf.job.empty()) {
        try {
          sf_job = std::stoi(std::string{sf.job.begin(), sf.job.end()});
        } catch (...) {
          sf_job = 0;
        }
      }
      bool r = false;
      for (auto [k, v] : *n->get_children()) {
        auto job = static_cast<wz::Property<int>*>(v[0])->get();
        r = sf_job == job;
        if (r) {
          break;
        }
      }
      if (!r) {
        cache.emplace(kc, std::nullopt);
        return std::nullopt;
      }
    }
    cache.emplace(kc, GameQuest{.quest_id = id});
  }
  auto q = cache.at(kc);
  if (!q.has_value()) {
    return std::nullopt;
  }
  // Check/0/skill（12 个任务）：要求指定技能达到某等级。
  // 技能等级会随游戏进程变化，不走上面的缓存，每次实时校验。
  if (auto n = node->find(u"Check/0/skill"); n != nullptr) {
    for (auto [k, v] : *n->get_children()) {
      if (v.empty() || v[0] == nullptr) {
        continue;
      }
      auto* id_node = v[0]->get_child(u"id");
      if (id_node == nullptr || id_node->type != wz::Type::Int) {
        continue;
      }
      auto ski_id = static_cast<wz::Property<int>*>(id_node)->get();
      int need = 1;
      if (auto* lv_node = v[0]->get_child(u"level");
          lv_node != nullptr && lv_node->type == wz::Type::Int) {
        need = static_cast<wz::Property<int>*>(lv_node)->get();
      }
      auto tmp = std::to_string(ski_id);
      std::u16string ski_str{tmp.begin(), tmp.end()};
      if (JobSkillGameInstance::LoadSkiLevel(ski_str) < need) {
        return std::nullopt;
      }
    }
  }
  struct QuestCache {
    std::u16string id;
    int state;
  };
  static std::flat_map<std::u16string, std::vector<QuestCache>> cache2;
  if (!cache2.contains(id)) {
    if (auto n = node->find(u"Check/0/quest"); n != nullptr) {
      for (auto [k, v] : *n->get_children()) {
        auto q_id =
            static_cast<wz::Property<int>*>(v[0]->get_child(u"id"))->get();
        auto state =
            static_cast<wz::Property<int>*>(v[0]->get_child(u"state"))->get();
        auto tmp = std::format("{}.img", q_id);
        std::u16string id2{tmp.begin(), tmp.end()};
        cache2[id].push_back({id2, state});
      }
    } else {
      cache2[id] = {};
    }
  }
  auto quest_check = cache2.at(id);
  // Check/0/quest 的 state 是 095 的 QuestInfo.state 语义：
  //   2 = 前置任务必须已完成（本包 215 条前置需求全部是 2）；
  //   1 = 进行中或已完成；
  //   0 = 只要记录存在。
  // 放弃记录（decline_quests_）不算完成，避免放弃前置任务后解锁后续。
  for (auto& qc : quest_check) {
    bool r = false;
    if (qc.state >= 2) {
      r = complete_quests_.contains(qc.id);
    } else if (qc.state == 1) {
      r = progress_quests_.contains(qc.id) ||
          complete_quests_.contains(qc.id);
    } else {
      r = progress_quests_.contains(qc.id) ||
          complete_quests_.contains(qc.id) ||
          decline_quests_.contains(qc.id);
    }
    if (!r) {
      return std::nullopt;
    }
  }
  if (progress_quests_.contains(id) || complete_quests_.contains(id)) {
    return std::nullopt;
  }
  auto quest = q.value();
  if (decline_quests_.contains(id)) {
    quest.index = decline_quests_[id].index;
  }
  return quest;
}

std::vector<GameQuest> QuestGameInstance::LoadAvaliableQuest() {
  EnsureJobIntroQuests();
  static auto node = WzResource::quest_->find(u"QuestData");
  std::vector<GameQuest> q;
  for (auto k : *node->get_children() | std::views::keys) {
    auto qs = LoadAvaliableQuest(k);
    if (qs.has_value()) {
      q.push_back(qs.value());
    }
  }
  return q;
}

int QuestGameInstance::LoadQuestArea(const std::u16string& id) {
  auto node = LoadQuestNode(id);
  auto ar = node->find(u"QuestInfo/area");
  auto area = static_cast<wz::Property<int>*>(ar)->get();
  return area;
}

std::flat_map<int8_t, std::vector<GameQuest>>
QuestGameInstance::LoadUiAvaliableQuest() {
  std::flat_map<int8_t, std::vector<GameQuest>> r;
  auto quests = LoadAvaliableQuest();
  std::vector<GameQuest> q;
  for (auto& quest : quests) {
    auto area = LoadQuestArea(quest.quest_id);
    r[area].push_back(quest);
  }
  return r;
}

std::vector<GameQuest> QuestGameInstance::LoadProgressQuest() {
  return progress_quests_ | std::views::values | std::ranges::to<std::vector>();
}

std::flat_map<int8_t, std::vector<GameQuest>>
QuestGameInstance::LoadUiProgressQuest() {
  std::flat_map<int8_t, std::vector<GameQuest>> r;
  auto quests = LoadProgressQuest();
  std::vector<GameQuest> q;
  for (auto& quest : quests) {
    auto area = LoadQuestArea(quest.quest_id);
    r[area].push_back(quest);
  }
  return r;
}

std::vector<GameQuest> QuestGameInstance::LoadCompleteQuest() {
  return complete_quests_ | std::views::values | std::ranges::to<std::vector>();
}

std::flat_map<int8_t, std::vector<GameQuest>>
QuestGameInstance::LoadUiCompleteQuest() {
  std::flat_map<int8_t, std::vector<GameQuest>> r;
  auto quests = LoadCompleteQuest();
  std::vector<GameQuest> q;
  for (auto& quest : quests) {
    auto area = LoadQuestArea(quest.quest_id);
    r[area].push_back(quest);
  }
  return r;
}

int QuestGameInstance::LoadQuestIndex(const std::u16string& id) {
  auto node = WzResource::quest_->find(u"QuestData/" + id + u"/Act");
  return node->children_count() - 1;
}

int QuestGameInstance::LoadQuestProgress(const std::u16string& id) {
  if (progress_quests_.contains(id)) {
    return progress_quests_[id].index;
  }
  return 0;
}

std::u16string QuestGameInstance::LoadAreaName(int area) {
  static std::flat_map<int, std::u16string> cache;
  if (!cache.contains(area)) {
    auto node = WzResource::quest_->find(u"QuestCategory.img");
    for (int i = 0;; i++) {
      auto n = node->find(std::to_string(i));
      if (n == nullptr) {
        break;
      }
      auto category =
          static_cast<wz::Property<int>*>(n->get_child(u"category"))->get();
      auto name =
          static_cast<wz::Property<std::u16string>*>(n->get_child(u"title"))
              ->get();
      cache[category] = name;
    }
  }
  return cache.at(area);
}

wz::Node* QuestGameInstance::LoadQuestNode(const std::u16string& id) {
  EnsureJobIntroQuests();
  auto node = WzResource::quest_->find(u"QuestData/" + id);
  return node;
}

void QuestGameInstance::Load(CharacterSaveData& cs) {
  progress_quests_.clear();
  complete_quests_.clear();
  decline_quests_.clear();
  QuestAlarmUiSystem::quests_.clear();
  for (const auto& qa : cs.quest_alarms) {
    std::u16string id{qa.id.begin(), qa.id.end()};
    if (!id.empty()) {
      QuestAlarmUiSystem::quests_.insert(id);
    }
  }
  for (auto& q : cs.quests) {
    switch (q.type) {
      case QuestEnum::kProgress: { 
        // 旧存档里已完成的任务可能仍标记为进行中，且对应 Check 阶段已不存在。
        bool has_step = false;
        if (auto node = LoadQuestNode(q.quest_id); node != nullptr) {
          if (auto check = node->get_child(u"Check"); check != nullptr) {
            if (check->get_child(std::to_string(q.index)) != nullptr) {
              has_step = true;
            }
          }
        }
        if (!has_step && q.index > 1) {
          q.type = QuestEnum::kComplete;
          complete_quests_[q.quest_id] = q;
          break;
        }
        AcceptQuest(q);
        // 读档后重新计算任务目标，恢复 mob_bool / item_bool。
        UpdateCheckMob(u"", 0);
        UpdateCheckItem(q.quest_id);
        break;
      }
      case QuestEnum::kComplete: {
        // 存档中的 index 已经是完成后的任务阶段，不能再次 CompleteQuest 自增。
        complete_quests_[q.quest_id] = q;
        break;
      }
      case QuestEnum::kDecline: {
        decline_quests_[q.quest_id] = q;
        break;
      }
    }
  }

  QuestUiSystem::detail_ = true;
  QuestUiSystem::active_tab_ = 0;
  QuestUiSystem::disable_fold_.clear();
  QuestUiSystem::pages_ = {};
  QuestUiSystem::detail_quest_ = u"";
  // 优先选中已保存的进行中任务，避免读档时重新扫描全部可用任务。
  if (!progress_quests_.empty()) {
    QuestUiSystem::detail_quest_ = progress_quests_.begin()->first;
  }
}

void QuestGameInstance::AcceptQuest(GameQuest& q) {
  if (q.index == 0) {
    q.index = 1;
  }
  if (decline_quests_.contains(q.quest_id)) {
    q.index = decline_quests_[q.quest_id].index;
  }
  auto node = LoadQuestNode(q.quest_id);
  if (node == nullptr) {
    progress_quests_[q.quest_id] = q;
    return;
  }
  auto check = node->get_child(u"Check");
  if (check == nullptr) {
    progress_quests_[q.quest_id] = q;
    return;
  }
  auto i = std::to_string(q.index);
  node = check->get_child(i);
  if (node == nullptr) {
    progress_quests_[q.quest_id] = q;
    return;
  }
  if (auto n = node->get_child(u"item"); n != nullptr) {
    for (auto [k, v] : *n->get_children()) {
      if (v.empty() || v[0] == nullptr) {
        continue;
      }
      auto id_node = v[0]->get_child(u"id");
      if (id_node == nullptr || id_node->type != wz::Type::Int) {
        continue;
      }
      auto id = static_cast<wz::Property<int>*>(id_node)->get();
      int count = 0;
      if (auto nn = v[0]->get_child(u"count");
          nn != nullptr && nn->type == wz::Type::Int) {
        count = static_cast<wz::Property<int>*>(nn)->get();
      }
      QuestItem qi;
      auto tmp = std::format("{:08d}", id);
      qi.id = {tmp.begin(), tmp.end()};
      qi.count = count;
      q.check_item[qi.id] = qi;
    }
  }
  if (auto n = node->get_child(u"mob"); n != nullptr) {
    for (auto [k, v] : *n->get_children()) {
      if (v.empty() || v[0] == nullptr) {
        continue;
      }
      auto id_node = v[0]->get_child(u"id");
      if (id_node == nullptr || id_node->type != wz::Type::Int) {
        continue;
      }
      auto id = static_cast<wz::Property<int>*>(id_node)->get();
      int count = 0;
      if (auto nn = v[0]->get_child(u"count");
          nn != nullptr && nn->type == wz::Type::Int) {
        count = static_cast<wz::Property<int>*>(nn)->get();
      }
      QuestMob qm;
      auto tmp = std::format("{:07d}", id);
      qm.id = {tmp.begin(), tmp.end()};
      qm.count = count;
      q.check_mob[qm.id] = qm;
    }
  }
  progress_quests_[q.quest_id] = q;
}

void QuestGameInstance::UpdateAcceptQuestTip(const std::u16string& quest_id) {
  const auto& quest = progress_quests_.at(quest_id);
  if (quest.check_item.empty() && quest.item_bool && quest.mob_bool) {
    PopupTipGameInstance::AddQuestTip(quest.quest_id);
  }
}

void QuestGameInstance::AcceptQuest(const std::u16string& id) {
  GameQuest q;
  q.quest_id = id;
  AcceptQuest(q);
  UpdateCheckMob(u"", 0);
  UpdateCheckItem(q.quest_id);
  UpdateAcceptQuestTip(q.quest_id);
}

void QuestGameInstance::CompleteQuest(GameQuest& q) {
  q.index += 1;
  q.type = QuestEnum::kComplete;
  complete_quests_[q.quest_id] = q;
  progress_quests_.erase(q.quest_id);
  AudioGameInstance::PlayNow(u"Game.img/QuestClear");
}

void QuestGameInstance::CompleteQuest(const std::u16string& id) {
  auto& q = progress_quests_.at(id);
  CompleteQuest(q);
}

void QuestGameInstance::DeclineQuest(GameQuest& q) {
  q.type = QuestEnum::kDecline;
  decline_quests_[q.quest_id] = q;
  progress_quests_.erase(q.quest_id);
  return;
}

void QuestGameInstance::DeclineQuest(const std::u16string& id) {
  auto& q = progress_quests_.at(id);
  DeclineQuest(q);
}

void QuestGameInstance::UpdateCheckItem(const std::u16string& quest_id) {
  auto& quest = progress_quests_.at(quest_id);
  bool pop_tip = false;

  for (auto [k, v] : quest.check_item) {
    int32_t back_num = -1;
    if (quest.item.contains(k)) {
      back_num = quest.item[k].count;
    }
    auto num = PackageGameInstance::LoadItemNum(k);
    if (v.count == 0) {
      // count 缺省为 0 表示“使用/消耗该物品”：
      // 持有数量从 >0 变成 0 时才算完成，而不是一开始就满足。
      if (back_num > 0 && num == 0) {
        pop_tip = true;
      }
    } else if (back_num < (int32_t)v.count && num >= v.count) {
      pop_tip = true;
    }
    quest.item[k] = {.id = v.id, .count = num};
  }
  quest.item_bool = true;
  for (auto [k, v] : quest.check_item) {
    uint32_t itm_count = 0;
    if (quest.item.contains(k)) {
      itm_count = quest.item[k].count;
    }
    if (v.count == 0 && itm_count != 0) {
      quest.item_bool = false;
      break;
    } else if (v.count > itm_count) {
      quest.item_bool = false;
      break;
    }
  }
  if (pop_tip && quest.item_bool && quest.mob_bool) {
    PopupTipGameInstance::AddQuestTip(quest.quest_id);
  }
  CheckJobTestReturn();
}

void QuestGameInstance::UpdateCheckItem() {
  for (auto [k, v] : progress_quests_) {
    UpdateCheckItem(k);
  }
  CheckJobTestReturn();
}

std::optional<QuestGameInstance::JobTestArea>
QuestGameInstance::LoadJobTestArea(const std::u16string& quest_id) {
  // 二转试炼（Test of Qualification）：每个职业一个试炼场，场里的怪物掉对应的
  // 试炼道具；场地只有一个出生传送门，进出都靠脚本，所以这里记下往返地图。
  struct Row {
    const char16_t* quest_id;
    JobTestArea area;
  };
  constexpr Row kRows[] = {
      {u"20002.img", {80001300, 10004023}},  // 战士：04031017
      {u"20102.img", {80001100, 10002070}},  // 法师：04031019
      {u"20202.img", {80001000, 10001090}},  // 弓手：04031021
      {u"20302.img", {80001200, 10003080}},  // 飞侠：04031023
  };
  for (const auto& row : kRows) {
    if (quest_id == row.quest_id) {
      return row.area;
    }
  }
  return std::nullopt;
}

void QuestGameInstance::CheckJobTestReturn() {
  for (const auto& [quest_id, quest] : progress_quests_) {
    const auto area = LoadJobTestArea(quest_id);
    if (!area.has_value()) {
      continue;
    }
    if (SceneSystemInstance::map_id_ != area->arena_map) {
      continue;
    }
    if (!quest.item_bool || !quest.mob_bool) {
      continue;
    }
    // 试炼道具已凑齐：送玩家回去向教官报告。
    SceneSystemInstance::EnterPrepare(area->instructor_map, u"sp", 0);
    return;
  }
}

bool QuestGameInstance::EnterJobTestArena(const std::u16string& quest_id) {
  const auto area = LoadJobTestArea(quest_id);
  if (!area.has_value()) {
    return false;
  }
  auto it = progress_quests_.find(quest_id);
  if (it == progress_quests_.end()) {
    return false;
  }
  // 条件已达成的交任务流程不在这里处理。
  if (it->second.item_bool && it->second.mob_bool) {
    return false;
  }
  SceneSystemInstance::EnterPrepare(area->arena_map, u"sp", 0);
  return true;
}

bool QuestGameInstance::RunJobTestFailScript() {
  for (const auto& [quest_id, quest] : progress_quests_) {
    const auto area = LoadJobTestArea(quest_id);
    if (!area.has_value()) {
      continue;
    }
    if (SceneSystemInstance::map_id_ != area->arena_map) {
      continue;
    }
    auto* node = LoadQuestNode(quest_id);
    if (node == nullptr) {
      return false;
    }
    auto* n = node->find(u"Check/1/failscript");
    if (n == nullptr || n->type != wz::Type::String) {
      return false;
    }
    const auto name = static_cast<wz::Property<std::u16string>*>(n)->get();
    auto& fns = Script::fns();
    auto it = fns.find(name);
    if (it == fns.end()) {
      return false;
    }
    it->second(nullptr);
    return true;
  }
  return false;
}

void QuestGameInstance::UpdateCheckMob(const std::u16string& mob_id, int num) {
  for (auto&& [k, quest] : progress_quests_) {
    bool pop_tip = false;
    if (quest.check_mob.contains(mob_id)) {
      if (quest.mob[mob_id].count < quest.check_mob[mob_id].count) {
        quest.mob[mob_id].count += num;
        pop_tip = true;
      }
    }
    quest.mob_bool = true;
    for (auto [m_id, v] : quest.check_mob) {
      uint32_t mob_count = 0;
      if (quest.mob.contains(m_id)) {
        mob_count = quest.mob[m_id].count;
      }
      if (mob_count < v.count) {
        quest.mob_bool = false;
        break;
      }
    }
    if (pop_tip && quest.item_bool && quest.mob_bool) {
      PopupTipGameInstance::AddQuestTip(quest.quest_id);
    }
  }
}

std::flat_map<std::u16string, int> QuestGameInstance::LoadQuestActItem(
    const std::u16string& id) {
  std::flat_map<std::u16string, int> r;
  uint8_t index = 0;
  if (progress_quests_.contains(id)) {
    index = progress_quests_.at(id).index;
  }
  auto quest_node = LoadQuestNode(id);
  quest_node = quest_node->find("Act/" + std::to_string(index) + "/item");
  if (quest_node != nullptr) {
    for (auto [k, v] : *quest_node->get_children()) {
      if (v[0]->get_child(u"prop")) {
        continue;
      }
      auto id = static_cast<wz::Property<int>*>(v[0]->get_child(u"id"))->get();
      auto count =
          static_cast<wz::Property<int>*>(v[0]->get_child(u"count"))->get();
      auto tmp = std::format("{:08d}", id);
      std::u16string id2{tmp.begin(), tmp.end()};
      r[id2] = count;
    }
  }
  return r;
}

std::flat_map<std::u16string, int> QuestGameInstance::LoadQuestStartItem(
    const std::u16string& id) {
  std::flat_map<std::u16string, int> r;
  auto quest_node = LoadQuestNode(id);
  if (quest_node == nullptr) {
    return r;
  }
  auto check = quest_node->find(u"Check/1/item");
  auto act = quest_node->find(u"Act/0/item");
  if (check == nullptr || act == nullptr) {
    return r;
  }

  std::flat_set<std::u16string> required;
  for (auto [k, v] : *check->get_children()) {
    auto item_id =
        static_cast<wz::Property<int>*>(v[0]->get_child(u"id"))->get();
    auto tmp = std::format("{:08d}", item_id);
    required.emplace(tmp.begin(), tmp.end());
  }
  for (auto [k, v] : *act->get_children()) {
    auto item_id =
        static_cast<wz::Property<int>*>(v[0]->get_child(u"id"))->get();
    auto tmp = std::format("{:08d}", item_id);
    std::u16string item_str{tmp.begin(), tmp.end()};
    if (!required.contains(item_str)) {
      continue;
    }
    int count = 1;
    if (auto n = v[0]->get_child(u"count"); n != nullptr) {
      count = static_cast<wz::Property<int>*>(n)->get();
    }
    if (count > 0) {
      r[item_str] = count;
    }
  }
  return r;
}

std::flat_map<std::u16string, int> QuestGameInstance::LoadQuestActRandomItem(
    const std::u16string& id) {
  std::flat_map<std::u16string, int> r;
  uint8_t index = 0;
  if (progress_quests_.contains(id)) {
    index = progress_quests_.at(id).index;
  }
  auto quest_node = LoadQuestNode(id);
  quest_node = quest_node->find("Act/" + std::to_string(index) + "/item");
  if (quest_node != nullptr) {
    for (auto [k, v] : *quest_node->get_children()) {
      if (!v[0]->get_child(u"prop")) {
        continue;
      }
      auto id = static_cast<wz::Property<int>*>(v[0]->get_child(u"id"))->get();
      auto count =
          static_cast<wz::Property<int>*>(v[0]->get_child(u"count"))->get();
      auto tmp = std::format("{:08d}", id);
      std::u16string id2{tmp.begin(), tmp.end()};
      r[id2] = count;
    }
  }
  return r;
}

uint32_t QuestGameInstance::LoadQuestActExp(const std::u16string& id) {
  if (progress_quests_.contains(id)) {
    auto quest_node = LoadQuestNode(id);
    auto index = progress_quests_.at(id).index;
    quest_node = quest_node->find("Act/" + std::to_string(index) + "/exp");
    if (quest_node != nullptr) {
      return static_cast<wz::Property<int>*>(quest_node)->get();
    }
  }
  return 0;
}

int32_t QuestGameInstance::LoadQuestActMeso(const std::u16string& id) {
  if (progress_quests_.contains(id)) {
    auto quest_node = LoadQuestNode(id);
    auto index = progress_quests_.at(id).index;
    quest_node = quest_node->find("Act/" + std::to_string(index) + "/money");
    if (quest_node != nullptr) {
      return static_cast<wz::Property<int>*>(quest_node)->get();
    }
  }
  return 0;
}

// Act/<idx>/pop：095 CQuest::LoadReward 的人气奖励。
int32_t QuestGameInstance::LoadQuestActPop(const std::u16string& id) {
  uint8_t index = 0;
  if (progress_quests_.contains(id)) {
    index = progress_quests_.at(id).index;
  }
  auto* node = LoadQuestNode(id);
  if (node == nullptr) {
    return 0;
  }
  auto* pop = node->find("Act/" + std::to_string(index) + "/pop");
  if (pop == nullptr || pop->type != wz::Type::Int) {
    return 0;
  }
  return static_cast<wz::Property<int>*>(pop)->get();
}

// Act/<idx>/skill：授予技能。数据形如 skill/0/{id, exp}，
// exp 为授予的等级（095 CQuest::LoadReward 的 learnSkill）。
std::flat_map<uint32_t, uint8_t> QuestGameInstance::LoadQuestActSkill(
    const std::u16string& id) {
  std::flat_map<uint32_t, uint8_t> r;
  uint8_t index = 0;
  if (progress_quests_.contains(id)) {
    index = progress_quests_.at(id).index;
  }
  auto* node = LoadQuestNode(id);
  if (node == nullptr) {
    return r;
  }
  auto* skills = node->find("Act/" + std::to_string(index) + "/skill");
  if (skills == nullptr) {
    return r;
  }
  for (auto [k, v] : *skills->get_children()) {
    if (v.empty() || v[0] == nullptr) {
      continue;
    }
    auto* id_node = v[0]->get_child(u"id");
    if (id_node == nullptr || id_node->type != wz::Type::Int) {
      continue;
    }
    auto ski_id = static_cast<wz::Property<int>*>(id_node)->get();
    int level = 1;
    if (auto* lv = v[0]->get_child(u"exp");
        lv != nullptr && lv->type == wz::Type::Int) {
      level = static_cast<wz::Property<int>*>(lv)->get();
    }
    r[static_cast<uint32_t>(ski_id)] =
        static_cast<uint8_t>(std::clamp(level, 0, 255));
  }
  return r;
}

// 095 CQuestMan::CheckCompleteDemand 的客户端等价实现：
// 交任务时按 level → item → mob 的顺序返回第一个不满足的原因。
// 物品"完全丢失"（全部持有量为 0）时由调用方优先改用 Say/<idx>/lost 对白。
std::u16string QuestGameInstance::CheckCompleteDemand(
    const std::u16string& id) {
  auto it = progress_quests_.find(id);
  if (it == progress_quests_.end()) {
    return u"";
  }
  const auto& quest = it->second;
  auto* node = LoadQuestNode(id);
  if (node == nullptr) {
    return u"";
  }
  auto* check = node->find("Check/" + std::to_string(quest.index));
  if (check == nullptr) {
    return u"";
  }
  // 等级：Check/1/lvmin，数据中 89 个任务使用，对应 Say/<idx>/stop/level。
  if (auto* n = check->get_child(u"lvmin");
      n != nullptr && n->type == wz::Type::Int) {
    auto lv = static_cast<wz::Property<int>*>(n)->get();
    if (CharacterGameInstance::self_.level < lv) {
      return u"level";
    }
  }
  if (!quest.item_bool) {
    return u"item";
  }
  if (!quest.mob_bool) {
    return u"mob";
  }
  return u"";
}

}  // namespace sdlms
