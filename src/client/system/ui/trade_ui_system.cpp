#include "trade_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <ranges>

SDL_FPoint TradeUiSystem::LoadWh() { return SDL_FPoint{567, 442}; }

bool TradeUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void TradeUiSystem::Close() {
  std::erase(System::render_systems, Render);
  std::erase(System::event_systems, Event);
}

void TradeUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::Render);
  if (it != System::render_systems.end()) {
    auto wh = LoadWh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    CharacterLogicSystem::RunStandAction(other);
    CharacterLogicSystem::RunFaceAction(other, u"default");
    other.flip = 0;

    System::render_systems.insert(it, Render);
    System::event_systems.insert(System::event_systems.begin(), Event);
  }
}

void TradeUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems, fn)) {
    Close();
  } else {
    Open();
  }
}

void TradeUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui->find(u"Trade.img/TradingRoom/backgrnd"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);
}

void TradeUiSystem::RenderButton() {
  const static std::array buttons_node = {
      WzResource::ui->find(u"MapleChat.img/MapleChat/button:claim"),
      WzResource::ui->find(u"Trade.img/TradingRoom/button:trade"),
      WzResource::ui->find(u"MapleChat.img/MapleChat/button:enter"),
      WzResource::ui->find(u"Trade.img/TradingRoom/button:coin"),
      WzResource::ui->find(u"Trade.img/TradingRoom/button:reset"),
  };
  const std::array buttons_rect = {
      SDL_FRect{541, 250, 20, 19}, // claim
      SDL_FRect{116, 48, 60, 18},  // trade
      SDL_FRect{500, 251, 39, 17}, // enter
      SDL_FRect{151, 243, 36, 18}, // coin
      SDL_FRect{116, 70, 60, 18},  // reset
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto CursorIn = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && CursorIn == Render) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::LoadTexture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::LoadTexture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::LoadTexture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

bool TradeUiSystem::Render() { return true; }

void TradeUiSystem::EventButtonClose() { Close(); }

bool TradeUiSystem::EventButton(SDL_Event *Event) {
  const static std::array buttons_rect = {
      SDL_FRect{119, 115, 47, 18}, //
  };
  const static std::array buttons_func = {
      EventButtonClose,
  };

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      buttons_func[i]();
      return true;
    }
  }
  return false;
}

bool TradeUiSystem::Event(SDL_Event *Event) {
  bool r = true;
  switch (Event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        r = !EventButton(Event);
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