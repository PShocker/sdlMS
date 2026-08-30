#include "quest_game_instance.h"
#include "src/client/game/game_quest.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
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

std::vector<game_quest> quest_game_instance::load_avaliable_quest() {
  auto self = character_game_instance::self;
  static std::flat_map<int, std::vector<game_quest>> cache;
  if (!cache.contains(self.level)) {
    std::vector<game_quest> q;
    auto node = wz_resource::quest->find(u"QuestData");
    for (auto [k, v] : *node) {
      auto lv = node->find(k + u"/Check/0/lvmin");
      int lvl = 0;
      if (lv != nullptr) {
        lvl = static_cast<wz::Property<int> *>(lv)->get();
      }
      if (self.level >= lvl) {
        game_quest quest{
            .quest_id = k,
            .index = 0,
        };
        q.push_back(quest);
      }
    }
    cache[self.level] = q;
  }
  auto q = cache.at(self.level);
  auto p = quests;
  std::erase_if(q, [&p](const game_quest &quest) {
    return std::find_if(p.begin(), p.end(), [&](const game_quest &other) {
             return quest.quest_id == other.quest_id;
           }) != p.end();
  });
  //  job filter
  auto sf_job = std::stoi(std::string{self.job.begin(), self.job.end()});

  std::erase_if(q, [sf_job](const game_quest &quest) {
    auto node = load_quest_node(quest.quest_id);
    if (!node->find(u"Check/0/job")) {
      return false;
    } else {
      node = node->find(u"Check/0/job");
      for (auto [k, v] : *node->get_children()) {
        auto job = static_cast<wz::Property<int> *>(v[0])->get();
        bool result = sf_job == job;
        if (result) {
          return false;
        }
      }
    }
    return true;
  });
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
  std::vector<game_quest> q;
  for (const auto &v : quests) {
    if (!v.complete) {
      q.push_back(v);
    }
  }
  return q;
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
  std::vector<game_quest> q;
  for (const auto &v : quests) {
    if (v.complete) {
      q.push_back(v);
    }
  }
  return q;
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
  for (auto &q : quests) {
    if (id == q.quest_id) {
      return q.index;
    }
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

void quest_game_instance::load_quest_check(game_quest &q) {
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
      q.check_item.push_back(qi);
    }
  }
}

void quest_game_instance::load(const character_save &cs) {
  game_quest q;
  q.quest_id = u"1002.img";
  q.index = 1;
  load_quest_check(q);
  quests.push_back(q);
}