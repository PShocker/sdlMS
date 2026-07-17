#include "skill_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "character_game_instance.h"
#include "equip_game_instance.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "text_game_instance.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <array>
#include <chrono>
#include <cstdint>
#include <flat_map>
#include <string>

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

SDL_FRect skill_game_instance::load_ltrb(SDL_FPoint lt, SDL_FPoint rb,
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

SDL_FRect skill_game_instance::load_ski_ltrb(const std::u16string &id,
                                             uint8_t l,
                                             game_character &g_character) {
  auto level_node = load_ski_level_node(id, l);
  auto lt = wz_resource::load_fpoint(level_node->get_child(u"lt"));
  auto rb = wz_resource::load_fpoint(level_node->get_child(u"rb"));

  auto pos = g_character.pos;
  auto flip = g_character.flip;

  return load_ltrb(lt, rb, pos, flip);
}

SDL_FRect skill_game_instance::load_ski_r(const std::u16string &id, uint8_t l,
                                          game_character &g_character) {
  auto node = load_ski_level_node(id, l);
  if (node->get_child(u"lt")) {
    return load_ski_ltrb(id, l, g_character);
  }
  if (node->get_child(u"range")) {
    return {0, 0, 0, 0};
  }
  auto pos = g_character.pos;
  auto flip = g_character.flip;
  if (g_character.weapon.has_value()) {
    auto type = equip_game_instance::load_weapon_type(g_character);
    switch (type) {
    case equip_game_instance::weapon_type::BOW:
    case equip_game_instance::weapon_type::CROSSBOW: {
      auto lt = SDL_FPoint{-350, -90};
      auto rb = SDL_FPoint{-25, -10};
      return load_ltrb(lt, rb, pos, flip);
      break;
    }
    case equip_game_instance::weapon_type::CLAW: {
      auto lt = SDL_FPoint{-350, -90};
      auto rb = SDL_FPoint{-25, -10};
      return load_ltrb(lt, rb, pos, flip);
      break;
    }
    default: {
      auto r = afterimage_game_instance::load_rect(g_character);
      return r.value();
    }
    }
  }
  static std::flat_map<std::u16string, std::array<SDL_FPoint, 2>> hardcode = {
      {u"3201004", {SDL_FPoint{0.0f, 0.0f}, SDL_FPoint{0.0f, 0.0f}}}};
  auto lt = hardcode.at(id)[0];
  auto rb = hardcode.at(id)[1];
  return load_ltrb(lt, rb, pos, flip);
}

bool skill_game_instance::load_ski_attack(const std::u16string &id, uint8_t l) {
  auto level_node = load_ski_level_node(id, l);
  if (level_node->get_children()->contains(u"mobCount")) {
    return true;
  }
  return false;
}

bool skill_game_instance::load_ski_ball(const std::u16string &id, uint8_t l) {
  auto level_node = load_ski_level_node(id, l);
  if (level_node->get_children()->contains(u"ball") ||
      level_node->get_children()->contains(u"bulletCount")) {
    return true;
  }
  return false;
}

uint64_t skill_game_instance::load_ski_time(game_character &g_character) {
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