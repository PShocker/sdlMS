
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_drop.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/effect_logic_system.h"
#include "src/client/system/render/effect_render_system.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cstdint>
#include <vector>

static void jinqianzhadan() {
  GameSkill g_skill;
  g_skill.id = u"4211004";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = CharacterGameInstance::self;
    CharacterLogicSystem::run_action(sf, u"prone2");
    auto g_r = SkillGameInstance::load_ski_r(u"4211004");
    auto cm = CharacterLogicSystem::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    ClientDropFadeT ccdf;
    uint64_t meso = 0;
    for (const auto &[k, v] : DropGameInstance::data) {
      if (v.type != GameDrop::DropEnum::kLand) {
        continue;
      }
      if (SDL_PointInRectFloat(&v.pos, &g_r)) {
        if (v.data->id == u"00000000") {
          meso += ItemGameInstance::load_item_num(v.data);
          ccdf.random_id.push_back(v.random_id);
        }
      }
    }
    if (!ccdf.random_id.empty()) {
      ClientRequest::send_to_host(ccdf);
    }
    if (!cm.data.empty() && meso > 0) {
      auto delay = SkillGameInstance::load_ski_time(sf);
      // Create and send attack payload
      for (auto &data : cm.data) {
        data.hits = {(int64_t)meso};
      }
      cat = SkillGameInstance::create_attack_payload(cm, sf.pos, delay);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::create_skill_payload(cat, 4211004, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

static void luoyezhan() {
  GameSkill g_skill;
  g_skill.id = u"4211001";

  struct DataPack {
    GameCharacter *c;
    std::vector<GameEffect> es;
  };

  g_skill.effect = [](SDL_FPoint p, GameEffect *e, bool f) {
    DataPack *dp;
    const std::type_info &tp = e->data.type();
    if (tp == typeid(GameCharacter *)) {
      DataPack d;
      auto g_character = std::any_cast<GameCharacter *>(e->data);
      d.c = g_character;
      GameEffect ef = {
          .id = u"4211001",
          .index = 0,
          .time = 0,
          .delay = 0,
          .type = GameEffect::EffectType::kSkillUse,
          .pos = std::nullopt,
          .data = g_character,
      };
      d.es.emplace_back(ef);
      ef.order = 0;
      d.es.emplace_back(ef);
      e->data = d;
    }
    dp = std::any_cast<DataPack>(&e->data);

    if (EffectLogicSystem::run_effect(*e)) {
      return false;
    }
    auto g_character = std::any_cast<GameCharacter *>(e->data);
    EffectRenderSystem::render_effect(g_character->pos, *e);
    return false;
  };
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = CharacterGameInstance::self;
    CharacterLogicSystem::run_attack_action(sf, false);
    auto g_r = SkillGameInstance::load_ski_r(u"4211001");
    auto cm = CharacterLogicSystem::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::load_ski_time(sf);
      // Create and send attack payload
      cat = SkillGameInstance::create_attack_payload(cm, sf.pos, delay);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::create_skill_payload(cat, 4211001, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
    // 地面位移，与瞬移不同
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  jinqianzhadan();
  luoyezhan();
  return true;
}();