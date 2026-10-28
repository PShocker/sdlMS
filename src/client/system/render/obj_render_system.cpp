#include "src/client/system/render/obj_render_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_render.h>
#include <wz/Property.h>
#include <wz/Wz.h>
namespace sdlms {


bool ObjRenderSystem::Render(GameObj& g_obj) {
  auto obj_node = WzResource::map_->find(g_obj.path);
  auto index = std::to_string(g_obj.ani_index);
  obj_node = obj_node->get_child(index);
  if (obj_node->type == wz::Type::UOL) {
    obj_node = static_cast<wz::Property<wz::WzUOL>*>(obj_node)->get_uol();
  }
  auto texture = WzResource::LoadTexture(obj_node);
  auto v =
      static_cast<wz::Property<wz::WzVec2D>*>(obj_node->get_child(u"origin"))
          ->get();
  auto origin = SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)};
  SDL_FRect pos_rect = {
      .x = g_obj.pos.x - origin.x,
      .y = g_obj.pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  auto& camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_SetTextureAlphaMod(texture, g_obj.alpha);
    auto flip = g_obj.flip;
    SDL_RenderTextureRotated(Window::renderer_, texture, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)flip);
  }

  return true;
}

void ObjRenderSystem::RenderClock(SDL_FRect& r) {}

}  // namespace sdlms
