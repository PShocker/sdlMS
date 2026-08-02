#include "skill_buff_ui_system.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "package_ui_system.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system_instance/fade_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "tooltip_ui_system.h"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>

static std::optional<game_skill> mouse_ski;

void skill_buff_ui_system::render_ui(game_skill &sk, float x, float y) {
  auto id = sk.id;
  auto ski_node = skill_game_instance::load_ski_node(id);
  auto icon = wz_resource::load_texture(ski_node->get_child(u"icon"));
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(window::renderer, icon, NULL, &pos_rect);
  auto &mouse_pos = window::mouse_pos;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    mouse_ski = sk;
  }
  if (sk.destroy‌) {
    SDL_SetRenderDrawBlendMode(window::renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(window::renderer, 0, 0, 0, 100);
    auto d = sk.destroy‌ - window::dt_now;
    pos_rect.x = x;
    pos_rect.h = 32 * (1 - (d / (float)sk.duration));
    pos_rect.y = y + 32 - pos_rect.h;
    pos_rect.w = 32;
    SDL_RenderFillRect(window::renderer, &pos_rect);
    // 渲染冷却时间
    auto num = d / 1000;
    package_ui_system::render_number(num, x, y + 21);
  }
}

bool skill_buff_ui_system::render_ui_info() {
  if (!mouse_ski.has_value()) {
    return true;
  }
  if (cursor_game_instance::cursor_ui != nullptr) {
    return true;
  }
  auto ski = mouse_ski.value();
  // 宽度固定330
  const auto w = 330;
  auto &mouse_pos = window::mouse_pos;
  const auto &camera = camera_game_instance::camera;
  auto x = 0;
  if (mouse_pos.x + w <= camera.w) {
    x = mouse_pos.x;
  } else {
    x = camera.w - 330;
  }
  tooltip_ui_system::render_skill(ski.id, ski.lv, x, mouse_pos.y);
  return true;
}

void skill_buff_ui_system::render_ui() {
  // ski buff
  const auto &camera = camera_game_instance::camera;
  for (uint32_t i = 0; i < skill_game_instance::ski.size(); i++) {
    auto col = i % 5;
    auto row = i / 5;
    auto x = camera.w - 32 - col * 32;
    auto y = row * 32;
    render_ui(skill_game_instance::ski[i], x, y);
  }
}

void skill_buff_ui_system::event_motion(SDL_Event *event) {
  auto &sys = system::render_systems;
  auto it = std::ranges::find(sys, &render_ui_info);
  if (it != sys.end()) {
    sys.erase(it);
  }
  if (!mouse_ski.has_value()) {
    return;
  }
  it = std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render_ui_info);
  }
}

bool skill_buff_ui_system::render() {
  mouse_ski = std::nullopt;
  render_ui();
  return true;
}

bool skill_buff_ui_system::event(SDL_Event *event) {
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_RIGHT) {
      if (cursor_game_instance::cursor_ui == nullptr) {
        if (mouse_ski.has_value()) {
          mouse_ski->end();
        }
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_motion(event);
    break;
  }
  default: {
    break;
  }
  }

  return true;
}