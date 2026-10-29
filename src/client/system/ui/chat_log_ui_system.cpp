#include "src/client/system/ui/chat_log_ui_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/chat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/scroll.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>

#include <flat_map>
#include <format>

namespace sdlms {

void ChatLogUiSystem::RenderChats() {
  float str_h = 0;
  auto& chats = ChatGameInstance::chats_;
  for (int i = chats.size() - 1; i >= 0; i--) {
    auto chat = chats[i];
    Freetype::LoadAligned(true);
    Freetype::LoadSize(12);
    auto str = chat.owner + u":" + chat.text;
    auto screen_w = CameraGameInstance::camera_.w;
    auto screen_h = CameraGameInstance::camera_.h;
    auto base_x = (screen_w - 808) / 2;
    auto base_y = (screen_h - 73);
    Freetype::LoadColor(255, 255, 255, 255);
    str_h += Freetype::LoadCh(str, 550, 1.1);
    Freetype::DrawStr(str, base_x + 8, base_y - str_h + 2, 550, 1.1);
  }
}

void ChatLogUiSystem::RenderBackgrnd() {
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  SDL_FRect pos_rect;
  static auto chatbackgrnd1 = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/chat/canvas:chatbackgrnd1"));
  pos_rect.x = base_x + 6;
  pos_rect.y = base_y - 75;
  pos_rect.w = chatbackgrnd1->w;
  pos_rect.h = 80;

  SDL_RenderTexture(Window::renderer_, chatbackgrnd1, nullptr, &pos_rect);
}

bool ChatLogUiSystem::Render() {
  RenderBackgrnd();
  RenderChats();
  RenderUI();
  return true;
}

void ChatLogUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void ChatLogUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void ChatLogUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

UIComponents& ChatLogUiSystem::GetComponent() {
  components_.clear();
  const uint32_t length = 80;
  auto size = 6;
  auto cursor_in = CursorGameInstance::cursor_ui_;
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  bool top = cursor_in == Render && !CursorGameInstance::modal_overlay_;
  Scroll scroll(page_, size, length, 1);
  scroll.pos = {base_x + 570, base_y - 75};
  scroll.top = top;
  components_.push_back(scroll);
  return components_;
}

void ChatLogUiSystem::RenderUI() {
  RenderComponents(GetComponent());
}

bool ChatLogUiSystem::EventUI(SDL_Event* event) {
  for (auto& component : GetComponent()) {
    if (auto* scroll = std::get_if<Scroll>(&component)) {
      page_ = scroll->Click();
    }
  }
  return false;
}

bool ChatLogUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = !EventUI(event);
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
