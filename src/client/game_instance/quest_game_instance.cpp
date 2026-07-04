#include "quest_game_instance.h"
#include "src/client/game/game_quest.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <optional>
#include <string>

std::vector<game_quest> quest_game_instance::load_quest() {
  static std::vector<game_quest> cache;
  if (cache.empty()) {
    auto node = wz_resource::quest->find(u"QuestData");
    for (auto [k, v] : *node) {
      auto n = wz_resource::quest->find(u"QuestData/" + k);
      game_quest quest{
          .quest_id = k,
          .node = n,
      };
      cache.push_back(quest);
    }
  }
  return cache;
}

std::vector<game_quest>
quest_game_instance::load_npc_quest(const std::u16string &id) {
  std::vector<game_quest> q;
  auto quests = load_quest();
  for (const auto &quest : quests) {
    auto npc = quest.node->find(u"Check/0/npc");
    if (npc != nullptr) {
      auto npc_i = static_cast<wz::Property<int> *>(npc)->get();
      auto tmp = std::format("{:07d}", npc_i);
      std::u16string npc_id{tmp.begin(), tmp.end()};
      if (npc_id == id) {
        q.push_back(quest);
      }
    }
  }
  return q;
}