#include "effect_render_system.h"
#include "src/client/game/game_effect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <chrono>
#include <string>

void effect_render_system::render_afterimage(SDL_FPoint pos,
                                             game_effect &g_effect) {
  auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
  if (current_time <= g_effect.delay) {
    return;
  }
  auto atk_pos = g_effect.pos.value();
  std::u16string type = g_effect.id;
  auto hit_node = wz_resource::character->find(u"Afterimage/hit.img/" + type);
  auto index = std::to_string(g_effect.index);
  auto texture_node = hit_node->get_child(index);
  auto texture = wz_resource::load_texture(texture_node);
  auto origin = wz_resource::load_fpoint(texture_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = pos.x - origin.x + atk_pos.x,
      .y = pos.y - origin.y + atk_pos.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
}

bool effect_render_system::render(SDL_FPoint pos, game_effect &g_effect) {
  switch (g_effect.type) {
  case game_effect::effect_type::afterimage: {
    render_afterimage(pos, g_effect);
    break;
  }
  case game_effect::effect_type::custom: {
    break;
  }
  default: {
    break;
  }
  }

  return true;
}

bool effect_render_system::render_mob_back(game_mob &g_mob) {
  if (!effect_game_instance::m_effect.contains(g_mob.index)) {
    return false;
  }
  auto &v = effect_game_instance::m_effect.at(g_mob.index);
  for (auto &e : v) {
    if (e.z.has_value() && !e.z.value()) {
      render(g_mob.pos, e);
    }
  }
  return true;
}