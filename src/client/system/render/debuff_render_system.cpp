#include "debuff_render_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/trap_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include <string>

void debuff_render_system::render(game_debuff::debuff_type type,
                                  SDL_FPoint pos) {
  wz::Node *n;
  switch (type) {
  case game_debuff::dizz: {
    static auto node = wz_resource::skill->find(u"AttackType.img/1001/level/1");
    n = node;
    break;
  }
  case game_debuff::slow: {
    static auto node = wz_resource::skill->find(u"AttackType.img/1002/level/1");
    n = node;
    break;
  }
  default: {
    break;
  }
  }
  auto delays = trap_game_instance::load_animate_delays(n);
  auto data = trap_game_instance::load_animate_index(delays);
  auto index = data.index;
  n = n->get_child(std::to_string(index));
  auto t = wz_resource::load_texture(n);
  auto origin = wz_resource::load_fpoint(n->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = pos.x - origin.x,
      .y = pos.y - origin.y,
      .w = static_cast<float>(t->w),
      .h = static_cast<float>(t->h),
  };
  auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  }
}

void debuff_render_system::render_character(game_character &character) {
  for (auto k : character.debuffs | std::views::keys) {
    render(k, character.pos);
  }
}

void debuff_render_system::render_mob(game_mob &mob) {
  for (auto k : mob.debuffs | std::views::keys) {
    render(k, mob.pos);
  }
}