#include "src/client/system/ui/user_list_ui_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/friend_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <algorithm>
#include <ranges>
#include <string>

namespace sdlms {

namespace {

SDL_FPoint g_user_list_pos = {0, 0};
bool g_user_list_visible = false;

constexpr float kUserListW = 314.0f;
constexpr float kUserListH = 390.0f;

std::u16string ToU16(int v) {
  std::string s = std::to_string(v);
  return std::u16string(s.begin(), s.end());
}

}  // namespace

void UserListUiSystem::RenderBackgrnd() {
  auto* node = WzResource::ui_->find(u"UserList.img/backgrnd");
  if (node == nullptr) {
    return;
  }
  auto* t = WzResource::LoadTexture(node);
  SDL_FRect r{g_user_list_pos.x, g_user_list_pos.y, static_cast<float>(t->w),
              static_cast<float>(t->h)};
  SDL_RenderTexture(Window::renderer_, t, nullptr, &r);
}

bool UserListUiSystem::Render() {
  RenderBackgrnd();

  // 标题：BUDDY (数量)
  Freetype::LoadAligned(true);
  Freetype::LoadSize(13);
  Freetype::LoadBold(true);
  Freetype::LoadColor(20, 20, 20, 255);
  Freetype::DrawLine(
      u"BUDDY " + ToU16(static_cast<int>(FriendGameInstance::friends_.size())),
      g_user_list_pos.x + 12, g_user_list_pos.y + 10);

  // 好友列表：在线用 “[+]”，离线用 “[-]”。
  Freetype::LoadAligned(true);
  Freetype::LoadSize(12);
  Freetype::LoadBold(false);
  Freetype::LoadColor(20, 20, 20, 255);
  const int max_rows = 14;
  const int n =
      std::min(static_cast<int>(FriendGameInstance::friends_.size()), max_rows);
  for (int i = 0; i < n; ++i) {
    const auto& f = FriendGameInstance::friends_[i];
    const bool online = FriendGameInstance::IsOnline(f.id);
    std::u16string line = (online ? u"[+] " : u"[-] ") + f.name;
    Freetype::DrawLine(line, g_user_list_pos.x + 14,
                       g_user_list_pos.y + 52 + static_cast<float>(i) * 18);
  }
  if (FriendGameInstance::friends_.empty()) {
    Freetype::DrawLine(u"(no friends)", g_user_list_pos.x + 14,
                       g_user_list_pos.y + 52);
  }

  // 操作提示
  Freetype::LoadSize(11);
  Freetype::LoadBold(false);
  Freetype::LoadColor(90, 90, 90, 255);
  Freetype::DrawLine(u"Right-click a player -> Add Friend",
                     g_user_list_pos.x + 12, g_user_list_pos.y + 362);
  return true;
}

bool UserListUiSystem::Event(SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      if (event->key.scancode == SDL_SCANCODE_ESCAPE ||
          event->key.scancode == SDL_SCANCODE_O) {
        Close();
        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      // 窗口覆盖，吞掉点击避免误触后面的地图。
      return true;
    }
    default: {
      break;
    }
  }
  return false;
}

void UserListUiSystem::Open() {
  if (g_user_list_visible) {
    return;
  }
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it == System::render_systems_.end()) {
    return;
  }
  const auto& camera = CameraGameInstance::camera_;
  g_user_list_pos.x = (camera.w - kUserListW) / 2;
  g_user_list_pos.y = (camera.h - kUserListH) / 2;
  System::render_systems_.insert(it, Render);
  System::event_systems_.insert(System::event_systems_.begin(), Event);
  g_user_list_visible = true;
}

void UserListUiSystem::Close() {
  if (!g_user_list_visible) {
    return;
  }
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  g_user_list_visible = false;
}

void UserListUiSystem::Toggle() {
  if (g_user_list_visible) {
    Close();
  } else {
    Open();
  }
}

}  // namespace sdlms