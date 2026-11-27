#include "src/client/system/ui/party_hp_ui_system.h"

#include "src/client/game_instance/party_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_main.h"

#include <SDL3/SDL_render.h>

#include <algorithm>
#include <string>

namespace sdlms {

namespace {

// 095 PartyHP.img：GaugeBar/bar 63x7 底条、gauge 1x7 填充、graduation 69x13 刻度。
constexpr float kPanelX = 8;
constexpr float kPanelY = 120;
constexpr float kRowHeight = 26;

}  // namespace

bool PartyHpUiSystem::Render() {
  auto& party = PartyGameInstance::data_;
  if (!party.has_value() || party->members.size() < 2) {
    return true;
  }

  static auto bar = WzResource::LoadTexture(
      WzResource::ui_->find(u"PartyHP.img/GaugeBar/bar"));
  static auto gauge = WzResource::LoadTexture(
      WzResource::ui_->find(u"PartyHP.img/GaugeBar/gauge"));
  static auto graduation = WzResource::LoadTexture(
      WzResource::ui_->find(u"PartyHP.img/GaugeBar/graduation"));
  if (bar == nullptr || gauge == nullptr) {
    return true;
  }

  Freetype::LoadSize(11);
  Freetype::LoadBold(false);
  Freetype::LoadAligned(true);

  auto y = kPanelY;
  for (const auto& member : party->members) {
    // 自己的血量由状态栏显示，这里只列其他队员（095 CUIPartyHP 同）。
    if (member.player.client_id == ServerMain::local_addr_) {
      continue;
    }

    std::u16string name;
    if (member.player.character != nullptr) {
      name = {member.player.character->name.begin(),
              member.player.character->name.end()};
    }
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawLine(name, kPanelX + 2, y);

    SDL_FRect bar_rect{
        kPanelX + 2,
        y + 14,
        static_cast<float>(bar->w),
        static_cast<float>(bar->h),
    };
    SDL_RenderTexture(Window::renderer_, bar, nullptr, &bar_rect);

    if (member.max_hp > 0 && gauge != nullptr) {
      auto ratio = std::clamp(
          static_cast<float>(member.hp) / static_cast<float>(member.max_hp),
          0.0f, 1.0f);
      SDL_FRect gauge_rect{
          bar_rect.x,
          bar_rect.y,
          bar_rect.w * ratio,
          static_cast<float>(gauge->h),
      };
      if (gauge_rect.w > 0) {
        SDL_RenderTextureTiled(Window::renderer_, gauge, nullptr, 1,
                               &gauge_rect);
      }
    }

    if (graduation != nullptr) {
      SDL_FRect grad_rect{
          bar_rect.x + (bar_rect.w - graduation->w) / 2,
          bar_rect.y + (bar_rect.h - graduation->h) / 2,
          static_cast<float>(graduation->w),
          static_cast<float>(graduation->h),
      };
      SDL_RenderTexture(Window::renderer_, graduation, nullptr, &grad_rect);
    }

    y += kRowHeight;
  }
  return true;
}

}  // namespace sdlms
