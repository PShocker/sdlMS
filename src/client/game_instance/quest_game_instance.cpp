#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/game/game_quest.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/ui/quest_ui_system.h"
#include "src/common/wz/wz_resource.h"
#include <wz/Property.h>
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <format>
#include <optional>
#include <string>
namespace sdlms {


std::vector<GameQuest>
QuestGameInstance::LoadNpc(const std::u16string &id) {
  std::vector<GameQuest> q;
  auto node = WzResource::quest_->find(u"QuestData");
  for (auto [k, v] : *node) {
    auto npc = node->find(k + u"/Check/0/npc");
    if (npc != nullptr) {
      auto npc_i = static_cast<wz::Property<int> *>(npc)->get();
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

std::vector<GameQuest>
QuestGameInstance::LoadNpcQuest(const std::u16string &id) {
  static std::flat_map<std::u16string, std::vector<GameQuest>> cache;
  if (!cache.contains(id)) {
    cache[id] = LoadNpc(id);
  }
  return cache.at(id);
}

std::optional<GameQuest>
QuestGameInstance::LoadAvaliableQuest(const std::u16string &id) {
  struct KeyCache {
    std::u16string id;
    int level;
    std::u16string job;

    auto operator<=>(const KeyCache &) const = default;
  };
  static std::flat_map<KeyCache, std::optional<GameQuest>> cache;
  auto &sf = CharacterGameInstance::self_;
  auto node = LoadQuestNode(id);
  KeyCache kc{id, sf.level, sf.job};
  if (!cache.contains(kc)) {
    if (auto n = node->find(u"Check/0/lvmin"); n != nullptr) {
      auto lv = static_cast<wz::Property<int> *>(n)->get();
      if (kc.level < lv) {
        cache.emplace(kc, std::nullopt);
        return std::nullopt;
      }
    }
    if (auto n = node->find(u"Check/0/job"); n != nullptr) {
      auto sf_job = std::stoi(std::string{sf.job.begin(), sf.job.end()});
      bool r = false;
      for (auto [k, v] : *n->get_children()) {
        auto job = static_cast<wz::Property<int> *>(v[0])->get();
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
  struct QuestCache {
    std::u16string id;
    int state;
  };
  static std::flat_map<std::u16string, std::vector<QuestCache>> cache2;
  if (!cache2.contains(id)) {
    if (auto n = node->find(u"Check/0/quest"); n != nullptr) {
      for (auto [k, v] : *n->get_children()) {
        auto q_id =
            static_cast<wz::Property<int> *>(v[0]->get_child(u"id"))->get();
        auto state =
            static_cast<wz::Property<int> *>(v[0]->get_child(u"state"))->get();
        auto tmp = std::format("{}.img", q_id);
        std::u16string id2{tmp.begin(), tmp.end()};
        cache2[id].push_back({id2, state});
      }
    } else {
      cache2[id] = {};
    }
  }
  auto quest_check = cache2.at(id);
  for (auto &qc : quest_check) {
    bool r = false;
    for (auto &qs : {progress_quests_, complete_quests_, decline_quests_}) {
      if (qs.contains(qc.id)) {
        auto q_state = qs.at(qc.id).index;
        if (q_state >= qc.state) {
          r = true;
          break;
        }
      }
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

int QuestGameInstance::LoadQuestArea(const std::u16string &id) {
  auto node = LoadQuestNode(id);
  auto ar = node->find(u"QuestInfo/area");
  auto area = static_cast<wz::Property<int> *>(ar)->get();
  return area;
}

std::flat_map<int8_t, std::vector<GameQuest>>
QuestGameInstance::LoadUiAvaliableQuest() {
  std::flat_map<int8_t, std::vector<GameQuest>> r;
  auto quests = LoadAvaliableQuest();
  std::vector<GameQuest> q;
  for (auto &quest : quests) {
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
  for (auto &quest : quests) {
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
  for (auto &quest : quests) {
    auto area = LoadQuestArea(quest.quest_id);
    r[area].push_back(quest);
  }
  return r;
}

int QuestGameInstance::LoadQuestIndex(const std::u16string &id) {
  auto node = WzResource::quest_->find(u"QuestData/" + id + u"/Act");
  return node->children_count() - 1;
}

int QuestGameInstance::LoadQuestProgress(const std::u16string &id) {
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
          static_cast<wz::Property<int> *>(n->get_child(u"category"))->get();
      auto name =
          static_cast<wz::Property<std::u16string> *>(n->get_child(u"title"))
              ->get();
      cache[category] = name;
    }
  }
  return cache.at(area);
}

wz::Node *QuestGameInstance::LoadQuestNode(const std::u16string &id) {
  auto node = WzResource::quest_->find(u"QuestData/" + id);
  return node;
}

void QuestGameInstance::Load(CharacterSaveData &cs) {
  AcceptQuest(u"10317.img");
  for (auto &q : cs.quests) {
    switch (q.type) {
    case QuestEnum::kProgress: {
      AcceptQuest(q);
      break;
    }
    case QuestEnum::kComplete: {
      CompleteQuest(q);
      break;
    }
    case QuestEnum::kDecline: {
      DeclineQuest(q);
      break;
    }
    }
  }

  QuestUiSystem::detail_ = true;
  QuestUiSystem::active_tab_ = 0;
  QuestUiSystem::disable_fold_.clear();
  QuestUiSystem::pages_ = {};
  QuestUiSystem::detail_quest_ = u"";
  if (auto aq = QuestGameInstance::LoadUiAvaliableQuest(); !aq.empty()) {
    QuestUiSystem::detail_quest_ = aq.begin()->second.begin()->quest_id;
  }
}

void QuestGameInstance::AcceptQuest(GameQuest &q) {
  q.index = 1;
  if (decline_quests_.contains(q.quest_id)) {
    q.index = decline_quests_[q.quest_id].index;
  }
  auto node = LoadQuestNode(q.quest_id);
  auto i = std::to_string(q.index);
  node = node->get_child(u"Check");
  node = node->get_child(i);
  if (auto n = node->get_child(u"item"); n != nullptr) {
    for (auto [k, v] : *n->get_children()) {
      auto id = static_cast<wz::Property<int> *>(v[0]->get_child(u"id"))->get();
      int count = 0;
      if (auto nn = v[0]->get_child(u"count"); nn != nullptr) {
        count = static_cast<wz::Property<int> *>(nn)->get();
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
      auto id = static_cast<wz::Property<int> *>(v[0]->get_child(u"id"))->get();
      int count = 0;
      if (auto nn = v[0]->get_child(u"count"); nn != nullptr) {
        count = static_cast<wz::Property<int> *>(nn)->get();
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

void QuestGameInstance::UpdateAcceptQuestTip(
    const std::u16string &quest_id) {
  const auto &quest = progress_quests_.at(quest_id);
  if (quest.check_item.empty() && quest.item_bool && quest.mob_bool) {
    PopupTipGameInstance::AddQuestTip(quest.quest_id);
  }
}

void QuestGameInstance::AcceptQuest(const std::u16string &id) {
  GameQuest q;
  q.quest_id = id;
  AcceptQuest(q);
  UpdateCheckMob(u"", 0);
  UpdateCheckItem(q.quest_id);
  UpdateAcceptQuestTip(q.quest_id);
}

void QuestGameInstance::CompleteQuest(GameQuest &q) {
  q.index += 1;
  complete_quests_[q.quest_id] = q;
  progress_quests_.erase(q.quest_id);
}

void QuestGameInstance::CompleteQuest(const std::u16string &id) {
  auto &q = progress_quests_.at(id);
  CompleteQuest(q);
}

void QuestGameInstance::DeclineQuest(GameQuest &q) {
  decline_quests_[q.quest_id] = q;
  progress_quests_.erase(q.quest_id);
  return;
}

void QuestGameInstance::DeclineQuest(const std::u16string &id) {
  auto &q = progress_quests_.at(id);
  DeclineQuest(q);
}

void QuestGameInstance::UpdateCheckItem(const std::u16string &quest_id) {
  auto &quest = progress_quests_.at(quest_id);
  bool pop_tip = false;

  for (auto [k, v] : quest.check_item) {
    int32_t back_num = -1;
    if (quest.item.contains(k)) {
      back_num = quest.item[k].count;
    }
    auto num = PackageGameInstance::LoadItemNum(k);
    if (back_num < (int32_t)v.count && num >= v.count) {
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
}

void QuestGameInstance::UpdateCheckItem() {
  for (auto [k, v] : progress_quests_) {
    UpdateCheckItem(k);
  }
}

void QuestGameInstance::UpdateCheckMob(const std::u16string &mob_id,
                                           int num) {
  for (auto [k, quest] : progress_quests_) {
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

std::flat_map<std::u16string, int>
QuestGameInstance::LoadQuestActItem(const std::u16string &id) {
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
      auto id = static_cast<wz::Property<int> *>(v[0]->get_child(u"id"))->get();
      auto count =
          static_cast<wz::Property<int> *>(v[0]->get_child(u"count"))->get();
      auto tmp = std::format("{:08d}", id);
      std::u16string id2{tmp.begin(), tmp.end()};
      r[id2] = count;
    }
  }
  return r;
}

std::flat_map<std::u16string, int>
QuestGameInstance::LoadQuestActRandomItem(const std::u16string &id) {
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
      auto id = static_cast<wz::Property<int> *>(v[0]->get_child(u"id"))->get();
      auto count =
          static_cast<wz::Property<int> *>(v[0]->get_child(u"count"))->get();
      auto tmp = std::format("{:08d}", id);
      std::u16string id2{tmp.begin(), tmp.end()};
      r[id2] = count;
    }
  }
  return r;
}

uint32_t QuestGameInstance::LoadQuestActExp(const std::u16string &id) {
  if (progress_quests_.contains(id)) {
    auto quest_node = LoadQuestNode(id);
    auto index = progress_quests_.at(id).index;
    quest_node = quest_node->find("Act/" + std::to_string(index) + "/exp");
    if (quest_node != nullptr) {
      return static_cast<wz::Property<int> *>(quest_node)->get();
    }
  }
  return 0;
}

int32_t QuestGameInstance::LoadQuestActMeso(const std::u16string &id) {
  if (progress_quests_.contains(id)) {
    auto quest_node = LoadQuestNode(id);
    auto index = progress_quests_.at(id).index;
    quest_node = quest_node->find("Act/" + std::to_string(index) + "/money");
    if (quest_node != nullptr) {
      return static_cast<wz::Property<int> *>(quest_node)->get();
    }
  }
  return 0;
}

}  // namespace sdlms
