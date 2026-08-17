#include "skill_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "character_game_instance.h"
#include "equip_game_instance.h"
#include "src/client/game/game_gauge.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "text_game_instance.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
#include <string>

using namespace fbs;

wz::Node *skill_game_instance::load_ski_level_node(const std::u16string &id,
                                                   uint8_t l) {
  auto skill_node = load_ski_node(id);
  auto l2 = std::to_string(l);
  auto level_node = skill_node->get_child(u"level")->get_child(l2);
  return level_node;
}

wz::Node *skill_game_instance::load_ski_node(uint32_t id) {
  auto tmp = std::format("{:07d}", id);
  std::u16string ski_id{tmp.begin(), tmp.end()};
  return load_ski_node(ski_id);
}

wz::Node *skill_game_instance::load_ski_node(const std::string &id) {
  return load_ski_node(std::u16string{id.begin(), id.end()});
}

wz::Node *skill_game_instance::load_ski_node(const std::u16string &id) {
  static std::flat_map<std::u16string, wz::Node *> cache;
  if (!cache.contains(id)) {
    auto id2 = id.substr(0, id.length() - 4);
    cache[id] = wz_resource::skill->find(id2 + u".img/skill/" + id);
  }
  return cache.at(id);
}

uint64_t skill_game_instance::load_ski_time(game_character &g_character) {
  uint64_t r = window::dt_time;
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

SDL_FRect skill_game_instance::load_r(SDL_FPoint lt, SDL_FPoint rb,
                                      SDL_FPoint pos, bool flip) {
  SDL_FRect rect{
      .x = lt.x,
      .y = lt.y,
      .w = rb.x - lt.x,
      .h = rb.y - lt.y,
  };
  rect.x += pos.x;
  rect.y += pos.y;
  if (flip == 1) {
    rect.x += 2 * (pos.x - rect.x) - rect.w;
  }
  return rect;
}

skill_game_instance::skill_name
skill_game_instance::load_ski_name(const std::u16string &id) {
  static std::flat_map<std::u16string, skill_name> cache;
  if (!cache.contains(id)) {
    static auto ski_node = wz_resource::string->find(u"Skill.img");
    auto node = ski_node->get_child(id);
    skill_name ski_name;
    ski_name.name =
        static_cast<wz::Property<std::u16string> *>(node->get_child(u"name"))
            ->get();
    ski_name.desc = text_game_instance::load_rstr(node->get_child(u"desc"));
    auto child = node->get_children();
    child->erase(u"name");
    child->erase(u"desc");
    for (auto [k, v] : *child) {
      auto level = static_cast<wz::Property<std::u16string> *>(v[0])->get();
      ski_name.level.push_back(level);
    }
    cache[id] = ski_name;
  }
  return cache.at(id);
}

int skill_game_instance::load_ski_max_lvl(const std::u16string &id) {
  auto ski_name = load_ski_name(id);
  return ski_name.level.size();
}

bool skill_game_instance::load_ski_active(const std::u16string &id) {
  auto ski_node = load_ski_node(id);
  if (ski_node->get_child(u"action")) {
    return true;
  }
  if (ski_node->get_child(u"hit")) {
    return true;
  }
  if (ski_node->get_child(u"effect")) {
    return true;
  }
  return false;
}

ClientCharacterAttackT
skill_game_instance::create_attack_payload(check_mobs &cm, SDL_FPoint pos,
                                           uint64_t delay, uint32_t interval) {
  ClientCharacterAttackT attack_payload;
  auto &mobs = cm.data;

  for (int m = 0; m < mobs.size(); m++) {
    auto &mob = mobs[m];
    for (int n = 0; n < mob.hits.size(); n++) {
      CharacterAttackT ct;
      ct.mob_index = mob.mob.index;
      ct.attack = std::make_unique<AttackT>();
      ct.attack->num = mob.hits[n];
      ct.attack->delay = delay + n * interval;
      ct.attack->x = mob.x;
      ct.attack->y = mob.y;
      ct.left = pos.x < mob.mob.pos.x;
      attack_payload.payload.push_back(
          std::make_unique<CharacterAttackT>(std::move(ct)));
    }
    // gauge
    if (!mob_game_instance::data.at(mob.mob.index).mob.gauge.has_value()) {
      game_gauge g;
      g.hp_percent = (float)mob.mob.hp / mob.mob.max_hp;
      g.hp_percent_now = g.hp_percent;
      mob_game_instance::data.at(mob.mob.index).mob.gauge = g;
    }
  }
  return attack_payload;
}

ClientCharacterSkillT skill_game_instance::create_skill_payload(
    const ClientCharacterAttackT &attack_payload, int ski_id, uint8_t ski_lv) {
  ClientCharacterSkillT skill_payload;
  skill_payload.ski_id = ski_id;
  skill_payload.ski_lv = ski_lv;

  for (const auto &a : attack_payload.payload) {
    CharacterSkillT c;
    c.delay = a->attack->delay;
    c.mob = a->mob_index;
    c.x = a->attack->x;
    c.y = a->attack->y;
    skill_payload.payload.push_back(
        std::make_unique<CharacterSkillT>(std::move(c)));
  }
  return skill_payload;
}