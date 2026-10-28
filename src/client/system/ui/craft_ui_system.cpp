#include "craft_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>

void CraftUiSystem::render_backgrnd() {
  static auto texture = WzResource::load_texture(
      WzResource::ui->find(u"Crafting.img/backgrnd"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
}

void CraftUiSystem::render_tab() {
  const static SDL_FPoint lt = {9, 23};
  const static SDL_FPoint rb = {537, 44};
  const static auto tab_node =
      WzResource::ui->find(u"Crafting.img/tab:category");
  const static std::array selected_texture = {
      WzResource::load_texture(tab_node->find(u"selected/0")),
      WzResource::load_texture(tab_node->find(u"selected/1")),
      WzResource::load_texture(tab_node->find(u"selected/2")),
      WzResource::load_texture(tab_node->find(u"selected/3")),
      WzResource::load_texture(tab_node->find(u"selected/4")),
      WzResource::load_texture(tab_node->find(u"selected/5")),
  };
  const static std::array normal_texture = {
      WzResource::load_texture(tab_node->find(u"normal/0")),
      WzResource::load_texture(tab_node->find(u"normal/1")),
      WzResource::load_texture(tab_node->find(u"normal/2")),
      WzResource::load_texture(tab_node->find(u"normal/3")),
      WzResource::load_texture(tab_node->find(u"normal/4")),
      WzResource::load_texture(tab_node->find(u"normal/5")),
  };
  for (uint8_t i = 0; i < selected_texture.size(); i++) {
    SDL_Texture *t =
        selected_tab == i ? selected_texture[i] : normal_texture[i];
    SDL_FPoint tab_pos = {static_cast<float>(lt.x + i * 88), lt.y};
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos.x)),
                       static_cast<float>(int(pos.y + tab_pos.y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  }
}

bool CraftUiSystem::render() {
  render_backgrnd();
  render_tab();
  return true;
}

SDL_FPoint CraftUiSystem::load_wh() { return {545, 409}; }

void CraftUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void CraftUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
}

void CraftUiSystem::event_top() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void CraftUiSystem::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void CraftUiSystem::event_drag_end() {
  drag = std::nullopt;
  return;
}

void CraftUiSystem::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void CraftUiSystem::toggle() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool CraftUiSystem::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void CraftUiSystem::event_tab(SDL_Event *event) {
  const static SDL_FPoint lt = {9, 23};
  const static SDL_FPoint rb = {537, 44};
  for (uint8_t i = 0; i <= 5; i++) {
    SDL_FPoint tab_pos = {static_cast<float>(9 + i * 88), 23};
    SDL_FRect pos_rect{pos.x + tab_pos.x, pos.y + tab_pos.y, 88, 21};
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      selected_tab = i;
    }
  }
}

bool CraftUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        event_top();
        event_drag_start(event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
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

bool CraftUiSystem::run() { return true; }