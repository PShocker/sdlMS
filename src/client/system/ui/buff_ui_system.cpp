#include "src/client/system/ui/buff_ui_system.h"

#include "src/client/game/game_item_buff.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/tooltip_ui_system.h"
#include "src/client/system_instance/fade_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace sdlms {

static std::optional<GameSkill> mouse_ski;
static std::optional<GameItemBuff> mouse_item;

void BuffUiSystem::RenderUi(GameSkill& sk, float x, float y) {
  auto id = sk.id;
  auto ski_node = SkillGameInstance::LoadSkiNode(id);
  auto icon = WzResource::LoadTexture(ski_node->get_child(u"icon"));
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(Window::renderer_, icon, NULL, &pos_rect);
  auto& mouse_pos = Window::mouse_pos_;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    mouse_ski = sk;
  }
  if (sk.duration != 0) {
    SDL_SetRenderDrawBlendMode(Window::renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, 100);
    auto d = sk.destroy - Window::dt_now_;
    pos_rect.x = x;
    pos_rect.h = 32 * (1 - (d / (float)sk.duration));
    pos_rect.y = y + 32 - pos_rect.h;
    pos_rect.w = 32;
    SDL_RenderFillRect(Window::renderer_, &pos_rect);
    // 渲染冷却时间
    auto num = d / 1000;
    PackageUiSystem::RenderNumber(num, x, y + 21);
  }
}

void BuffUiSystem::RenderUi(GameItemBuff& gb, float x, float y) {
  auto id = gb.id;
  auto info = ItemGameInstance::LoadItemInfo(id, 0);
  auto icon = WzResource::LoadTexture(info->get_child(u"icon"));
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(Window::renderer_, icon, NULL, &pos_rect);
  auto& mouse_pos = Window::mouse_pos_;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    mouse_item = gb;
  }
  if (gb.destroy) {
    SDL_SetRenderDrawBlendMode(Window::renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, 100);
    auto d = gb.destroy - Window::dt_now_;
    pos_rect.x = x;
    pos_rect.h = 32 * (1 - (d / (float)gb.duration));
    pos_rect.y = y + 32 - pos_rect.h;
    pos_rect.w = 32;
    SDL_RenderFillRect(Window::renderer_, &pos_rect);
    // 渲染冷却时间
    auto num = d / 1000;
    PackageUiSystem::RenderNumber(num, x, y + 21);
  }
}

bool BuffUiSystem::RenderUiInfo() {
  if (!mouse_ski.has_value()) {
    return true;
  }
  if (CursorGameInstance::cursor_ui != nullptr) {
    return true;
  }
  auto ski = mouse_ski.value();
  // 宽度固定330
  const auto w = 390;
  auto& mouse_pos = Window::mouse_pos_;
  const auto& camera = CameraGameInstance::camera_;
  auto x = 0;
  if (mouse_pos.x + w <= camera.w) {
    x = mouse_pos.x;
  } else {
    x = camera.w - w;
  }
  TooltipUiSystem::RenderSkill(ski.id, ski.lv, x, mouse_pos.y);
  return true;
}

void BuffUiSystem::RenderUi() {
  // ski buff
  const auto& camera = CameraGameInstance::camera_;
  for (uint32_t i = 0; i < SkillGameInstance::ski_.size(); i++) {
    auto col = i % 5;
    auto row = i / 5;
    auto x = camera.w - 32 - col * 32;
    auto y = row * 32;
    RenderUi(SkillGameInstance::ski_[i], x, y);
  }
  auto ski_size = SkillGameInstance::ski_.size();
  for (uint32_t i = 0; i < ItemGameInstance::buff_.size(); i++) {
    auto col = (i + ski_size) % 5;
    auto row = (i + ski_size) / 5;
    auto x = camera.w - 32 - col * 32;
    auto y = row * 32;
    RenderUi(ItemGameInstance::buff_[i], x, y);
  }
}

void BuffUiSystem::EventMotion(SDL_Event* event) {
  auto& sys = System::render_systems_;
  std::erase(sys, RenderUiInfo);
  auto it = std::ranges::find(sys, &CursorRenderSystem::Render);
  if (it != sys.end()) {
    sys.insert(it, RenderUiInfo);
  }
}

bool BuffUiSystem::Render() {
  mouse_ski = std::nullopt;
  mouse_item = std::nullopt;
  RenderUi();
  return true;
}

bool BuffUiSystem::Event(SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_RIGHT) {
        if (CursorGameInstance::cursor_ui == nullptr) {
          if (mouse_ski.has_value()) {
            mouse_ski->end();
          }
          if (mouse_item.has_value()) {
            ItemGameInstance::UnuseBuffItem(mouse_item->id);
          }
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventMotion(event);
      break;
    }
    default: {
      break;
    }
  }

  return true;
}

}  // namespace sdlms
