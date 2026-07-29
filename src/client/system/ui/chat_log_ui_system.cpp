#include "chat_log_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "scroll_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/chat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include <flat_map>
#include <format>

void chat_log_ui_system::render_vscr() {
  const uint32_t length = 80;
  auto size = 6;
  auto cursor_in = cursor_game_instance::cursor_ui;
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  bool top = cursor_in == render && !cursor_game_instance::modal_overlay;
  scroll_ui_system::render_vscroll(base_x + 570, base_y - 75, 5, size, length,
                                   top, 1);
  return;
}

void chat_log_ui_system::render_chats() {
  float str_h = 0;
  auto &chats = chat_game_instance::chats;
  for (int i = chats.size() - 1; i >= 0; i--) {
    auto chat = chats[i];
    freetype::load_aligned(true);
    freetype::load_size(12);
    auto str = chat.owner + u":" + chat.text;
    auto screen_w = camera_game_instance::camera.w;
    auto screen_h = camera_game_instance::camera.h;
    auto base_x = (screen_w - 808) / 2;
    auto base_y = (screen_h - 73);
    freetype::load_color(255, 255, 255, 255);
    str_h += freetype::load_h(str, 550, 1.1);
    freetype::draw_str(str, base_x + 8, base_y - str_h + 2, 550, 1.1);
  }
}

void chat_log_ui_system::render_backgrnd() {
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  SDL_FRect pos_rect;
  static auto chatbackgrnd1 = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/chat/canvas:chatbackgrnd1"));
  pos_rect.x = base_x + 6;
  pos_rect.y = base_y - 75;
  pos_rect.w = chatbackgrnd1->w;
  pos_rect.h = 80;

  SDL_RenderTexture(window::renderer, chatbackgrnd1, nullptr, &pos_rect);
}

bool chat_log_ui_system::render() {
  render_backgrnd();
  render_chats();
  render_vscr();
  return true;
}

void chat_log_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void chat_log_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void chat_log_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void chat_log_ui_system::event_click_chat_vscr() {
  const uint32_t length = 76;
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top = cursor_in == render;
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);

  int size = 0;
  for (const auto &chats : chat_game_instance::chats) {
    auto str = chats.owner + u":" + chats.text;
    auto h = freetype::load_h(str, 564, 1.1);
    size += h;
  }
  size = size / (freetype::load_lh() * 1.1);
  size = std::max(0, size);
  auto val = scroll_ui_system::click_vscroll(base_x + 564, base_y - 71, page,
                                             size, length, top);
  page = val;
  return;
}

bool chat_log_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        event_click_chat_vscr();
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    break;
  }
  default: {
    break;
  }
  }

  return r;
}