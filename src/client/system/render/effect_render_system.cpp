#include "effect_render_system.h"
#include "src/client/game/game_effect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <array>
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

void effect_render_system::render_damage(SDL_FPoint pos,
                                         game_effect &g_effect) {
  static auto red0_n = wz_resource::effect->find(u"BasicEff.img/NoRed0");
  const static std::array red0 = {
      wz_resource::load_texture(red0_n->get_child(u"0")),
      wz_resource::load_texture(red0_n->get_child(u"1")),
      wz_resource::load_texture(red0_n->get_child(u"2")),
      wz_resource::load_texture(red0_n->get_child(u"3")),
      wz_resource::load_texture(red0_n->get_child(u"4")),
      wz_resource::load_texture(red0_n->get_child(u"5")),
      wz_resource::load_texture(red0_n->get_child(u"6")),
      wz_resource::load_texture(red0_n->get_child(u"7")),
      wz_resource::load_texture(red0_n->get_child(u"8")),
      wz_resource::load_texture(red0_n->get_child(u"9")),
  };

  static auto red1_n = wz_resource::effect->find(u"BasicEff.img/NoRed1");
  const static std::array red1 = {
      wz_resource::load_texture(red1_n->get_child(u"0")),
      wz_resource::load_texture(red1_n->get_child(u"1")),
      wz_resource::load_texture(red1_n->get_child(u"2")),
      wz_resource::load_texture(red1_n->get_child(u"3")),
      wz_resource::load_texture(red1_n->get_child(u"4")),
      wz_resource::load_texture(red1_n->get_child(u"5")),
      wz_resource::load_texture(red1_n->get_child(u"6")),
      wz_resource::load_texture(red1_n->get_child(u"7")),
      wz_resource::load_texture(red1_n->get_child(u"8")),
      wz_resource::load_texture(red1_n->get_child(u"9")),
  };

  static auto red2_n = wz_resource::effect->find(u"BasicEff.img/NoRed2");
  const static std::array red2 = {
      wz_resource::load_texture(red2_n->get_child(u"0")),
      wz_resource::load_texture(red2_n->get_child(u"1")),
      wz_resource::load_texture(red2_n->get_child(u"2")),
      wz_resource::load_texture(red2_n->get_child(u"3")),
      wz_resource::load_texture(red2_n->get_child(u"4")),
      wz_resource::load_texture(red2_n->get_child(u"5")),
      wz_resource::load_texture(red2_n->get_child(u"6")),
      wz_resource::load_texture(red2_n->get_child(u"7")),
      wz_resource::load_texture(red2_n->get_child(u"8")),
      wz_resource::load_texture(red2_n->get_child(u"9")),
  };
  // int32_t num = std::any_cast<int32_t>(g_effect.data);
}

void effect_render_system::render_skill_use(SDL_FPoint pos,
                                            game_effect &g_effect, bool flip) {
  auto ski_node = skill_game_instance::load_skill_node(g_effect.id);
  ski_node = ski_node->get_child(u"effect");
  auto index = std::to_string(g_effect.index);
  auto texture_node = ski_node->get_child(index);
  auto texture = wz_resource::load_texture(texture_node);
  auto origin = wz_resource::load_fpoint(texture_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = pos.x - origin.x,
      .y = pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  if (flip == 1) {
    pos_rect.x = pos.x;
    pos_rect.x = (pos_rect.x - (texture->w - origin.x));
  }
  auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTextureRotated(window::renderer, texture, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)flip);
  }
}

void effect_render_system::render_skill_hit(SDL_FPoint pos,
                                            game_effect &g_effect, bool flip) {
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();
  if (g_effect.delay >= now) {
    return;
  }
  auto ski_node = skill_game_instance::load_skill_node(g_effect.id);
  ski_node = ski_node->find(u"hit/0");
  auto index = std::to_string(g_effect.index);
  auto texture_node = ski_node->get_child(index);
  auto texture = wz_resource::load_texture(texture_node);
  if (texture_node->type == wz::Type::UOL) {
    texture_node =
        static_cast<wz::Property<wz::WzUOL> *>(texture_node)->get_uol();
  }
  auto origin = wz_resource::load_fpoint(texture_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = pos.x - origin.x,
      .y = pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  if (flip == 1) {
    pos_rect.x = pos.x;
    pos_rect.x = (pos_rect.x - (texture->w - origin.x));
  }
  auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTextureRotated(window::renderer, texture, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)flip);
  }
}

bool effect_render_system::render(SDL_FPoint pos, game_effect &g_effect,
                                  bool flip) {
  switch (g_effect.type) {
  case game_effect::effect_type::afterimage: {
    render_afterimage(pos, g_effect);
    break;
  }
  case game_effect::effect_type::damage: {
    // render_damage(pos, g_effect);
    break;
  }
  case game_effect::effect_type::skill_use: {
    render_skill_use(pos, g_effect, flip);
    break;
  }
  case game_effect::effect_type::skill_hit: {
    render_skill_hit(pos, g_effect, flip);
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
  auto &v = g_mob.effect;
  for (auto &e : v) {
    if (e.z.has_value() && !e.z.value()) {
      render(g_mob.pos, e, g_mob.flip);
    }
  }
  return true;
}

bool effect_render_system::render_character_back(game_character *g_character) {
  auto &v = g_character->effect;
  for (auto &e : v) {
    if (e.z.has_value() && !e.z.value()) {
      render(g_character->pos, e, g_character->flip);
    }
  }
  return true;
}