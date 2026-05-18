#include "skill_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include <flat_map>
#include <string>

wz::Node *skill_game_instance::load_skill_node(const std::u16string &id) {
  static std::flat_map<std::u16string, wz::Node *> cache;
  if (!cache.contains(id)) {
    auto id2 = id.substr(0, id.length() - 4);
    cache[id] = wz_resource::skill->find(id2 + u".img/skill/" + id);
  }
  return cache.at(id);
}