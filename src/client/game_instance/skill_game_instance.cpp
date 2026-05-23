#include "skill_game_instance.h"
#include "character_game_instance.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include <chrono>
#include <cstdint>
#include <flat_map>
#include <string>

wz::Node *skill_game_instance::load_skill_level_node(const std::u16string &id,
                                                     uint8_t l) {
  auto skill_node = load_skill_node(id);
  auto l2 = std::to_string(l);
  auto level_node = skill_node->get_child(u"level")->get_child(l2);
  return level_node;
}

wz::Node *skill_game_instance::load_skill_node(const std::u16string &id) {
  static std::flat_map<std::u16string, wz::Node *> cache;
  if (!cache.contains(id)) {
    auto id2 = id.substr(0, id.length() - 4);
    cache[id] = wz_resource::skill->find(id2 + u".img/skill/" + id);
  }
  return cache.at(id);
}

SDL_FRect skill_game_instance::load_skill_rect(const std::u16string &id,
                                               uint8_t l) {
  auto level_node = load_skill_level_node(id, l);
  auto lt = wz_resource::load_fpoint(level_node->get_child(u"lt"));
  auto rb = wz_resource::load_fpoint(level_node->get_child(u"rb"));
  SDL_FRect rect{
      .x = lt.x,
      .y = lt.y,
      .w = rb.x - lt.x,
      .h = rb.y - lt.y,
  };
  auto pos = character_game_instance::self.pos;
  auto flip = character_game_instance::self.flip;
  rect.x += pos.x;
  rect.y += pos.y;
  if (flip == 1) {
    rect.x += 2 * (pos.x - rect.x) - rect.w;
  }
  return rect;
}

skill_game_instance::skill_type
skill_game_instance::load_skill_type(const std::u16string &id, uint8_t l) {
  return attack;
}

uint64_t skill_game_instance::load_beat_time(game_character &g_character) {
  uint64_t r = std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count();
  if (character_game_instance::bone_data.contains(g_character.action)) {
    r = afterimage_game_instance::load_beat_time(g_character);
  } else {
    auto &externs =
        character_game_instance::extern_action.at(g_character.action);
    for (auto e : externs) {
      if (e.delay < 0) {
        r -= e.delay;
      } else {
        break;
      }
    }
  }
  return r;
}