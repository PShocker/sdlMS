#include "src/client/system/render/reactor_render_system.h"
#include <SDL3/SDL_rect.h>
#include "src/client/game/game_reactor.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <wz/Property.h>
#include <wz/Wz.h>
#include <string>
namespace sdlms {


bool ReactorRenderSystem::Render(GameReactor &g_reactor) {
  auto node = WzResource::reactor_->find(g_reactor.id + u".img");
  if (node->get_child(std::to_string(g_reactor.state))) {
    node = node->get_child(std::to_string(g_reactor.state));
    if (!g_reactor.hit) {
      node = node->get_child(std::to_string(g_reactor.ani_index));
    } else {
      node = node->find("hit/" + std::to_string(g_reactor.ani_index));
    }
    if (node == nullptr) {
      return true;
    }
    if (node->type == wz::Type::UOL) {
      node = static_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }
    auto t = WzResource::LoadTexture(node);
    SDL_FRect pos_rect;
    auto o = WzResource::LoadFpoint(node->get_child(u"origin"));
    pos_rect.x = g_reactor.pos.x - o.x;
    pos_rect.y = g_reactor.pos.y - o.y;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    auto &camera = CameraGameInstance::camera_;
    if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
      pos_rect.x -= camera.x;
      pos_rect.y -= camera.y;
      SDL_SetTextureAlphaMod(t, g_reactor.alpha);
      SDL_RenderTextureRotated(Window::renderer_, t, nullptr, &pos_rect, 0,
                               nullptr, (SDL_FlipMode)g_reactor.f);
    }
  }
  return true;
}

}  // namespace sdlms
