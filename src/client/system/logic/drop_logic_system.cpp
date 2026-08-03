#include "drop_logic_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_drop.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/window/window.h"
#include <algorithm>
#include <cmath>
#include <ranges>

void drop_logic_system::run_state_machine(game_drop &drop) {
  switch (drop.type) {
  case game_drop::drop_enum::fly: {
    auto delta_time = window::delta_time / 1000.0;
    drop.vspeed = drop.vspeed + delta_time * 800;
    drop.vspeed = std::min(drop.vspeed, 400.0f);
    auto dy = drop.vspeed * delta_time;
    drop.pos.y += dy;
    if (drop.vspeed <= 0) {
      auto dx = drop.hspeed * delta_time;
      drop.pos.x += dx;
    }
    if (drop.pos.y >= drop.goal.y && drop.vspeed > 0) {
      drop.pos.x = drop.goal.x;
      drop.pos.y = drop.goal.y;
      drop.type = game_drop::drop_enum::land;
      drop.vspeed = 0;
      drop.hspeed = 0;
      drop.rotate = 0;
      break;
    }
    if (drop.data->id != u"00000000") {
      drop.rotate += (float)window::delta_time;
    }
    break;
  }
  case game_drop::drop_enum::land: {
    drop.hspeed += window::delta_time;
    double t = fmod((double)drop.hspeed / 3500, 1.0);
    auto dy = (1.0 - fabs(2.0 * t - 1.0)) * 8.0 - 4.0;
    drop.pos.y = drop.goal.y + dy;
    break;
  }
  case game_drop::drop_enum::pick: {
    constexpr auto PICK_UP_DURATION = 700;
    constexpr auto ALPHA_FADE_START = 420;
    constexpr auto ALPHA_FADE_DURATION =
        PICK_UP_DURATION - ALPHA_FADE_START; // 280
    constexpr auto PARABOLA_PEAK_DELTA = 40; // 最高点偏移
    constexpr auto PARABOLA_PEAK_TIME = 350;
    constexpr auto PARABOLA_SCALE = 160;

    int elapsed = window::dt_now - drop.pick_time;
    bool is_animating = elapsed <= PICK_UP_DURATION;

    if (!is_animating) {
      drop.alpha = 0;
      break;
    }

    // 计算透明度
    if (elapsed < ALPHA_FADE_START) {
      drop.alpha = 255;
    } else {
      float fade_progress =
          static_cast<float>(elapsed - ALPHA_FADE_START) / ALPHA_FADE_DURATION;
      drop.alpha = static_cast<uint8_t>(255 * (1.0f - fade_progress * 0.75f));
    }

    // 获取拾取者位置
    const auto &picker = drop.picker;
    std::optional<SDL_FPoint> picker_pos;

    if (picker->pet_index.has_value()) {
    } else {
      auto client_id = picker->client_id;
      if (client_id == 0) {
        picker_pos = character_game_instance::self.pos;
      } else if (auto it = character_game_instance::others.find(client_id);
                 it != character_game_instance::others.end()) {
        picker_pos = it->second.g_character.pos;
      }
    }

    if (!picker_pos.has_value()) {
      drop.alpha = 0;
      break;
    }

    // 计算位置插值（带抛物线弧线效果）
    float progress = static_cast<float>(elapsed) / PICK_UP_DURATION;
    float inverse_progress = 1.0f - progress;

    // 线性插值位置
    float x = picker_pos->x * progress + drop.goal.x * inverse_progress;

    // Y轴带抛物线弧线：基础线性插值 + 抛物线偏移
    float parabola_offset =
        PARABOLA_SCALE * SDL_powf((elapsed - PARABOLA_PEAK_TIME) /
                                      static_cast<float>(PICK_UP_DURATION),
                                  2) -
        PARABOLA_PEAK_DELTA;
    float y = (picker_pos->y * progress + drop.goal.y * inverse_progress) +
              parabola_offset;
    drop.pos = {x, y};
    break;
  }
  case game_drop::drop_enum::fade: {
    break;
  }
  }
  return;
}

void drop_logic_system::run_destroy() {
  std::erase_if(drop_game_instance::data, [](const auto &pair) {
    const auto &[id, drop] = pair;
    return drop.alpha == 0;
  });
}

bool drop_logic_system::run() {
  for (auto &drop : drop_game_instance::data | std::views::values) {
    run_state_machine(drop);
  }
  run_destroy();
  return true;
}