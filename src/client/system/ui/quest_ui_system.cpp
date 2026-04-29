#include "quest_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>

void quest_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd"));
  static auto backgrnd2 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd2"));
  static auto backgrnd3 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd3"));
  static auto backgrnd4 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd4"));
  static auto backgrnd5 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd5"));
  static auto notice0 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/notice0"));
  static auto notice1 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/notice1"));
  static auto notice2 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/notice2"));
  static auto notice3 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/notice3"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
}

bool quest_ui_system::render() {
  render_backgrnd();
  return true;
}

SDL_FPoint quest_ui_system::load_wh() { return {209, 289}; }

void quest_ui_system::open() {
  auto wh = load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void quest_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void quest_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void quest_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void quest_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void quest_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void quest_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool quest_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool quest_ui_system::event(SDL_Event *event) {
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