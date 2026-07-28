#include "popup_tip_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include <string>

void popup_tip_ui_system::render_backgrnd(int i) {
  SDL_Texture *texture;
  auto t = popup_tip_game_instance::data[i];
  switch (t.type) {
  case popup_tip_enums::trade: {
    static auto texture = wz_resource::load_texture(
        wz_resource::ui->find(u"StatusBar.img/submenu/backgrnd/1"));
    break;
  }
  case popup_tip_enums::party: {
    static auto texture = wz_resource::load_texture(
        wz_resource::ui->find(u"StatusBar.img/submenu/backgrnd/1"));
    break;
  }
  }
  SDL_FRect pos_rect{
      pos.x,
      pos.y - 46 * i,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

void popup_tip_ui_system::render_text(int i) {
  auto t = popup_tip_game_instance::data[i];
  SDL_FPoint p;
  std::u16string text;
  switch (t.type) {
  case popup_tip_enums::trade: {
    static auto t = wz_resource::load_texture(
        wz_resource::ui->find(u"StatusBar.img/submenu/backgrnd/1"));
    break;
  }
  case popup_tip_enums::party: {
    static auto t = wz_resource::load_texture(
        wz_resource::ui->find(u"StatusBar.img/submenu/backgrnd/1"));
    break;
  }
  }
  freetype::load_size(12);
  freetype::draw_str(text, p.x + pos.x, p.y + pos.y, 100, 1.3);
}

void popup_tip_ui_system::render_button(int i) {
  // 构建按钮列表
  std::vector<wz::Node *> nodes = {};
  std::vector<SDL_FRect> rects = {};
  auto t = popup_tip_game_instance::data[i];
  switch (t.type) {
  case popup_tip_enums::trade: {
    nodes = {};
    rects = {};
    break;
  }
  case popup_tip_enums::party: {
    nodes = {};
    rects = {};
    break;
  }
  }
  // 渲染所有按钮
  bool mouse_down = window::mouse_state & SDL_BUTTON_LMASK;
  bool cursor_on_ui = cursor_game_instance::cursor_ui == render;
  bool modal_blocked = cursor_game_instance::modal_overlay;

  for (size_t i = 0; i < nodes.size(); ++i) {
    SDL_FRect rect = {
        rects[i].x + (int)pos.x,
        rects[i].y + (int)pos.y,
        rects[i].w,
        rects[i].h,
    };

    std::u16string state = u"normal";
    if (cursor_on_ui && !modal_blocked &&
        SDL_PointInRectFloat(&window::mouse_pos, &rect)) {
      state = mouse_down ? u"pressed" : u"mouseOver";
    }
    auto texture = wz_resource::load_texture(nodes[i]->find((state + u"/0")));
    SDL_RenderTexture(window::renderer, texture, nullptr, &rect);
  }
}

bool popup_tip_ui_system::render() {
  for (int i = 0; i < popup_tip_game_instance::data.size(); i++) {
    render_backgrnd(i);
    render_button(i);
  }
  return true;
}

void popup_tip_ui_system::event_button_party(int i) {
  auto t = popup_tip_game_instance::data[i];
  ClientCharacterPartyT ccp;
  ccp.step = 1;
  ccp.confirm = true;
  client_request::send_to_host(ccp);
  return;
}

void popup_tip_ui_system::event_button_trade(int i) {}

void popup_tip_ui_system::event_button_ok(int i) {
  auto t = popup_tip_game_instance::data[i];
  switch (t.type) {
  case popup_tip_enums::trade: {
    event_button_trade(i);
    break;
  }
  case popup_tip_enums::party: {
    event_button_party(i);
    break;
  }
  }
}

void popup_tip_ui_system::event_button_cancel(int i) {
  popup_tip_game_instance::data.erase(popup_tip_game_instance::data.begin() +
                                      i);
}

bool popup_tip_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> buttons_rect;
  std::vector<void (*)(int)> fns = {event_button_ok, event_button_cancel};

  for (int i = 0; i < popup_tip_game_instance::data.size(); i++) {
    auto t = popup_tip_game_instance::data[i];
    switch (t.type) {
    case popup_tip_enums::trade: {
      break;
    }
    case popup_tip_enums::party: {
      break;
    }
    }
  }

  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += base_x;
    pos_rect.y += base_y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i](i);
      return true;
    }
  }
  return false;
}

bool popup_tip_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        r = event_button(event);
      }
    }
    break;
  }
  default: {
    break;
  }
  }
  return r;
}