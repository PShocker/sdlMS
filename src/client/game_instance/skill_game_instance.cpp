#include "skill_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "character_game_instance.h"
#include "src/client/game/game_gauge.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
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

wz::Node *SkillGameInstance::load_ski_level_node(const std::u16string &id,
                                                   uint8_t l) {
  auto skill_node = load_ski_node(id);
  auto l2 = std::to_string(l);
  auto level_node = skill_node->get_child(u"level")->get_child(l2);
  return level_node;
}

wz::Node *SkillGameInstance::load_ski_node(uint32_t id) {
  auto tmp = std::format("{:07d}", id);
  std::u16string ski_id{tmp.begin(), tmp.end()};
  return load_ski_node(ski_id);
}

wz::Node *SkillGameInstance::load_ski_node(const std::string &id) {
  return load_ski_node(std::u16string{id.begin(), id.end()});
}

wz::Node *SkillGameInstance::load_ski_node(const std::u16string &id) {
  static std::flat_map<std::u16string, wz::Node *> cache;
  if (!cache.contains(id)) {
    auto id2 = id.substr(0, id.length() - 4);
    cache[id] = WzResource::skill->find(id2 + u".img/skill/" + id);
  }
  return cache.at(id);
}

uint64_t SkillGameInstance::load_ski_time(GameCharacter &g_character) {
  uint64_t r = Window::dt_time;
  if (CharacterGameInstance::bone_data.contains(g_character.action)) {
    r = AfterimageGameInstance::load_beat_time(g_character);
  } else {
    auto &externs =
        CharacterGameInstance::extern_action.at(g_character.action);
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

SDL_FRect SkillGameInstance::load_r(SDL_FPoint lt, SDL_FPoint rb,
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

SkillGameInstance::SkillName
SkillGameInstance::load_ski_name(const std::u16string &id) {
  static std::flat_map<std::u16string, SkillName> cache;
  if (!cache.contains(id)) {
    static auto ski_node = WzResource::string->find(u"Skill.img");
    auto node = ski_node->get_child(id);
    SkillName ski_name;
    ski_name.name =
        static_cast<wz::Property<std::u16string> *>(node->get_child(u"name"))
            ->get();
    ski_name.desc = TextGameInstance::load_rstr(node->get_child(u"desc"));
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

int SkillGameInstance::load_ski_max_lvl(const std::u16string &id) {
  auto ski_name = load_ski_name(id);
  return ski_name.level.size();
}

bool SkillGameInstance::load_ski_active(const std::u16string &id) {
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

SDL_FRect SkillGameInstance::load_ski_r(const std::u16string &id,
                                          SDL_FPoint pos, bool flip) {
  auto ski_lv = JobSkillGameInstance::load_ski_level(id);
  auto ski_node = load_ski_level_node(id, ski_lv);
  if (ski_node->get_child(u"lt")) {
    auto lt = WzResource::load_fpoint(ski_node->get_child(u"lt"));
    auto rb = WzResource::load_fpoint(ski_node->get_child(u"rb"));
    auto g_r = SkillGameInstance::load_r(lt, rb, pos, flip);
    return g_r;
  }
  if (ski_node->get_child(u"range")) {
    auto range =
        static_cast<wz::Property<int> *>(ski_node->get_child(u"range"))->get();
    SDL_FPoint lt = {static_cast<float>(-range), -100};
    SDL_FPoint rb = {0, 0};
    auto g_r = SkillGameInstance::load_r(lt, rb, pos, flip);
    return g_r;
  }
  // mag
  if (id.starts_with(u"2")) {
    SDL_FPoint lt = {-300, -90};
    SDL_FPoint rb = {-25, 10};
    auto g_r = SkillGameInstance::load_r(lt, rb, pos, flip);
    return g_r;
  }
  // afterimage
  auto &sf = CharacterGameInstance::self;
  auto g_r = AfterimageGameInstance::load_rect(sf);
  if (g_r.has_value()) {
    return g_r.value();
  }
  return {0, 0, 0, 0};
}

SDL_FRect SkillGameInstance::load_ski_r(const std::u16string &id) {
  auto &sf = CharacterGameInstance::self;
  return load_ski_r(id, sf.pos, sf.flip);
}

int SkillGameInstance::load_ski_ball_num(const std::u16string &id,
                                           uint8_t lv) {
  auto ski_node = load_ski_level_node(id, lv);
  int r = 0;
  if (ski_node->get_child(u"bulletCount")) {
    r = static_cast<wz::Property<int> *>(ski_node->get_child(u"bulletCount"))
            ->get();
  }
  return r;
}

int SkillGameInstance::load_ski_mob_count(const std::u16string &id,
                                            uint8_t lv) {
  auto ski_node = load_ski_level_node(id, lv);
  int r = 1;
  if (ski_node->get_child(u"mobCount")) {
    r = static_cast<wz::Property<int> *>(ski_node->get_child(u"mobCount"))
            ->get();
  }
  return r;
}

int SkillGameInstance::load_ski_atk_count(const std::u16string &id,
                                            uint8_t lv) {
  auto ski_node = load_ski_level_node(id, lv);
  int r = 1;
  if (ski_node->get_child(u"attackCount")) {
    r = static_cast<wz::Property<int> *>(ski_node->get_child(u"attackCount"))
            ->get();
  }
  return r;
}

ClientCharacterAttackT SkillGameInstance::create_attack_payload(
    CheckMobs &cm, SDL_FPoint pos, uint64_t delay, uint32_t interval,
    std::string effect, std::string sound) {
  ClientCharacterAttackT attack_payload;
  auto &Mobs = cm.data;
  for (int m = 0; m < Mobs.size(); m++) {
    auto &mob = Mobs[m];
    auto &mb = MobGameInstance::data.at(mob.mob.index).mob;
    for (int n = 0; n < mob.hits.size(); n++) {
      CharacterAttackT ct;
      ct.mob_index = mob.mob.index;
      ct.attack = std::make_unique<AttackT>();
      ct.attack->num = mob.hits[n];
      ct.attack->delay = delay + n * interval + mob.delay;
      ct.attack->x = mob.x;
      ct.attack->y = mob.y;
      ct.left = pos.x < mob.mob.pos.x;
      ct.effect = effect;
      ct.sound = sound;
      attack_payload.payload.push_back(
          std::make_unique<CharacterAttackT>(std::move(ct)));
    }
    // gauge
    if (!mb.gauge.has_value()) {
      GameGauge g;
      g.hp_percent = (float)mob.mob.hp / mob.mob.max_hp;
      g.hp_percent_now = g.hp_percent;
      mb.gauge = g;
    }
  }
  return attack_payload;
}

ClientCharacterAttackT
SkillGameInstance::create_attack_payload(CheckMobs &cm, SDL_FPoint pos,
                                           uint64_t delay) {
  return create_attack_payload(cm, pos, delay, 60, "", "");
}

ClientCharacterSkillT SkillGameInstance::create_skill_payload(
    const ClientCharacterAttackT &attack_payload, int ski_id, uint8_t ski_lv) {
  ClientCharacterSkillT skill_payload;
  skill_payload.ski_id = ski_id;
  skill_payload.ski_lv = ski_lv;
  skill_payload.x = INT32_MAX;
  skill_payload.y = INT32_MAX;

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

ClientCharacterSkillT SkillGameInstance::create_skill_payload(
    const std::vector<uint64_t> &buff_payload, int ski_id, uint8_t ski_lv) {
  ClientCharacterSkillT skill_payload;
  skill_payload.ski_id = ski_id;
  skill_payload.ski_lv = ski_lv;
  skill_payload.x = INT32_MAX;
  skill_payload.y = INT32_MAX;

  for (const auto &client_id : buff_payload) {
    CharacterSkillT c;
    c.delay = 0;
    c.player = client_id;
    skill_payload.payload.push_back(
        std::make_unique<CharacterSkillT>(std::move(c)));
  }
  return skill_payload;
}

ClientCharacterSkillT
SkillGameInstance::create_skill_payload(int ski_id, uint8_t ski_lv) {
  ClientCharacterSkillT skill_payload;
  skill_payload.ski_id = ski_id;
  skill_payload.ski_lv = ski_lv;
  skill_payload.x = INT32_MAX;
  skill_payload.y = INT32_MAX;

  return skill_payload;
}