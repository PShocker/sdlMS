#include "src/client/game_instance/skill_game_instance.h"

#include "src/client/game/game_gauge.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <cstdint>
#include <flat_map>
#include <string>
namespace sdlms {


wz::Node* SkillGameInstance::LoadSkiLevelNode(const std::u16string& id,
                                              uint8_t l) {
  auto skill_node = LoadSkiNode(id);
  auto l2 = std::to_string(l);
  auto level_node = skill_node->get_child(u"level")->get_child(l2);
  return level_node;
}

wz::Node* SkillGameInstance::LoadSkiNode(uint32_t id) {
  auto tmp = std::format("{:07d}", id);
  std::u16string ski_id{tmp.begin(), tmp.end()};
  return LoadSkiNode(ski_id);
}

wz::Node* SkillGameInstance::LoadSkiNode(const std::string& id) {
  return LoadSkiNode(std::u16string{id.begin(), id.end()});
}

wz::Node* SkillGameInstance::LoadSkiNode(const std::u16string& id) {
  static std::flat_map<std::u16string, wz::Node*> cache;
  if (!cache.contains(id)) {
    auto id2 = id.substr(0, id.length() - 4);
    cache[id] = WzResource::skill_->find(id2 + u".img/skill/" + id);
  }
  return cache.at(id);
}

uint64_t SkillGameInstance::LoadSkiTime(GameCharacter& g_character) {
  uint64_t r = Window::dt_time_;
  if (CharacterGameInstance::bone_data_.contains(g_character.action)) {
    r = AfterimageGameInstance::LoadBeatTime(g_character);
  } else {
    auto& externs =
        CharacterGameInstance::extern_action_.at(g_character.action);
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

SDL_FRect SkillGameInstance::LoadR(SDL_FPoint lt, SDL_FPoint rb, SDL_FPoint pos,
                                   bool flip) {
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

SkillGameInstance::SkillName SkillGameInstance::LoadSkiName(
    const std::u16string& id) {
  static std::flat_map<std::u16string, SkillName> cache;
  if (!cache.contains(id)) {
    static auto ski_node = WzResource::string_->find(u"Skill.img");
    auto node = ski_node->get_child(id);
    SkillName ski_name;
    ski_name.name =
        static_cast<wz::Property<std::u16string>*>(node->get_child(u"name"))
            ->get();
    ski_name.desc = TextGameInstance::LoadRstr(node->get_child(u"desc"));
    auto child = node->get_children();
    child->erase(u"name");
    child->erase(u"desc");
    for (auto [k, v] : *child) {
      auto level = static_cast<wz::Property<std::u16string>*>(v[0])->get();
      ski_name.level.push_back(level);
    }
    cache[id] = ski_name;
  }
  return cache.at(id);
}

int SkillGameInstance::LoadSkiMaxLvl(const std::u16string& id) {
  auto ski_name = LoadSkiName(id);
  return ski_name.level.size();
}

bool SkillGameInstance::LoadSkiActive(const std::u16string& id) {
  auto ski_node = LoadSkiNode(id);
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

SDL_FRect SkillGameInstance::LoadSkiR(const std::u16string& id, SDL_FPoint pos,
                                      bool flip) {
  auto ski_lv = JobSkillGameInstance::LoadSkiLevel(id);
  auto ski_node = LoadSkiLevelNode(id, ski_lv);
  if (ski_node->get_child(u"lt")) {
    auto lt = WzResource::LoadFpoint(ski_node->get_child(u"lt"));
    auto rb = WzResource::LoadFpoint(ski_node->get_child(u"rb"));
    auto g_r = SkillGameInstance::LoadR(lt, rb, pos, flip);
    return g_r;
  }
  if (ski_node->get_child(u"range")) {
    auto range =
        static_cast<wz::Property<int>*>(ski_node->get_child(u"range"))->get();
    SDL_FPoint lt = {static_cast<float>(-range), -100};
    SDL_FPoint rb = {0, 0};
    auto g_r = SkillGameInstance::LoadR(lt, rb, pos, flip);
    return g_r;
  }
  // mag
  if (id.starts_with(u"2")) {
    SDL_FPoint lt = {-300, -90};
    SDL_FPoint rb = {-25, 10};
    auto g_r = SkillGameInstance::LoadR(lt, rb, pos, flip);
    return g_r;
  }
  // afterimage
  auto& sf = CharacterGameInstance::self_;
  auto g_r = AfterimageGameInstance::LoadRect(sf);
  if (g_r.has_value()) {
    return g_r.value();
  }
  return {0, 0, 0, 0};
}

SDL_FRect SkillGameInstance::LoadSkiR(const std::u16string& id) {
  auto& sf = CharacterGameInstance::self_;
  return LoadSkiR(id, sf.pos, sf.flip);
}

int SkillGameInstance::LoadSkiBallNum(const std::u16string& id, uint8_t lv) {
  auto ski_node = LoadSkiLevelNode(id, lv);
  int r = 0;
  if (ski_node->get_child(u"bulletCount")) {
    r = static_cast<wz::Property<int>*>(ski_node->get_child(u"bulletCount"))
            ->get();
  }
  return r;
}

int SkillGameInstance::LoadSkiMobCount(const std::u16string& id, uint8_t lv) {
  auto ski_node = LoadSkiLevelNode(id, lv);
  int r = 1;
  if (ski_node->get_child(u"mobCount")) {
    r = static_cast<wz::Property<int>*>(ski_node->get_child(u"mobCount"))
            ->get();
  }
  return r;
}

int SkillGameInstance::LoadSkiAtkCount(const std::u16string& id, uint8_t lv) {
  auto ski_node = LoadSkiLevelNode(id, lv);
  int r = 1;
  if (ski_node->get_child(u"attackCount")) {
    r = static_cast<wz::Property<int>*>(ski_node->get_child(u"attackCount"))
            ->get();
  }
  return r;
}

fbs::ClientCharacterAttackT SkillGameInstance::CreateAttackPayload(
    CheckMobs& cm, SDL_FPoint pos, uint64_t delay, uint32_t interval,
    std::string effect, std::string sound) {
  fbs::ClientCharacterAttackT attack_payload;
  auto& Mobs = cm.data;
  for (int m = 0; m < Mobs.size(); m++) {
    auto& mob = Mobs[m];
    auto& mb = MobGameInstance::data_.at(mob.mob.index).mob;
    for (int n = 0; n < mob.hits.size(); n++) {
      fbs::CharacterAttackT ct;
      ct.mob_index = mob.mob.index;
      ct.attack = std::make_unique<fbs::AttackT>();
      ct.attack->num = mob.hits[n];
      ct.attack->delay = delay + n * interval + mob.delay;
      ct.attack->x = mob.x;
      ct.attack->y = mob.y;
      ct.left = pos.x < mob.mob.pos.x;
      ct.effect = effect;
      ct.sound = sound;
      attack_payload.payload.push_back(
          std::make_unique<fbs::CharacterAttackT>(std::move(ct)));
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

fbs::ClientCharacterAttackT SkillGameInstance::CreateAttackPayload(
    CheckMobs& cm, SDL_FPoint pos, uint64_t delay) {
  return CreateAttackPayload(cm, pos, delay, 60, "", "");
}

fbs::ClientCharacterSkillT SkillGameInstance::CreateSkillPayload(
    const fbs::ClientCharacterAttackT& attack_payload, int ski_id,
    uint8_t ski_lv) {
  fbs::ClientCharacterSkillT skill_payload;
  skill_payload.ski_id = ski_id;
  skill_payload.ski_lv = ski_lv;
  skill_payload.x = INT32_MAX;
  skill_payload.y = INT32_MAX;

  for (const auto& a : attack_payload.payload) {
    fbs::CharacterSkillT c;
    c.delay = a->attack->delay;
    c.mob = a->mob_index;
    c.x = a->attack->x;
    c.y = a->attack->y;
    skill_payload.payload.push_back(
        std::make_unique<fbs::CharacterSkillT>(std::move(c)));
  }
  return skill_payload;
}

fbs::ClientCharacterSkillT SkillGameInstance::CreateSkillPayload(
    const std::vector<uint64_t>& buff_payload, int ski_id, uint8_t ski_lv) {
  fbs::ClientCharacterSkillT skill_payload;
  skill_payload.ski_id = ski_id;
  skill_payload.ski_lv = ski_lv;
  skill_payload.x = INT32_MAX;
  skill_payload.y = INT32_MAX;

  for (const auto& client_id : buff_payload) {
    fbs::CharacterSkillT c;
    c.delay = 0;
    c.player = client_id;
    skill_payload.payload.push_back(
        std::make_unique<fbs::CharacterSkillT>(std::move(c)));
  }
  return skill_payload;
}

fbs::ClientCharacterSkillT SkillGameInstance::CreateSkillPayload(
    int ski_id, uint8_t ski_lv) {
  fbs::ClientCharacterSkillT skill_payload;
  skill_payload.ski_id = ski_id;
  skill_payload.ski_lv = ski_lv;
  skill_payload.x = INT32_MAX;
  skill_payload.y = INT32_MAX;

  return skill_payload;
}

}  // namespace sdlms
