
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_ball.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_skill.h"
#include "src/client/game/game_triangle.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/effect_logic_system.h"
#include "src/client/system/render/effect_render_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"
#include <array>
#include <cstdint>
#include <memory>
#include <numbers>
#include <string>
#include <vector>

static void Douqijizhong() {
  GameSkill g_skill;
  g_skill.climb = true;
  g_skill.id = u"1111000";

  struct DataPack {
    GameCharacter *c;
    std::array<float, 5> zoom = {0, 0, 0, 0, 0};
  };

  const static auto end_ski = [](GameCharacter *c) {
    if (c == nullptr) {
      auto &ccs = CharacterLogicSystem::ccs;
      StateT st;
      st.state = StateEnum_BUFF_SKILL;
      st.val = 1111000;
      st.sub_val = 0;
      ccs.payload.push_back(std::make_unique<StateT>(st));
      c = &CharacterGameInstance::self;
    }
    auto &eff = c->effect;
    std::erase_if(eff, [&](const auto &ef) { return ef.id == u"1111000"; });
  };

  g_skill.end = []() {
    auto &ski = SkillGameInstance::ski;
    auto it = std::ranges::find_if(
        ski, [](const GameSkill &s) { return s.id == u"1111000"; });
    if (it != ski.end()) {
      ski.erase(it);
      end_ski(nullptr);
    }
  };

  g_skill.frame = []() { return; };

  g_skill.state = [](GameCharacter *c, int index) {
    if (index > 6) {
      return;
    }
    auto &eff = c->effect;
    bool find_ski = false;
    if (index > 1) {
      for (auto &e : eff) {
        if (e.id == u"1111000") {
          find_ski = true;
          const std::type_info &tp = e.data.type();
          if (tp == typeid(GameCharacter *) && index == 2) {
            DataPack dp;
            dp.c = c;
            dp.zoom[0] = 3;
            e.data = dp;
            break;
          }
          auto dp = std::any_cast<DataPack>(&e.data);
          dp->zoom[index - 2] = 3;
          break;
        }
      }
      if (!find_ski) {
        GameEffect e = {
            .id = u"1111000",
            .index = 0,
            .time = 0,
            .delay = Window::dt_now,
            .lv = 1,
            .type = GameEffect::EffectType::kSkillCustom,
            .pos = std::nullopt,
            .z = false,
            .data = c,
        };
        DataPack dp;
        dp.c = c;
        std::ranges::fill_n(dp.zoom.begin(), index - 1, 1.0f);
        e.data = dp;
        eff.push_back(e);
      }
    } else {
      GameEffect e = {
          .id = u"1111000",
          .index = 0,
          .time = 0,
          .delay = Window::dt_now,
          .lv = 1,
          .type = GameEffect::EffectType::kSkillCustom,
          .pos = std::nullopt,
          .z = false,
          .data = c,
      };
      eff.push_back(e);
    }
    if (c == &CharacterGameInstance::self) {
      auto &ccs = CharacterLogicSystem::ccs;
      StateT st;
      st.state = StateEnum_BUFF_SKILL;
      st.val = 1111000;
      st.sub_val = index;
      ccs.payload.push_back(std::make_unique<StateT>(st));
    }
  };

  g_skill.action = [g_skill](GameSkill *s, std::u16string ski_id) {
    if (ski_id.empty()) {
      return;
    }
    auto &sf = CharacterGameInstance::self;
    auto &eff = sf.effect;
    for (auto &e : eff) {
      if (e.id == u"1111000") {
        const std::type_info &tp = e.data.type();
        if (tp == typeid(GameCharacter *)) {
          break;
        }
        auto dp = std::any_cast<DataPack>(&e.data);
        int i = 0;
        for (auto &f : dp->zoom) {
          if (f == 0) {
            break;
          }
          i++;
        }
        g_skill.state(&sf, i + 2);
        return;
      }
    }
    g_skill.state(&sf, 2);
    return;
  };

  g_skill.effect = [](SDL_FPoint p, GameEffect *e, bool f) {
    GameCharacter *g_character;
    DataPack *dp;
    const std::type_info &tp = e->data.type();
    if (tp == typeid(GameCharacter *)) {
      g_character = std::any_cast<GameCharacter *>(e->data);
    } else {
      dp = std::any_cast<DataPack>(&e->data);
      g_character = dp->c;
    }
    if (e->lv == 0) {
      e->type = GameEffect::EffectType::kSkillUse;
      auto origin_data = e->data;
      e->data = g_character;
      if (EffectLogicSystem::RunSkillUse(*e)) {
        e->lv = 1;
      } else {
        EffectRenderSystem::RenderSkiUse(g_character->pos, *e,
                                             g_character->flip);
      }
      e->type = GameEffect::EffectType::kSkillCustom;
      e->data = origin_data;
    }
    static auto t = WzResource::LoadTexture(
        WzResource::skill->find(u"111.img/skill/1111000/special/0"));

    auto x = g_character->pos.x - CameraGameInstance::camera.x;
    auto y = g_character->pos.y - CameraGameInstance::camera.y - 28;
    const auto origin = SDL_FPoint{36, 37};
    SDL_FRect pos_rect{
        x - origin.x,
        y - origin.y,
        static_cast<float>(t->w),
        static_cast<float>(t->h),
    };
    if (g_character->flip == 1) {
      pos_rect.x = x;
      pos_rect.x = (pos_rect.x - (t->w - origin.x));
    }
    SDL_RenderTextureRotated(Window::renderer, t, nullptr, &pos_rect,
                             Window::dt_now / 10, nullptr, (SDL_FlipMode)0);
    if (dp) {
      static auto node =
          WzResource::skill->find(u"111.img/skill/1111000/state");
      auto &z = dp->zoom;
      const int count = static_cast<int>(z.size());
      for (int i = 0; i < count; i++) {
        auto texture_node = node->get_child(std::to_string(i));
        auto t = WzResource::LoadTexture(texture_node);
        auto origin =
            WzResource::LoadFpoint(texture_node->get_child(u"origin"));

        // 每个 state 的角度：均匀分布 + 随时间旋转
        float angle =
            (2.0f * std::numbers::pi / count) * i + Window::dt_now / 800.0f;

        // 围绕原点 (x, y) 的圆周位置
        const float radius = 36.0f;
        float cx = x + radius * std::cos(angle);
        float cy = y + radius * std::sin(angle);

        auto &scale = z[i];
        if (scale == 0) {
          continue;
        }
        // 以 cx,cy 为中心摆放贴图（origin 是贴图的旋转/中心锚点）
        SDL_FRect pos_rect{
            cx - origin.x * scale,
            cy - origin.y * scale,
            static_cast<float>(t->w * scale),
            static_cast<float>(t->h * scale),
        };

        SDL_RenderTextureRotated(Window::renderer, t, nullptr, &pos_rect, angle,
                                 nullptr, (SDL_FlipMode)0);
        scale -= 0.08;
        scale = std::max(scale, 1.0f);
      }
    }

    return true;
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto &ski = SkillGameInstance::ski;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 0;
    g_skill.destroy = 0;
    g_skill.start = 0;
    ski.push_back(g_skill);

    SkillGameInstance::Skis()[u"1111000"].cd = Window::dt_now + 1000;

    auto &sf = CharacterGameInstance::self;
    auto ckt = SkillGameInstance::CreateSkillPayload(1111000, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);

    CharacterLogicSystem::RunAction(sf, u"alert4");
    auto &ccs = CharacterLogicSystem::ccs;
    StateT st;
    st.state = StateEnum_BUFF_SKILL;
    st.val = 1111000;
    st.sub_val = 1;
    ccs.payload.push_back(std::make_unique<StateT>(st));
  };

  auto &Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

static void Hupaoxiao() {
  GameSkill g_skill;
  g_skill.id = u"1111003";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = CharacterGameInstance::self;
    CharacterLogicSystem::RunAction(sf, u"alert3");
    auto g_r = SkillGameInstance::LoadSkiR(u"1111003");
    auto cm = CharacterLogicSystem::RunAttackCheck(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = SkillGameInstance::LoadSkiTime(sf);
      // Create and send attack payload
      for (auto &data : cm.data) {
        data.hits = {30, 30};
      }
      cat = SkillGameInstance::CreateAttackPayload(cm, sf.pos, delay);
      ClientRequest::send_to_host(cat);
    }
    auto ckt = SkillGameInstance::CreateSkillPayload(cat, 1111003, ski_lv);
    ServerCharacterInstance::HandleSki(ckt.ski_id, ski_lv, ckt.payload, 0);
    ClientRequest::send_to_host(ckt);
  };
  auto &Skis = SkillGameInstance::Skis();
  Skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  Douqijizhong();
  Hupaoxiao();
  return true;
}();