#include "effect_render_system.h"
#include "src/client/game/game_effect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

void effect_render_system::render_effect(SDL_FPoint pos,
                                         game_effect &g_effect) {
  auto current_time = window::dt_time;
  if (current_time <= g_effect.delay) {
    return;
  }
  wz::Node *node;
  switch (g_effect.lvl) {
  case 0: {
    node = wz_resource::character->find(g_effect.id);
    break;
  }
  case 1: {
    node = wz_resource::effect->find(g_effect.id);
    break;
  }
  case 2: {
    node = wz_resource::item->find(g_effect.id);
    break;
  }
  }
  auto pos2 = g_effect.pos.value();
  auto index = std::to_string(g_effect.index);
  auto texture_node = node->get_child(index);
  auto texture = wz_resource::load_texture(texture_node);
  auto origin = wz_resource::load_fpoint(texture_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = pos.x - origin.x + pos2.x,
      .y = pos.y - origin.y + pos2.y,
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
  enum damage_type {
    n_0,
    n_1,
    n_2,
    n_3,
    n_4,
    n_5,
    n_6,
    n_7,
    n_8,
    n_9,
    miss,
    guard,
  };
  auto current_time = window::dt_time;
  if (current_time <= g_effect.delay) {
    return;
  }
  if (g_effect.z == false) {
    return;
  }
  static auto red = wz_resource::effect->find(u"BasicEff.img/NoRed0");
  static auto viole = wz_resource::effect->find(u"BasicEff.img/NoViolet0");
  static auto blue = wz_resource::effect->find(u"BasicEff.img/NoBlue0");

  const std::vector<std::u16string> damage_vector = {
      u"0", u"1", u"2", u"3", u"4",    u"5",
      u"6", u"7", u"8", u"9", u"Miss", u"guard",
  };
  auto &camera = camera_game_instance::camera;
  auto d_data = std::any_cast<damage_data>(g_effect.data);
  auto num_str = std::to_string(d_data.num);
  uint32_t w = 0;
  wz::Node *node;
  switch (d_data.type) {
  case damage_data::r: {
    node = red;
    break;
  }
  case damage_data::b: {
    node = blue;
    break;
  }
  case damage_data::v: {
    node = viole;
    break;
  }
  default: {
    break;
  }
  }
  for (auto n : num_str) {
    auto texture_node = node->get_child(std::string{n});
    auto texture = wz_resource::load_texture(texture_node);
    auto origin = wz_resource::load_fpoint(texture_node->get_child(u"origin"));
    auto x = pos.x - origin.x + w;
    auto y = pos.y - origin.y - g_effect.index * texture->h - 32 -
             (255 - g_effect.alpha) / 5;
    SDL_FRect pos_rect = {
        .x = x,
        .y = y,
        .w = static_cast<float>(texture->w),
        .h = static_cast<float>(texture->h),
    };
    if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
      pos_rect.x -= camera.x;
      pos_rect.y -= camera.y;
      SDL_SetTextureAlphaMod(texture, g_effect.alpha);
      SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
    }
    w += texture->w - 3;
  }
}

void effect_render_system::render_ski_use(SDL_FPoint pos, game_effect &g_effect,
                                          bool flip) {
  auto ski_node = skill_game_instance::load_ski_node(g_effect.id);
  if (!ski_node->get_child(u"effect")) {
    return;
  }
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

void effect_render_system::render_ski_hit(SDL_FPoint pos, game_effect &g_effect,
                                          bool flip) {
  auto now = window::dt_time;
  if (g_effect.delay >= now) {
    return;
  }
  auto ski_node = skill_game_instance::load_ski_node(g_effect.id);
  if (ski_node->find(u"hit/0")) {
    ski_node = ski_node->find(u"hit/0");
  } else {
    ski_node =
        skill_game_instance::load_ski_level_node(g_effect.id, g_effect.lvl);
    ski_node = ski_node->find(u"hit/0");
  }
  auto index = std::to_string(g_effect.index);
  auto texture_node = ski_node->get_child(index);
  auto texture = wz_resource::load_texture(texture_node);
  if (texture_node->type == wz::Type::UOL) {
    texture_node =
        static_cast<wz::Property<wz::WzUOL> *>(texture_node)->get_uol();
  }
  auto origin = wz_resource::load_fpoint(texture_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = pos.x - origin.x + g_effect.pos->x,
      .y = pos.y - origin.y + g_effect.pos->y,
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

bool effect_render_system::render_custom(SDL_FPoint pos, game_effect &g_effect,
                                         bool flip) {
  auto skis = skill_game_instance::skis();
  if (skis.contains(g_effect.id)) {
    return skis.at(g_effect.id).effect(pos, g_effect, flip);
  }
  return false;
}

bool effect_render_system::render(SDL_FPoint pos, game_effect &g_effect,
                                  bool flip) {
  bool r = true;
  switch (g_effect.type) {
  case game_effect::effect_type::effect: {
    render_effect(pos, g_effect);
    break;
  }
  case game_effect::effect_type::damage: {
    render_damage(pos, g_effect);
    break;
  }
  case game_effect::effect_type::skill_use: {
    render_ski_use(pos, g_effect, flip);
    break;
  }
  case game_effect::effect_type::skill_hit: {
    render_ski_hit(pos, g_effect, flip);
    break;
  }
  case game_effect::effect_type::skill_custom: {
    r = render_custom(pos, g_effect, flip);
    break;
  }
  default: {
    break;
  }
  }

  return r;
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
  auto &effect = g_character->effect;
  for (auto it = effect.begin(); it != effect.end();) {
    auto &e = *it;
    if (e.z.has_value() && !e.z.value()) {
      auto r = render(g_character->pos, e, g_character->flip);
      if (r == false) {
        it = effect.erase(it); // erase返回下一个迭代器
        continue;
      }
    }
    ++it; // 只有不删除时才前进
  }
  return true;
}

bool effect_render_system::render(game_effect &g_effect) {
  const auto &pos = g_effect.pos.value();
  const auto &flip = g_effect.flip.value();
  effect_render_system::render(pos, g_effect, flip);
  return true;
}
