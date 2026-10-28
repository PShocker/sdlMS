#include "tile_render_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
namespace sdlms {


bool TileRenderSystem::Render(GameTile &g_tile) {
  auto tile_node = WzResource::map_->find(g_tile.path);
  auto texture = WzResource::LoadTexture(tile_node);
  auto v =
      static_cast<wz::Property<wz::WzVec2D> *>(tile_node->get_child(u"origin"))
          ->get();
  auto origin = SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)};
  SDL_FRect pos_rect = {
      .x = g_tile.pos.x - origin.x,
      .y = g_tile.pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  auto &camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_SetTextureAlphaMod(texture, g_tile.alpha);
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
  return true;
}

}  // namespace sdlms
