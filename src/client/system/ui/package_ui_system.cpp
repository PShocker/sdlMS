#include "package_ui_system.h"
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

void package_ui_system::render_backgrnd() {
  static auto texture =
      wz_resource::load_texture(wz_resource::ui->find(u"Item.img/backgrnd"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

void package_ui_system::render_tab() {
  const static std::array tab_pos = {
      SDL_FPoint{5, 24},   //
      SDL_FPoint{38, 24},  //
      SDL_FPoint{71, 24},  //
      SDL_FPoint{104, 24}, //
      SDL_FPoint{137, 24}, //
      SDL_FPoint{170, 24}, //
  };
  const static auto tab_node = wz_resource::ui->find(u"Item.img/Tab");
  const static std::array active_texture = {
      wz_resource::load_texture(tab_node->find(u"enabled/0")),
      wz_resource::load_texture(tab_node->find(u"enabled/1")),
      wz_resource::load_texture(tab_node->find(u"enabled/2")),
      wz_resource::load_texture(tab_node->find(u"enabled/3")),
      wz_resource::load_texture(tab_node->find(u"enabled/4")),
      wz_resource::load_texture(tab_node->find(u"enabled/5")),
  };
  const static std::array disabled_texture = {
      wz_resource::load_texture(tab_node->find(u"disabled/0")),
      wz_resource::load_texture(tab_node->find(u"disabled/1")),
      wz_resource::load_texture(tab_node->find(u"disabled/2")),
      wz_resource::load_texture(tab_node->find(u"disabled/3")),
      wz_resource::load_texture(tab_node->find(u"disabled/4")),
      wz_resource::load_texture(tab_node->find(u"disabled/5")),
  };
  for (uint8_t i = 0; i < tab_pos.size(); i++) {
    SDL_Texture *t = active_tab == i ? active_texture[i] : disabled_texture[i];
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos[i].x)),
                       static_cast<float>(int(pos.y + tab_pos[i].y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  }
}

void package_ui_system::render_items() {
  if (active_tab == 0) {
    const auto &equips = package_game_instance::equips;
  } else {
  }
}

bool package_ui_system::render() {
  render_backgrnd();
  render_tab();
  return true;
}

SDL_FPoint package_ui_system::load_wh() { return {209, 289}; }

void package_ui_system::open() {
  auto wh = load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void package_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void package_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  std::erase(system::logic_systems, run);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
  system::logic_systems.push_back(run);
}

void package_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void package_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void package_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void package_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool package_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void package_ui_system::event_tab(SDL_Event *event) {
  const static std::array tab_rect = {
      SDL_FRect{5, 24, 33, 19},   //
      SDL_FRect{38, 24, 33, 19},  //
      SDL_FRect{71, 24, 33, 19},  //
      SDL_FRect{104, 24, 33, 19}, //
      SDL_FRect{137, 24, 33, 19}, //
      SDL_FRect{170, 24, 33, 19}, //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      active_tab = i;
    }
  }
}

bool package_ui_system::event(SDL_Event *event) {
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
        event_tab(event);
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

bool package_ui_system::run() { return true; }