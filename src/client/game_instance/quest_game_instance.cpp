#include "quest_game_instance.h"
#include "src/client/game/game_quest.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <flat_map>
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
  return q;
}

std::vector<game_quest> quest_game_instance::load_progress_quest() {
  auto q = quests;
  for (const auto &v : q) {
    if (!v.complete) {
      q.push_back(v);
    }
  }
  return q;
}

std::vector<game_quest> quest_game_instance::load_complete_quest() {
  auto q = quests;
  for (const auto &v : q) {
    if (v.complete) {
      q.push_back(v);
    }
  }
  return q;
}

int quest_game_instance::load_quest_index(const std::u16string &id) {
  auto node = wz_resource::quest->find(u"QuestData/" + id + u"/Act");
  return node->children_count() - 1;
}