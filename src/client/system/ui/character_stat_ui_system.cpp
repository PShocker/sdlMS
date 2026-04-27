#include "character_stat_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <cstdint>

SDL_FPoint character_stat_ui_system::load_wh() { return {185, 281}; }

static const SDL_FRect detail_rect = {185, 58, 187, 223};

void character_stat_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"CharacterStat.img/Main/backgrnd"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);

  static auto backgrnd_detail = wz_resource::load_texture(
      wz_resource::ui->find(u"CharacterStat.img/Detail/backgrnd"));
  pos_rect = {pos.x + detail_rect.x, pos.y + detail_rect.y,
              static_cast<float>(backgrnd_detail->w),
              static_cast<float>(backgrnd_detail->h)};
  SDL_RenderTexture(window::renderer, backgrnd_detail, nullptr, &pos_rect);
}

bool character_stat_ui_system::render() {
  render_backgrnd();
  return true;
}

void character_stat_ui_system::open() {
  detail = true;

  auto wh = load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void character_stat_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  event_drag_end();
}

void character_stat_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool character_stat_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        event_top();
        event_drag_start(event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
      }
      event_drag_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_drag_move(event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

void character_stat_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void character_stat_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void character_stat_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void character_stat_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto wh = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - wh.x);
    pos.y = std::clamp(pos.y, (float)0, camera.h - wh.y);
  }
  return;
}

bool character_stat_ui_system::cursor_in() {
  auto wh = load_wh();
  SDL_FRect pos_rect{pos.x, pos.y, wh.x, wh.y};
  if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
    return true;
  }
  if (detail) {
    pos_rect.x = pos.x + detail_rect.x;
    pos_rect.y = pos.y + detail_rect.y;
    pos_rect.w = pos.y + detail_rect.w;
    pos_rect.h = pos.y + detail_rect.h;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      return true;
    }
  }
  return false;
}