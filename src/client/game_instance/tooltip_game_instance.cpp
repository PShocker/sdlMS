#include "src/client/game_instance/tooltip_game_instance.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_render.h>
#include <wz/Property.h>

#include <algorithm>
#include <cmath>
#include <string>

namespace sdlms {

namespace {

int LoadInt(wz::Node* node, const char16_t* name) {
  auto* child = node->get_child(name);
  if (child == nullptr || child->type != wz::Type::Int) {
    return 0;
  }
  return static_cast<wz::Property<int32_t>*>(child)->get();
}

}  // namespace

void TooltipGameInstance::Load(uint32_t map_id) {
  data_.clear();
  auto* map_node = WzResource::LoadMapNode(map_id);
  if (map_node == nullptr) {
    return;
  }
  // 095 CField::RestoreToolTip：地图 ToolTip 节点，每个子项是一块矩形提示区。
  auto* node = map_node->get_child(u"ToolTip");
  if (node == nullptr) {
    return;
  }
  for (const auto& [key, values] : node->children) {
    for (auto* item : values) {
      auto x1 = LoadInt(item, u"x1");
      auto y1 = LoadInt(item, u"y1");
      auto x2 = LoadInt(item, u"x2");
      auto y2 = LoadInt(item, u"y2");
      std::u16string text;
      if (auto* t = item->get_child(u"text");
          t != nullptr && t->type == wz::Type::String) {
        text = static_cast<wz::Property<std::u16string>*>(t)->get();
      } else {
        text = key;
      }
      if (text.empty()) {
        continue;
      }
      GameTooltip tip;
      tip.path = text;
      tip.r = SDL_FRect{
          static_cast<float>(std::min(x1, x2)),
          static_cast<float>(std::min(y1, y2)),
          static_cast<float>(std::abs(x2 - x1)),
          static_cast<float>(std::abs(y2 - y1)),
      };
      data_.push_back(tip);
    }
  }
}

bool TooltipGameInstance::Render() {
  if (data_.empty()) {
    return true;
  }
  auto& camera = CameraGameInstance::camera_;
  for (const auto& tip : data_) {
    SDL_FRect area{
        tip.r.x - camera.x,
        tip.r.y - camera.y,
        tip.r.w,
        tip.r.h,
    };
    if (!SDL_PointInRectFloat(&Window::mouse_pos_, &area)) {
      continue;
    }

    Freetype::LoadSize(11);
    Freetype::LoadBold(false);
    Freetype::LoadAligned(true);
    auto w = Freetype::LoadW(tip.path);
    auto lh = static_cast<float>(Freetype::LoadLh());

    SDL_FRect box{
        Window::mouse_pos_.x + 14,
        Window::mouse_pos_.y - lh - 8,
        w + 10,
        lh + 8,
    };
    SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, 190);
    SDL_RenderFillRect(Window::renderer_, &box);
    SDL_SetRenderDrawColor(Window::renderer_, 220, 220, 220, 255);
    SDL_RenderRect(Window::renderer_, &box);
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawLine(tip.path, box.x + 5, box.y + 2);
    break;
  }
  return true;
}

}  // namespace sdlms