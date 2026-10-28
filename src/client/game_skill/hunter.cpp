
#include "src/client/game/game_character.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <vector>

namespace sdlms {

// 310
static void Baozhajian() {
  GameSkill g_skill;
  g_skill.id = u"3101005";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    GameTriangle tri = {
        {
            SDL_FPoint{-400, -100},
            SDL_FPoint{-400, 100},
            SDL_FPoint{0, -30},
        },
    };
    auto& sf = CharacterGameInstance::self_;
    CharacterLogicSystem::RunAttackAction(sf, true);
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, tri);
    if (!cm.data.empty()) {
      cm.data = {cm.data[0]};
      cm.data[0].hits = {60};
    }
    auto delay = SkillGameInstance::LoadSkiTime(sf);
    auto ski_lvl2 = std::to_string(ski_lv);
    std::u16string path = u"310.img/skill/3101004/ball";
    auto pos = sf.pos;
    pos.y -= 30;
    auto page = sf.page;
    SDL_FPoint goal = pos;
    if (sf.flip) {
      goal.x += 400;
    } else {
      goal.x -= 400;
    }
    auto cct = BallGameInstance::CreateBallPayload(cm, pos, goal, delay, page,
                                                   700, path);
    ClientRequest::send_to_host(cct);
    ServerBallInstance::HandleServerB(cct.payload);

    fbs::ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto d = BallGameInstance::LoadBallTime(cct);
      auto mob_pos = cm.data[0].mob.pos;
      auto g_r = SkillGameInstance::LoadSkiR(u"3101005", mob_pos, sf.flip);
      cm = CharacterLogicSystem::RunAttackCheck(mob_pos, g_r, 2);
      // Create and send attack payload
      for (int i = 0; i < cm.data.size(); i++) {
        auto& data = cm.data[i];
        if (i == 0) {
          data.hits = {100};
        } else {
          data.hits = {80};
          data.delay = 100;
        }
      }
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, d);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 3101005, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);

    fbs::ClientMobDebuffT cmb;
    cmb.map_id = SceneSystemInstance::map_id_;
    for (int i = 0; i < cm.data.size(); i++) {
      auto& data = cm.data[i];
      fbs::MobDebuffT md;
      md.mob_index = data.mob.index;
      md.payload->val = static_cast<int64_t>(GameDebuff::DebuffType::kDizz);
      md.payload->delay = Window::dt_time_;
      md.payload->time = Window::dt_time_ + 5000;
    }
    if (!cmb.payload.empty()) {
      ClientRequest::send_to_host(cmb);
    }
  };
  auto& Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

static void Wuyingjian() {
  GameSkill g_skill;
  g_skill.climb = true;
  g_skill.id = u"3101003";
  g_skill.end = []() {
    auto& ski = SkillGameInstance::ski_;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill& s) { return s.id == u"3101003"; });
    if (it != ski.end()) {
      ski.erase(it);
    }
  };

  g_skill.action = [](GameSkill* s, std::u16string ski_id) {
    auto b = PackageGameInstance::LoadActiveBall();
    if (b == nullptr) {
      return;
    }
    if (ski_id.empty()) {
      auto& sf = CharacterGameInstance::self_;
      if (sf.action == u"shoot1") {
        ItemGameInstance::AddItemNum(*b, 1);
      }
    } else {
      auto ski_lv = JobSkillGameInstance::LoadSkiLevel(ski_id);
      auto num = SkillGameInstance::LoadSkiBallNum(ski_id, ski_lv);
      ItemGameInstance::AddItemNum(*b, num);
    }
    return;
  };

  g_skill.frame = []() { return; };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto& sf = CharacterGameInstance::self_;
    auto action_type = CharacterLogicSystem::LoadActionType(sf);
    if (action_type != CharacterLogicSystem::ActionEnum::kClimb) {
      CharacterLogicSystem::RunAction(sf, u"alert2");
    } else {
      CharacterLogicSystem::self_climb_animate_cooldown_ =
          Window::dt_now_ + 700;
    }
    auto& ski = SkillGameInstance::ski_;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 30 * 1000;
    g_skill.destroy = Window::dt_now_ + g_skill.duration;
    ski.push_back(g_skill);

    SkillGameInstance::Skis()[u"3101003"].cd = Window::dt_now_ + 1000;

    auto ckt = SkillGameInstance::CreateSkillPayload(3101003, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };

  auto& Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  Baozhajian();
  Wuyingjian();
  return true;
}();

}  // namespace sdlms
