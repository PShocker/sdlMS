
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/effect_logic_system.h"
#include "src/client/system/render/effect_render_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/physic/physic.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include <algorithm>
#include <cstdint>
#include <string>
namespace sdlms {


static void Shunjianyidong() {
  GameSkill g_skill;
  g_skill.id = u"2301000";

  g_skill.effect = [](SDL_FPoint p, GameEffect *e, bool f) {
    e->id = u"BasicEff.img/Teleport";
    e->lv = 1;
    e->pos = SDL_FPoint{0, 0};
    if (EffectLogicSystem::RunEffect(*e)) {
      return false;
    }
    auto g_character = std::any_cast<GameCharacter *>(e->data);
    EffectRenderSystem::RenderEffect(g_character->pos, *e);
    e->id = u"2301000";
    return true;
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) {
    CharacterStatGameInstance::AddExp(1000);
    SkillGameInstance::Skis()[u"2301000"].cd = Window::dt_now_ + 500;
    auto &sf = CharacterGameInstance::self_;

    auto up = KeyboardGameInstance::FindKeyByVal("up");
    auto down = KeyboardGameInstance::FindKeyByVal("down");
    auto left = KeyboardGameInstance::FindKeyByVal("left");
    auto right = KeyboardGameInstance::FindKeyByVal("right");

    auto x = sf.pos.x;
    auto y = sf.pos.y;

    if (Window::keyboard_state_[right]) {
      x += 200;
    } else if (Window::keyboard_state_[left]) {
      x -= 200;
    } else if (Window::keyboard_state_[down]) {
      y += 200;
    } else if (Window::keyboard_state_[up]) {
      y -= 200;
    }

    const auto &fhs = FootholdGameInstance::data_;
    auto border =
        MapInfoGameInstance::LoadMrBorder(SceneSystemInstance::map_id_);
    auto [l, t, r, b] = border;
    x = std::clamp(x, l, r);
    y = std::clamp(y, t, b);
    if (y != sf.pos.y) {
      // 垂直移动
      auto ins = Physic::FallIntersectPos(sf.pos, {x, y}, fhs);
      if (!ins.empty()) {
        if (sf.pos.y < y) {
          sf.pos = ins.rbegin()->second.pos;
          CharacterLogicSystem::self_fh_ = ins.rbegin()->second.fh.id;
        } else {
          sf.pos = ins.begin()->second.pos;
          CharacterLogicSystem::self_fh_ = ins.begin()->second.fh.id;
        }
      }
    } else if (x != sf.pos.x) {
      auto ins = Physic::FallIntersectPos({x, y}, {x, y + 70}, fhs);
      if (!ins.empty()) {
        auto top = ins.begin()->first;
        sf.pos = ins.begin()->second.pos;
        CharacterLogicSystem::self_fh_ = ins.begin()->second.fh.id;
      } else {
        auto ins = Physic::FallIntersectPos({x, y}, {x, y - 70}, fhs);
        if (!ins.empty()) {
          auto top = ins.rbegin()->first;
          sf.pos = ins.rbegin()->second.pos;
          CharacterLogicSystem::self_fh_ = ins.begin()->second.fh.id;
        }
      }
    }

    CharacterLogicSystem::self_hspeed_ = 0;
    CharacterLogicSystem::self_vspeed_ = 0;

    auto ckt = SkillGameInstance::CreateSkillPayload(2301000, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

static void Huixueshu() {
  GameSkill g_skill;
  g_skill.climb = true;
  g_skill.id = u"2301001";

  static uint64_t now;
  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    SkillGameInstance::Skis()[u"2301001"].cd = Window::dt_now_ + 1000;

    auto &ski = SkillGameInstance::ski_;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 30 * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    ski.push_back(g_skill);

    auto &sf = CharacterGameInstance::self_;
    auto ckt = SkillGameInstance::CreateSkillPayload(1001, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };

  auto &Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

static void Qidao() {
  GameSkill g_skill;
  g_skill.climb = true;
  g_skill.id = u"2301003";
  g_skill.end = []() {
    auto &ski = SkillGameInstance::ski_;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill &s) { return s.id == u"2301003"; });
    if (it != ski.end()) {
      ski.erase(it);
      CharacterLogicSystem::self_hspeed_max_ -= 100;
      CharacterLogicSystem::self_hspeed_min_ += 100;
    }
  };

  g_skill.frame = []() { return; };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto &ski = SkillGameInstance::ski_;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 30 * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    ski.push_back(g_skill);

    CharacterLogicSystem::self_hspeed_max_ += 100;
    CharacterLogicSystem::self_hspeed_min_ -= 100;

    if (client_id != 0) {
      return;
    }

    SkillGameInstance::Skis()[u"2301003"].cd = Window::dt_now_ + 1000;

    auto &sf = CharacterGameInstance::self_;
    auto g_r = SkillGameInstance::LoadSkiR(u"2301003");
    auto players = CharacterLogicSystem::RunBuffCheck(sf, g_r);
    auto ckt =
        SkillGameInstance::CreateSkillPayload(players, 2301003, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };

  auto &Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  Shunjianyidong();
  return true;
}();

}  // namespace sdlms
