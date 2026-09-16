
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

static void douqijizhong() {
  game_skill g_skill;
  g_skill.climb = true;
  g_skill.id = u"1111000";

  struct data_pack {
    game_character *c;
    std::array<float, 5> zoom = {0, 0, 0, 0, 0};
  };

  const static auto end_ski = [](game_character *c) {
    if (c == nullptr) {
      auto &ccs = character_logic_system::ccs;
      StateT st;
      st.state = StateEnum_BUFF_SKILL;
      st.val = 1111000;
      st.sub_val = 0;
      ccs.payload.push_back(std::make_unique<StateT>(st));
      c = &character_game_instance::self;
    }
    auto &eff = c->effect;
    std::erase_if(eff, [&](const auto &ef) { return ef.id == u"1111000"; });
  };

  g_skill.end = []() {
    auto &ski = skill_game_instance::ski;
    auto it = std::ranges::find_if(
        ski, [](const game_skill &s) { return s.id == u"1111000"; });
    if (it != ski.end()) {
      ski.erase(it);
      end_ski(nullptr);
    }
  };

  g_skill.frame = []() { return; };

  g_skill.state = [](game_character *c, int index) {
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
          if (tp == typeid(game_character *) && index == 2) {
            data_pack dp;
            dp.c = c;
            dp.zoom[0] = 3;
            e.data = dp;
            break;
          }
          auto dp = std::any_cast<data_pack>(&e.data);
          dp->zoom[index - 2] = 3;
          break;
        }
      }
      if (!find_ski) {
        game_effect e = {
            .id = u"1111000",
            .index = 0,
            .time = 0,
            .delay = window::dt_now,
            .lv = 1,
            .type = game_effect::effect_type::skill_custom,
            .pos = std::nullopt,
            .z = false,
            .data = c,
        };
        data_pack dp;
        dp.c = c;
        std::ranges::fill_n(dp.zoom.begin(), index - 1, 1.0f);
        e.data = dp;
        eff.push_back(e);
      }
    } else {
      game_effect e = {
          .id = u"1111000",
          .index = 0,
          .time = 0,
          .delay = window::dt_now,
          .lv = 1,
          .type = game_effect::effect_type::skill_custom,
          .pos = std::nullopt,
          .z = false,
          .data = c,
      };
      eff.push_back(e);
    }
    if (c == &character_game_instance::self) {
      auto &ccs = character_logic_system::ccs;
      StateT st;
      st.state = StateEnum_BUFF_SKILL;
      st.val = 1111000;
      st.sub_val = index;
      ccs.payload.push_back(std::make_unique<StateT>(st));
    }
  };

  g_skill.action = [g_skill](game_skill *s, std::u16string ski_id) {
    if (ski_id.empty()) {
      return;
    }
    auto &sf = character_game_instance::self;
    auto &eff = sf.effect;
    for (auto &e : eff) {
      if (e.id == u"1111000") {
        const std::type_info &tp = e.data.type();
        if (tp == typeid(game_character *)) {
          break;
        }
        auto dp = std::any_cast<data_pack>(&e.data);
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

  g_skill.effect = [](SDL_FPoint p, game_effect *e, bool f) {
    game_character *g_character;
    data_pack *dp;
    const std::type_info &tp = e->data.type();
    if (tp == typeid(game_character *)) {
      g_character = std::any_cast<game_character *>(e->data);
    } else {
      dp = std::any_cast<data_pack>(&e->data);
      g_character = dp->c;
    }
    if (e->lv == 0) {
      e->type = game_effect::effect_type::skill_use;
      auto origin_data = e->data;
      e->data = g_character;
      if (effect_logic_system::run_skill_use(*e)) {
        e->lv = 1;
      } else {
        effect_render_system::render_ski_use(g_character->pos, *e,
                                             g_character->flip);
      }
      e->type = game_effect::effect_type::skill_custom;
      e->data = origin_data;
    }
    static auto t = wz_resource::load_texture(
        wz_resource::skill->find(u"111.img/skill/1111000/special/0"));

    auto x = g_character->pos.x - camera_game_instance::camera.x;
    auto y = g_character->pos.y - camera_game_instance::camera.y - 28;
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
    SDL_RenderTextureRotated(window::renderer, t, nullptr, &pos_rect,
                             window::dt_now / 10, nullptr, (SDL_FlipMode)0);
    if (dp) {
      static auto node =
          wz_resource::skill->find(u"111.img/skill/1111000/state");
      auto &z = dp->zoom;
      const int count = static_cast<int>(z.size());
      for (int i = 0; i < count; i++) {
        auto texture_node = node->get_child(std::to_string(i));
        auto t = wz_resource::load_texture(texture_node);
        auto origin =
            wz_resource::load_fpoint(texture_node->get_child(u"origin"));

        // 每个 state 的角度：均匀分布 + 随时间旋转
        float angle =
            (2.0f * std::numbers::pi / count) * i + window::dt_now / 800.0f;

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

        SDL_RenderTextureRotated(window::renderer, t, nullptr, &pos_rect, angle,
                                 nullptr, (SDL_FlipMode)0);
        scale -= 0.08;
        scale = std::max(scale, 1.0f);
      }
    }

    return true;
  };

  g_skill.use = [g_skill](uint64_t client_id, int ski_lv) mutable {
    auto &ski = skill_game_instance::ski;
    g_skill.end();
    g_skill.lv = ski_lv;
    g_skill.duration = 0;
    g_skill.destroy = 0;
    g_skill.start = 0;
    ski.push_back(g_skill);

    skill_game_instance::skis()[u"1111000"].cd = window::dt_now + 1000;

    auto &sf = character_game_instance::self;
    auto ckt = skill_game_instance::create_skill_payload(1111000, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);

    character_logic_system::run_action(sf, u"alert4");
    auto &ccs = character_logic_system::ccs;
    StateT st;
    st.state = StateEnum_BUFF_SKILL;
    st.val = 1111000;
    st.sub_val = 1;
    ccs.payload.push_back(std::make_unique<StateT>(st));
  };

  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

static void hupaoxiao() {
  game_skill g_skill;
  g_skill.id = u"1111003";
  g_skill.use = [](uint64_t client_id, int ski_lv) {
    auto &sf = character_game_instance::self;
    character_logic_system::run_action(sf, u"alert3");
    auto g_r = skill_game_instance::load_ski_r(u"1111003");
    auto cm = character_logic_system::run_attack_check(sf, g_r);
    ClientCharacterAttackT cat;
    if (!cm.data.empty()) {
      auto delay = skill_game_instance::load_ski_time(sf);
      // Create and send attack payload
      for (auto &data : cm.data) {
        data.hits = {30, 30};
      }
      cat = skill_game_instance::create_attack_payload(cm, sf.pos, delay);
      client_request::send_to_host(cat);
    }
    auto ckt = skill_game_instance::create_skill_payload(cat, 1111003, ski_lv);
    server_character_instance::handle_ski(ckt.ski_id, ski_lv, ckt.payload, 0);
    client_request::send_to_host(ckt);
  };
  auto &skis = skill_game_instance::skis();
  skis[g_skill.id] = g_skill;
}

[[maybe_unused]] static const bool r = [] {
  douqijizhong();
  hupaoxiao();
  return true;
}();