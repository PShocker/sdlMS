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

void PopupTipUiSystem::render_backgrnd(GamePopupTip &t) {
  SDL_Texture *te;
  SDL_Texture *i = nullptr;
  SDL_FPoint icon_pos;
  switch (t.type) {
  case PopupTipEnums::trade: {
    static auto texture = WzResource::load_texture(
        WzResource::ui->find(u"StatusBar.img/submenu/backgrnd/1"));
    break;
  }
  case PopupTipEnums::party: {
    static auto texture = WzResource::load_texture(
        WzResource::ui->find(u"StatusBar.img/submenu/backgrnd/1"));
    break;
  }
  case PopupTipEnums::quest: {
    static auto texture = WzResource::load_texture(
        WzResource::ui->find(u"FadeYesNo.img/FadeYesNo/backgrnd3"));
    static auto icon = WzResource::load_texture(
        WzResource::ui->find(u"FadeYesNo.img/FadeYesNo/icon6"));
    te = texture;
    i = icon;
    icon_pos = {-10, -12};
    break;
  }
  }
  auto screen_w = CameraGameInstance::camera.w;
  auto screen_h = CameraGameInstance::camera.h;
  auto x = pos.x + (screen_w - 808) / 2;
  auto y = pos.y + (screen_h - 73);

  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(te->w),
      static_cast<float>(te->h),
  };
  SDL_SetTextureAlphaMod(te, t.alpha);
  SDL_RenderTexture(Window::renderer, te, nullptr, &pos_rect);

  pos_rect.x -= icon_pos.x;
  pos_rect.y -= icon_pos.y;
  pos_rect.w = i->w;
  pos_rect.h = i->h;
  SDL_SetTextureAlphaMod(i, t.alpha);
  SDL_RenderTexture(Window::renderer, i, nullptr, &pos_rect);
}

void PopupTipUiSystem::render_text(GamePopupTip &t) {
  auto screen_w = CameraGameInstance::camera.w;
  auto screen_h = CameraGameInstance::camera.h;
  auto x = pos.x + (screen_w - 808) / 2;
  auto y = pos.y + (screen_h - 73);

  switch (t.type) {
  case PopupTipEnums::quest: {
    auto quest_id = std::any_cast<std::u16string>(t.data);
    auto node = QuestGameInstance::load_quest_node(quest_id);
    node = node->find(u"QuestInfo/name");
    auto name = static_cast<wz::Property<std::u16string> *>(node)->get();
    if (name.size() > 17) {
      name = name.substr(0, 17) + u"...";
    }
    Freetype::load_size(12);
    Freetype::load_color(255, 255, 255, t.alpha);
    Freetype::load_aligned(true);
    Freetype::draw_line(name, x + 28, y + 3);

    name = u"Quest Complete!";
    Freetype::draw_line(name, x + 28, y + 16);
    break;
  }
  }
}

void PopupTipUiSystem::render_button(GamePopupTip &t) {
  // 构建按钮列表
  std::vector<wz::Node *> nodes = {};
  std::vector<SDL_FRect> rects = {};
  switch (t.type) {
  case PopupTipEnums::trade: {
    nodes = {};
    rects = {};
    break;
  }
  case PopupTipEnums::party: {
    nodes = {};
    rects = {};
    break;
  }
  }
  // 渲染所有按钮
  bool mouse_down = Window::mouse_state & SDL_BUTTON_LMASK;
  bool cursor_on_ui = CursorGameInstance::cursor_ui == render;
  bool modal_blocked = CursorGameInstance::modal_overlay;

  for (size_t i = 0; i < nodes.size(); ++i) {
    SDL_FRect rect = {
        rects[i].x + (int)pos.x,
        rects[i].y + (int)pos.y,
        rects[i].w,
        rects[i].h,
    };

    std::u16string state = u"normal";
    if (cursor_on_ui && !modal_blocked &&
        SDL_PointInRectFloat(&Window::mouse_pos, &rect)) {
      state = mouse_down ? u"pressed" : u"mouseOver";
    }
    auto texture = WzResource::load_texture(nodes[i]->find((state + u"/0")));
    SDL_RenderTexture(Window::renderer, texture, nullptr, &rect);
  }
}

void PopupTipUiSystem::destory() {
  auto &data = PopupTipGameInstance::data;
  const auto now = Window::dt_now;
  std::erase_if(data, [now](const GamePopupTip &t) {
    return t.destroy < now && t.alpha == 0.0f;
  });
}

bool PopupTipUiSystem::render() {
  auto &data = PopupTipGameInstance::data;
  const auto now = Window::dt_now;
  const auto dt = Window::delta_time;

  for (auto &t : data) {
    const float dir = (t.destroy < now) ? -1.0f : 1.0f;
    t.alpha = std::clamp(t.alpha + dir * dt, 0.0f, 255.0f);
    render_backgrnd(t);
    render_text(t);
    render_button(t);
  }
  
  destory();

  return true;
}

void PopupTipUiSystem::event_button_party(GamePopupTip &t) {
  ClientCharacterPartyT ccp;
  ccp.step = 1;
  ClientRequest::send_to_host(ccp);
  return;
}

void PopupTipUiSystem::event_button_trade(GamePopupTip &t) {}

void PopupTipUiSystem::event_button_ok(GamePopupTip &t) {
  switch (t.type) {
  case PopupTipEnums::trade: {
    event_button_trade(t);
    break;
  }
  case PopupTipEnums::party: {
    event_button_party(t);
    break;
  }
  }
}

void PopupTipUiSystem::event_button_cancel(GamePopupTip &t) {
  auto &data = PopupTipGameInstance::data;
  std::erase_if(data, [t](const auto &tip) { return &t == &tip; });
}

bool PopupTipUiSystem::event_button(SDL_Event *event) {
  auto screen_w = CameraGameInstance::camera.w;
  auto screen_h = CameraGameInstance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);

  std::vector<SDL_FRect> buttons_rect;
  std::vector<void (*)(GamePopupTip &)> fns = {
      event_button_ok,
      event_button_cancel,
  };

  for (int i = 0; i < PopupTipGameInstance::data.size(); i++) {
    auto &t = PopupTipGameInstance::data[i];
    switch (t.type) {
    case PopupTipEnums::trade: {
      break;
    }
    case PopupTipEnums::party: {
      break;
    }
    }
    for (size_t i = 0; i < buttons_rect.size(); ++i) {
      auto pos_rect = buttons_rect[i];
      pos_rect.x += base_x;
      pos_rect.y += base_y;
      if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
        fns[i](t);
        return true;
      }
    }
  }
  return false;
}

bool PopupTipUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
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