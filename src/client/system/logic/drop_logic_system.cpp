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
    const auto PICK_UP_DURATION = 700;
    int duration = window::dt_now - drop.pick_time;
    if (duration <= PICK_UP_DURATION) {
      // alpha
      if (duration < 420) {
        drop.alpha = 255;
      } else {
        drop.alpha = (192 * (420 - duration) / 280.0f + 255);
      }
      // pos
      const auto &picker = drop.picker;
      std::optional<SDL_FPoint> picker_pos;
      if (picker->pet_index.has_value()) {

      } else {
        auto client_id = picker->client_id;
        if (client_id == 0) {
          picker_pos = character_game_instance::self.pos;
        } else {
          const auto &others = character_game_instance::others;
          if (others.contains(client_id)) {
            picker_pos = others.at(client_id).g_character.pos;
          }
        }
      }
      if (picker_pos.has_value()) {
        float x = (picker_pos->x * duration +
                   drop.goal.x * (PICK_UP_DURATION - duration)) /
                  PICK_UP_DURATION;
        float y = (picker_pos->y * duration +
                   drop.goal.y * (PICK_UP_DURATION - duration)) /
                      PICK_UP_DURATION +
                  160 * SDL_powf((duration - 350.0f) / PICK_UP_DURATION, 2) -
                  40;
        drop.pos = {x, y};
      } else {
        drop.alpha = 0;
      }
    } else {
      drop.alpha = 0;
    }
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