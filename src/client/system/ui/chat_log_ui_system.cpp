#include "chat_log_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "scroll_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/chat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include <flat_map>
#include <format>

void ChatLogUiSystem::RenderVscr() {
  const uint32_t length = 80;
  auto size = 6;
  auto CursorIn = CursorGameInstance::cursor_ui;
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  bool top = CursorIn == Render && !CursorGameInstance::modal_overlay;
  ScrollUiSystem::RenderVscroll(base_x + 570, base_y - 75, 5, size, length,
                                   top, 1);
  return;
}

void ChatLogUiSystem::RenderChats() {
  float str_h = 0;
  auto &chats = ChatGameInstance::chats_;
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
  RenderVscr();
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

void ChatLogUiSystem::EventClickChatVscr() {
  const uint32_t length = 76;
  auto CursorIn = CursorGameInstance::cursor_ui;
  bool top = CursorIn == Render;
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);

  int size = 0;
  for (const auto &chats : ChatGameInstance::chats_) {
    auto str = chats.owner + u":" + chats.text;
    auto h = Freetype::LoadCh(str, 564, 1.1);
    size += h;
  }
  size = size / (Freetype::LoadLh() * 1.1);
  size = std::max(0, size);
  auto val = ScrollUiSystem::ClickVscroll(base_x + 564, base_y - 71, page_,
                                             size, length, top);
  page_ = val;
  return;
}

bool ChatLogUiSystem::Event(SDL_Event *Event) {
  bool r = true;
  switch (Event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        EventClickChatVscr();
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