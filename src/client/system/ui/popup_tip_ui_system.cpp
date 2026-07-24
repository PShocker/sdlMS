#include "popup_tip_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "wz/Node.h"

void popup_tip_ui_system::render_backgrnd(int i) {
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

void popup_tip_ui_system::render_button(int i) {
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

bool popup_tip_ui_system::event_button(SDL_Event *event) {
  std::vector<wz::Node *> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  std::vector<void (*)(int)> fns;

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