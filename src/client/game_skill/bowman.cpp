
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cstdint>
#include <memory>
#include <string>

static void duanhunjian() {
  GameSkill g_skill;
  g_skill.id = u"3001001";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    GameTriangle tri = {
        {
            SDL_FPoint{-350, -100},
            SDL_FPoint{-350, 100},
            SDL_FPoint{0, -28},
        },
    };
    auto &sf = CharacterGameInstance::self;
    CharacterLogicSystem::run_action(sf, u"swingO1");
    auto cm = CharacterLogicSystem::run_attack_check(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
    }
    auto delay = SkillGameInstance::load_ski_time(sf);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"300.img/skill/3001001/ball";
    auto pos = sf.pos;
    pos.y -= 28;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    if (sf.flip) {
      goal.x += 350;
    } else {
      goal.x -= 350;
    }
    auto cct = BallGameInstance::create_ball_payload(cm, pos, goal, delay,
                                                       page, 700, path);
    ClientRequest::send_to_host(cct);
    ServerBallInstance::handle_server_b(cct.payload);

    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::load_ball_time(cct);
      // Create and send attack payload
      cm.data[0].y = 0;
      cm.data[0].hits = {10};
      cat = SkillGameInstance::create_attack_payload(cm, sf.pos, d);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::create_skill_payload(cat, 3001001, ski_lv);
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

static void shuangfeizhan() {
  GameSkill g_skill;
  g_skill.id = u"3001002";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    const GameTriangle tri = {
        {
            SDL_FPoint{-350, -100},
            SDL_FPoint{-350, 100},
            SDL_FPoint{0, -28},
        },
    };
    auto &sf = CharacterGameInstance::self;
    auto WeaponType = EquipGameInstance::load_weapon_type(sf);
    if (!(WeaponType == EquipGameInstance::WeaponType::kBow ||
          WeaponType == EquipGameInstance::WeaponType::kCrossbow)) {
      return;
    }
    auto ball = PackageGameInstance::load_active_ball();
    if (ball == nullptr) {
      return;
    }
    auto ball_id = (*ball)->id;
    CharacterLogicSystem::run_action(sf, u"shoot1");
    auto cm = CharacterLogicSystem::run_attack_check(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
    }
    auto delay = SkillGameInstance::load_ski_time(sf);
    auto pre = ball_id.substr(0, 4);
    std::u16string path = BallGameInstance::load_ball_path(ball_id);
    auto pos = sf.pos;
    pos.y -= 32;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    if (sf.flip) {
      goal.x += 350;
    } else {
      goal.x -= 350;
    }
    auto cct = BallGameInstance::create_ball_payload(cm, pos, goal, delay,
                                                       page, 750, path);
    if (cct.payload->ball->mob_index != UINT32_MAX) {
      if (cct.payload->ball->y2 >= -8) {
        cct.payload->ball->y2 -= 8;
        cm.data[0].y -= 8;
      }
    }

    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::load_ball_time(cct);
      // Create and send attack payload
      cm.data[0].hits = {5, 5};
      cat = SkillGameInstance::create_attack_payload(cm, sf.pos, d);
      ClientRequest::send_to_host(cat);
    }
    for (auto i : {0, 1}) {
      cct.payload->ball->y1 = sf.pos.y - 28;
      cct.payload->ball->y2 += i * 8;
      ClientRequest::send_to_host(cct);
      ServerBallInstance::handle_server_b(cct.payload);
    }
    auto ckt = SkillGameInstance::create_skill_payload(cat, 3001002, ski_lv);
    if (!ckt.payload.empty()) {
      ckt.payload[1]->y += 8;
    }
    ServerCharacterInstance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &skis = SkillGameInstance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  duanhunjian();
  shuangfeizhan();
  return true;
}();