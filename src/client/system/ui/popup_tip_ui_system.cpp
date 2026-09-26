#include "popup_tip_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <string>

void popup_tip_ui_system::render_backgrnd(game_popup_tip &t) {
  SDL_Texture *te;
  SDL_Texture *i = nullptr;
  SDL_FPoint icon_pos;
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
  case popup_tip_enums::quest: {
    static auto texture = wz_resource::load_texture(
        wz_resource::ui->find(u"FadeYesNo.img/FadeYesNo/backgrnd3"));
    static auto icon = wz_resource::load_texture(
        wz_resource::ui->find(u"FadeYesNo.img/FadeYesNo/icon6"));
    te = texture;
    i = icon;
    icon_pos = {-10, -12};
    break;
  }
  }
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto x = pos.x + (screen_w - 808) / 2;
  auto y = pos.y + (screen_h - 73);

  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(te->w),
      static_cast<float>(te->h),
  };
  SDL_SetTextureAlphaMod(te, t.alpha);
  SDL_RenderTexture(window::renderer, te, nullptr, &pos_rect);

  pos_rect.x -= icon_pos.x;
  pos_rect.y -= icon_pos.y;
  pos_rect.w = i->w;
  pos_rect.h = i->h;
  SDL_SetTextureAlphaMod(i, t.alpha);
  SDL_RenderTexture(window::renderer, i, nullptr, &pos_rect);
}

void popup_tip_ui_system::render_text(game_popup_tip &t) {
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto x = pos.x + (screen_w - 808) / 2;
  auto y = pos.y + (screen_h - 73);

  switch (t.type) {
  case popup_tip_enums::quest: {
    auto quest_id = std::any_cast<std::u16string>(t.data);
    auto node = quest_game_instance::load_quest_node(quest_id);
    node = node->find(u"QuestInfo/name");
    auto name = static_cast<wz::Property<std::u16string> *>(node)->get();
    if (name.size() > 17) {
      name = name.substr(0, 17) + u"...";
    }
    freetype::load_size(12);
    freetype::load_color(255, 255, 255, t.alpha);
    freetype::load_aligned(true);
    freetype::draw_line(name, x + 28, y + 3);

    name = u"Quest Complete!";
    freetype::draw_line(name, x + 28, y + 16);
    break;
  }
  }
}

void popup_tip_ui_system::render_button(game_popup_tip &t) {
  // 构建按钮列表
  std::vector<wz::Node *> nodes = {};
  std::vector<SDL_FRect> rects = {};
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
  auto &data = popup_tip_game_instance::data;
  const auto now = window::dt_now;
  const auto dt = window::delta_time;

  for (auto &t : data) {
    const float dir = (t.destroy < now) ? -1.0f : 1.0f;
    t.alpha = std::clamp(t.alpha + dir * dt, 0.0f, 255.0f);
    render_backgrnd(t);
    render_text(t);
    render_button(t);
  }

  std::erase_if(data, [now](const game_popup_tip &t) {
    return t.destroy < now && t.alpha == 0.0f;
  });

  return true;
}

void popup_tip_ui_system::event_button_party(game_popup_tip &t) {
  ClientCharacterPartyT ccp;
  ccp.step = 1;
  client_request::send_to_host(ccp);
  return;
}

void popup_tip_ui_system::event_button_trade(game_popup_tip &t) {}

void popup_tip_ui_system::event_button_ok(game_popup_tip &t) {
  switch (t.type) {
  case popup_tip_enums::trade: {
    event_button_trade(t);
    break;
  }
  case popup_tip_enums::party: {
    event_button_party(t);
    break;
  }
  }
}

void popup_tip_ui_system::event_button_cancel(game_popup_tip &t) {
  auto &data = popup_tip_game_instance::data;
  std::erase_if(data, [t](const auto &tip) { return &t == &tip; });
}

bool popup_tip_ui_system::event_button(SDL_Event *event) {
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);

  std::vector<SDL_FRect> buttons_rect;
  std::vector<void (*)(game_popup_tip &)> fns = {
      event_button_ok,
      event_button_cancel,
  };

  for (int i = 0; i < popup_tip_game_instance::data.size(); i++) {
    auto &t = popup_tip_game_instance::data[i];
    switch (t.type) {
    case popup_tip_enums::trade: {
      break;
    }
    case popup_tip_enums::party: {
      break;
    }
    }
    for (size_t i = 0; i < buttons_rect.size(); ++i) {
      auto pos_rect = buttons_rect[i];
      pos_rect.x += base_x;
      pos_rect.y += base_y;
      if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
        fns[i](t);
        return true;
      }
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