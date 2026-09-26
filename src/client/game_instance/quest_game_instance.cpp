#include "quest_game_instance.h"
#include "popup_tip_game_instance.h"
#include "src/client/game/game_quest.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <format>
#include <optional>
#include <string>

std::vector<game_quest>
quest_game_instance::load_npc(const std::u16string &id) {
  std::vector<game_quest> q;
  auto node = wz_resource::quest->find(u"QuestData");
  for (auto [k, v] : *node) {
    auto npc = node->find(k + u"/Check/0/npc");
    if (npc != nullptr) {
      auto npc_i = static_cast<wz::Property<int> *>(npc)->get();
      auto tmp = std::format("{:07d}", npc_i);
      std::u16string npc_id{tmp.begin(), tmp.end()};
      if (npc_id == id) {
        game_quest quest{
            .quest_id = k,
        };
        q.push_back(quest);
      }
    }
  }
  return q;
}

std::vector<game_quest>
quest_game_instance::load_npc_quest(const std::u16string &id) {
  static std::flat_map<std::u16string, std::vector<game_quest>> cache;
  if (!cache.contains(id)) {
    cache[id] = load_npc(id);
  }
  return cache.at(id);
}

std::optional<game_quest>
quest_game_instance::load_avaliable_quest(const std::u16string &id) {
  struct key_cache {
    std::u16string id;
    int level;
    std::u16string job;

    auto operator<=>(const key_cache &) const = default;
  };
  static std::flat_map<key_cache, std::optional<game_quest>> cache;
  auto &sf = character_game_instance::self;
  auto node = load_quest_node(id);
  key_cache kc{id, sf.level, sf.job};
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
    cache.emplace(kc, game_quest{.quest_id = id});
  }
  auto q = cache.at(kc);
  if (!q.has_value()) {
    return std::nullopt;
  }
  struct quest_cache {
    std::u16string id;
    int state;
  };
  static std::flat_map<std::u16string, std::vector<quest_cache>> cache2;
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
    for (auto &qs : {progress_quests, complete_quests, decline_quests}) {
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
  if (progress_quests.contains(id) || complete_quests.contains(id)) {
    return std::nullopt;
  }
  auto quest = q.value();
  if (decline_quests.contains(id)) {
    quest.index = decline_quests[id].index;
  }
  return quest;
}

std::vector<game_quest> quest_game_instance::load_avaliable_quest() {
  static auto node = wz_resource::quest->find(u"QuestData");
  std::vector<game_quest> q;
  for (auto k : *node->get_children() | std::views::keys) {
    auto qs = load_avaliable_quest(k);
    if (qs.has_value()) {
      q.push_back(qs.value());
    }
  }
  return q;
}

int quest_game_instance::load_quest_area(const std::u16string &id) {
  auto node = load_quest_node(id);
  auto ar = node->find(u"QuestInfo/area");
  auto area = static_cast<wz::Property<int> *>(ar)->get();
  return area;
}

std::flat_map<int8_t, std::vector<game_quest>>
quest_game_instance::load_ui_avaliable_quest() {
  std::flat_map<int8_t, std::vector<game_quest>> r;
  auto quests = load_avaliable_quest();
  std::vector<game_quest> q;
  for (auto &quest : quests) {
    auto area = load_quest_area(quest.quest_id);
    r[area].push_back(quest);
  }
  return r;
}

std::vector<game_quest> quest_game_instance::load_progress_quest() {
  return progress_quests | std::views::values | std::ranges::to<std::vector>();
}

std::flat_map<int8_t, std::vector<game_quest>>
quest_game_instance::load_ui_progress_quest() {
  std::flat_map<int8_t, std::vector<game_quest>> r;
  auto quests = load_progress_quest();
  std::vector<game_quest> q;
  for (auto &quest : quests) {
    auto area = load_quest_area(quest.quest_id);
    r[area].push_back(quest);
  }
  return r;
}

std::vector<game_quest> quest_game_instance::load_complete_quest() {
  return complete_quests | std::views::values | std::ranges::to<std::vector>();
}

std::flat_map<int8_t, std::vector<game_quest>>
quest_game_instance::load_ui_complete_quest() {
  std::flat_map<int8_t, std::vector<game_quest>> r;
  auto quests = load_complete_quest();
  std::vector<game_quest> q;
  for (auto &quest : quests) {
    auto area = load_quest_area(quest.quest_id);
    r[area].push_back(quest);
  }
  return r;
}

int quest_game_instance::load_quest_index(const std::u16string &id) {
  auto node = wz_resource::quest->find(u"QuestData/" + id + u"/Act");
  return node->children_count() - 1;
}

int quest_game_instance::load_quest_progress(const std::u16string &id) {
  if (progress_quests.contains(id)) {
    return progress_quests[id].index;
  }
  return 0;
}

