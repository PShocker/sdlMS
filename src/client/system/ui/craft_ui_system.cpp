#include "craft_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>

void craft_ui_system::render_backgrnd() {
  static auto texture = wz_resource::load_texture(
      wz_resource::ui->find(u"Crafting.img/backgrnd"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

void craft_ui_system::render_tab() {
  const static SDL_FPoint lt = {9, 23};
  const static SDL_FPoint rb = {537, 44};
  const static auto tab_node =
      wz_resource::ui->find(u"Crafting.img/tab:category");
  const static std::array selected_texture = {
      wz_resource::load_texture(tab_node->find(u"selected/0")),
      wz_resource::load_texture(tab_node->find(u"selected/1")),
      wz_resource::load_texture(tab_node->find(u"selected/2")),
      wz_resource::load_texture(tab_node->find(u"selected/3")),
      wz_resource::load_texture(tab_node->find(u"selected/4")),
      wz_resource::load_texture(tab_node->find(u"selected/5")),
  };
  const static std::array normal_texture = {
      wz_resource::load_texture(tab_node->find(u"normal/0")),
      wz_resource::load_texture(tab_node->find(u"normal/1")),
      wz_resource::load_texture(tab_node->find(u"normal/2")),
      wz_resource::load_texture(tab_node->find(u"normal/3")),
      wz_resource::load_texture(tab_node->find(u"normal/4")),
      wz_resource::load_texture(tab_node->find(u"normal/5")),
  };
  for (uint8_t i = 0; i < selected_texture.size(); i++) {
    SDL_Texture *t =
        selected_tab == i ? selected_texture[i] : normal_texture[i];
    SDL_FPoint tab_pos = {static_cast<float>(9 + i * 88), 23};
    SDL_FRect pos_rect{pos.x + tab_pos.x, pos.y + tab_pos.y,
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  }
}

bool craft_ui_system::render() {
  render_backgrnd();
  render_tab();
  return true;
}

SDL_FPoint craft_ui_system::load_wh() { return {545, 409}; }

void craft_ui_system::open() {
  auto wh = load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void craft_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void craft_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void craft_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 18};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void craft_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void craft_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void craft_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool craft_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void craft_ui_system::event_tab(SDL_Event *event) {
  const static SDL_FPoint lt = {9, 23};
  const static SDL_FPoint rb = {537, 44};
  for (uint8_t i = 0; i <= 5; i++) {
    SDL_FPoint tab_pos = {static_cast<float>(9 + i * 88), 23};
    SDL_FRect pos_rect{pos.x + tab_pos.x, pos.y + tab_pos.y, 88, 21};
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      selected_tab = i;
    }
  }
}

bool craft_ui_system::event(SDL_Event *event) {
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

bool craft_ui_system::run() { return true; }