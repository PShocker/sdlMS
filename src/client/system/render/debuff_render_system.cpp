#include "debuff_render_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/trap_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include <string>

void DebuffRenderSystem::render(GameDebuff::DebuffType type,
                                  SDL_FPoint pos) {
  wz::Node *n;
  switch (type) {
  case GameDebuff::dizz: {
    static auto node = WzResource::skill->find(u"AttackType.img/1001/level/1");
    n = node;
    break;
  }
  case GameDebuff::slow: {
    static auto node = WzResource::skill->find(u"AttackType.img/1002/level/1");
    n = node;
    break;
  }
  default: {
    break;
  }
  }
  auto delays = TrapGameInstance::load_animate_delays(n);
  auto data = TrapGameInstance::load_animate_index(delays);
  auto index = data.index;
  n = n->get_child(std::to_string(index));
  auto t = WzResource::load_texture(n);
  auto origin = WzResource::load_fpoint(n->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = pos.x - origin.x,
      .y = pos.y - origin.y,
      .w = static_cast<float>(t->w),
      .h = static_cast<float>(t->h),
  };
  auto &camera = CameraGameInstance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  }
}

void DebuffRenderSystem::render_character(GameCharacter &character) {
  for (auto k : character.debuffs | std::views::keys) {
    render(k, character.pos);
  }
}

void DebuffRenderSystem::render_mob(GameMob &mob) {
  for (auto k : mob.debuffs | std::views::keys) {
    render(k, mob.pos);
  }
}