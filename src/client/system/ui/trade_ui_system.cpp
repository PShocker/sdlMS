#include "src/client/system/ui/trade_ui_system.h"

#include "src/client/component/button.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>

#include <algorithm>
#include <ranges>

namespace sdlms {

SDL_FPoint TradeUiSystem::LoadWh() {
  return SDL_FPoint{567, 442};
}

bool TradeUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void TradeUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void TradeUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    CharacterLogicSystem::RunStandAction(other_);
    CharacterLogicSystem::RunFaceAction(other_, u"default");
    other_.flip = 0;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void TradeUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

void TradeUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"Trade.img/TradingRoom/backgrnd"));
  SDL_FRect pos_rect{pos_.x, pos_.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
}

void TradeUiSystem::RenderButton() {
  components_.clear();
  const static std::array buttons_node = {
      WzResource::ui_->find(u"MapleChat.img/MapleChat/button:claim"),
      WzResource::ui_->find(u"Trade.img/TradingRoom/button:trade"),
      WzResource::ui_->find(u"MapleChat.img/MapleChat/button:enter"),
      WzResource::ui_->find(u"Trade.img/TradingRoom/button:coin"),
      WzResource::ui_->find(u"Trade.img/TradingRoom/button:reset"),
  };
  const std::array buttons_rect = {
      SDL_FRect{541, 250, 20, 19},  // claim
      SDL_FRect{116, 48, 60, 18},   // trade
      SDL_FRect{500, 251, 39, 17},  // enter
      SDL_FRect{151, 243, 36, 18},  // coin
      SDL_FRect{116, 70, 60, 18},   // reset
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x += pos_.x;
    rect.y += pos_.y;
    Button button{.node = buttons_node[i],
                  .rect = rect,
                  .enable = true,
                  .focused = CursorGameInstance::cursor_ui_ == Render &&
                             !CursorGameInstance::modal_overlay_};
    components_.push_back(button);
    button.Render();
  }
}

std::vector<Button>& TradeUiSystem::GetComponent() {
  return components_;
}


bool TradeUiSystem::Render() {
  return true;
}

void TradeUiSystem::EventButtonClose() {
  Close();
}

bool TradeUiSystem::EventButton(SDL_Event* event) {
  const static std::array buttons_rect = {
      SDL_FRect{119, 115, 47, 18},  //
  };
  const static std::array buttons_func = {
      EventButtonClose,
  };

  for (size_t i = 0; i < components_.size() && i < buttons_func.size(); ++i) {
    components_[i].callback = buttons_func[i];
  }
  if (DispatchButtonClick(components_, false)) {
    return true;
  }
  return false;
}

bool TradeUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = !EventButton(event);
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

}  // namespace sdlms
