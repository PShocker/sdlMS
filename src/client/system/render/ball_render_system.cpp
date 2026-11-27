#include "src/client/system/render/ball_render_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_render.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <string>

namespace sdlms {


bool BallRenderSystem::Render(GameBall& g_ball) {
  auto now = Window::dt_time_;
  if (g_ball.delay >= now) {
    return false;
  }
  // 资源缺失（有些武器的攻击弹道不在 Skill.wz/Item.wz）时直接不画，
  // 不能解引用空节点，否则攻击时崩溃。
  wz::Node* n = nullptr;
  if (WzResource::skill_ != nullptr) {
    n = WzResource::skill_->find(g_ball.path);
  }
  if (n == nullptr && WzResource::item_ != nullptr) {
    n = WzResource::item_->find(g_ball.path);
  }
  if (n == nullptr) {
    return false;
  }
  auto index = std::to_string(g_ball.ani_index);
  auto* texture_node = n->get_child(index);
  if (texture_node == nullptr) {
    return false;
  }
  auto* origin_node = texture_node->get_child(u"origin");
  if (origin_node == nullptr || origin_node->type != wz::Type::Vector2D) {
    return false;
  }
  SDL_Texture* t = WzResource::LoadTexture(texture_node);
  if (t == nullptr) {
    return false;
  }
  auto v = static_cast<wz::Property<wz::WzVec2D>*>(origin_node)->get();
  auto origin = SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)};
  SDL_FRect pos_rect = {
      .x = g_ball.pos.x - origin.x,
      .y = g_ball.pos.y - origin.y,
      .w = static_cast<float>(t->w),
      .h = static_cast<float>(t->h),
  };
  auto& camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    auto flip = g_ball.flip;
    SDL_RenderTextureRotated(Window::renderer_, t, nullptr, &pos_rect,
                             g_ball.rotate, nullptr, (SDL_FlipMode)flip);
  }
  return true;
}

}  // namespace sdlms