std::u16string quest_game_instance::load_area_name(int area) {
  static std::flat_map<int, std::u16string> cache;
  if (!cache.contains(area)) {
    auto node = wz_resource::quest->find(u"QuestCategory.img");
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

wz::Node *quest_game_instance::load_quest_node(const std::u16string &id) {
  auto node = wz_resource::quest->find(u"QuestData/" + id);
  return node;
}

void quest_game_instance::load(character_save &cs) {
  for (auto &q : cs.quests) {
    switch (q.type) {
    case quest_enum::progress: {
      accept_quest(q);
      break;
    }
    case quest_enum::complete: {
      complete_quest(q);
      break;
    }
    case quest_enum::decline: {
      decline_quest(q);
      break;
    }
    }
  }
}

void quest_game_instance::accept_quest(game_quest &q) {
  q.index = 1;
  if (decline_quests.contains(q.quest_id)) {
    q.index = decline_quests[q.quest_id].index;
  }
  auto node = load_quest_node(q.quest_id);
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
      quest_item qi;
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
      quest_mob qm;
      auto tmp = std::format("{:07d}", id);
      qm.id = {tmp.begin(), tmp.end()};
      qm.count = count;
      q.check_mob[qm.id] = qm;
    }
  }
  progress_quests[q.quest_id] = q;
}

void quest_game_instance::update_accept_quest_tip(
    const std::u16string &quest_id) {
  const auto &quest = progress_quests.at(quest_id);
  if (quest.check_item.empty() && quest.item_bool && quest.mob_bool) {
    popup_tip_game_instance::add_quest_tip(quest.quest_id);
  }
}

void quest_game_instance::accept_quest(const std::u16string &id) {
  game_quest q;
  q.quest_id = id;
  accept_quest(q);
  update_check_mob(u"", 0);
  update_check_item(q.quest_id);
  update_accept_quest_tip(q.quest_id);
}

void quest_game_instance::complete_quest(game_quest &q) {
  q.index += 1;
  complete_quests[q.quest_id] = q;
  progress_quests.erase(q.quest_id);
}

void quest_game_instance::complete_quest(const std::u16string &id) {
  auto &q = progress_quests.at(id);
  complete_quest(q);
}

void quest_game_instance::decline_quest(game_quest &q) {
  decline_quests[q.quest_id] = q;
  progress_quests.erase(q.quest_id);
  return;
}

void quest_game_instance::decline_quest(const std::u16string &id) {
  auto &q = progress_quests.at(id);
  decline_quest(q);
}

void quest_game_instance::update_check_item(const std::u16string &quest_id) {
  auto &quest = progress_quests.at(quest_id);
  bool pop_tip = false;

  for (auto [k, v] : quest.check_item) {
    int32_t back_num = -1;
    if (quest.item.contains(k)) {
      back_num = quest.item[k].count;
    }
    auto num = package_game_instance::load_item_num(k);
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
    popup_tip_game_instance::add_quest_tip(quest.quest_id);
  }
}

void quest_game_instance::update_check_item() {
  for (auto [k, v] : progress_quests) {
    update_check_item(k);
  }
}

void quest_game_instance::update_check_mob(const std::u16string &mob_id,
                                           int num) {
  for (auto [k, quest] : progress_quests) {
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
      popup_tip_game_instance::add_quest_tip(quest.quest_id);
    }
  }
}

std::flat_map<std::u16string, int>
quest_game_instance::load_quest_act_item(const std::u16string &id) {
  std::flat_map<std::u16string, int> r;
  uint8_t index = 0;
  if (progress_quests.contains(id)) {
    index = progress_quests.at(id).index;
  }
  auto quest_node = load_quest_node(id);
  quest_node = quest_node->find("Act/" + std::to_string(index) + "/item");
  if (quest_node != nullptr) {
    for (auto [k, v] : *quest_node->get_children()) {
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

uint32_t quest_game_instance::load_quest_act_exp(const std::u16string &id) {
  if (progress_quests.contains(id)) {
    auto quest_node = load_quest_node(id);
    auto index = progress_quests.at(id).index;
    quest_node = quest_node->find("Act/" + std::to_string(index) + "/exp");
    if (quest_node != nullptr) {
      return static_cast<wz::Property<int> *>(quest_node)->get();
    }
  }
  return 0;
}

int32_t quest_game_instance::load_quest_act_meso(const std::u16string &id) {
  if (progress_quests.contains(id)) {
    auto quest_node = load_quest_node(id);
    auto index = progress_quests.at(id).index;
    quest_node = quest_node->find("Act/" + std::to_string(index) + "/money");
    if (quest_node != nullptr) {
      return static_cast<wz::Property<int> *>(quest_node)->get();
    }
  }
  return 0;
}