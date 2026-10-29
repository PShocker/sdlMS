#include "src/client/system/ui/component/tab.h"

#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>

#include <optional>
#include <string>

namespace sdlms {

void Tab::Render() const {
  for (size_t i = 0; i < rects.size(); ++i) {
    const auto& state = selected == i ? on_state : off_state;
    SDL_Texture* texture = WzResource::LoadTexture(
        node->get_child(state)->get_child(std::to_string(i)));
    SDL_FRect pos_rect{
        static_cast<float>((int)rects[i].x),
        static_cast<float>((int)rects[i].y),
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
}

std::optional<uint8_t> Tab::Click(SDL_FPoint mouse) const {
  for (size_t i = 0; i < rects.size(); ++i) {
    if (SDL_PointInRectFloat(&mouse, &rects[i])) {
      return static_cast<uint8_t>(i);
    }
  }
  return std::nullopt;
}

}  // namespace sdlms
