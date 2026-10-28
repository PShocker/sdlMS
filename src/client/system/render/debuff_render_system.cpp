#include "debuff_render_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/trap_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include <string>
namespace sdlms {


void DebuffRenderSystem::Render(GameDebuff::DebuffType type,
                                  SDL_FPoint pos) {
  wz::Node *n;
  switch (type) {
  case GameDebuff::DebuffType::kDizz: {
    static auto node = WzResource::skill_->find(u"AttackType.img/1001/level/1");
    n = node;
    break;
  }
  case GameDebuff::DebuffType::kSlow: {
    static auto node = WzResource::skill_->find(u"AttackType.img/1002/level/1");
    n = node;
    break;
  }
  default: {
    break;
  }
  }
  auto delays = TrapGameInstance::LoadAnimateDelays(n);
  auto data = TrapGameInstance::LoadAnimateIndex(delays);
  auto index = data.index;
  n = n->get_child(std::to_string(index));
  auto t = WzResource::LoadTexture(n);
  auto origin = WzResource::LoadFpoint(n->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = pos.x - origin.x,
      .y = pos.y - origin.y,
      .w = static_cast<float>(t->w),
      .h = static_cast<float>(t->h),
  };
  auto &camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
  }
}

void DebuffRenderSystem::RenderCharacter(GameCharacter &character) {
  for (auto k : character.debuffs | std::views::keys) {
    Render(k, character.pos);
  }
}

void DebuffRenderSystem::RenderMob(GameMob &mob) {
  for (auto k : mob.debuffs | std::views::keys) {
    Render(k, mob.pos);
  }
}

}  // namespace sdlms
