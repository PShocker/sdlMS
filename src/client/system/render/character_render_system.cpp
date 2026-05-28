#include "character_render_system.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "nametag_render_system.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
#include <vector>

bool character_render_system::render_character(game_character &g_character) {
  std::flat_multimap<std::u16string, const character_avatar *> renders;
  std::flat_map<std::u16string, const character_avatar_render *> render_parts;
  auto &camera = camera_game_instance::camera;
  const auto &head = character_game_instance::avatar_data.at(g_character.head);
  const auto &body = character_game_instance::avatar_data.at(g_character.body);
  const auto &hair = character_game_instance::avatar_data.at(g_character.hair);
  render_parts.emplace(head.islot, &head);
  render_parts.emplace(body.islot, &body);
  render_parts.emplace(hair.islot, &hair);
  if (g_character.coat.has_value()) {
    const auto &coat =
        character_game_instance::avatar_data.at(g_character.coat->id);
    render_parts.emplace(coat.islot, &coat);
  }
  if (g_character.cap.has_value()) {
    const auto &cap =
        character_game_instance::avatar_data.at(g_character.cap->id);
    render_parts.emplace(cap.islot, &cap);
  }
  if (g_character.weapon.has_value()) {
    const auto &weapon =
        character_game_instance::avatar_data.at(g_character.weapon->id);
    render_parts.emplace(weapon.islot, &weapon);
  }
  if (g_character.shield.has_value()) {
    const auto &shield =
        character_game_instance::avatar_data.at(g_character.shield->id);
    render_parts.emplace(shield.islot, &shield);
  }
  if (g_character.pant.has_value()) {
    const auto &pant =
        character_game_instance::avatar_data.at(g_character.pant->id);
    render_parts.emplace(pant.islot, &pant);
  }
  const std::u16string action = g_character.action;
  if (character_game_instance::bone_data.at(action)[g_character.action_index]
          .face) {
    const auto &face =
        character_game_instance::face_data.at(g_character.face.id)
            .data.at(u"default");
    render_parts.emplace(face.islot, &face);
  }

  std::flat_set<std::u16string> smaps_cover;
  for (const auto &index : character_game_instance::zmap2) {
    if (render_parts.contains(index)) {
      auto r = render_parts.at(index);
      if (!r->data.contains(action)) {
        continue;
      }
      const auto &pts = r->data.at(action)[g_character.action_index];
      std::flat_set<std::u16string> smaps_inter;
      for (const auto &pt : pts) {
        // 获取默认的smap
        auto smaps = character_game_instance::smap.at(pt.z);
        // 求交集
        std::flat_set<std::u16string> smaps_inter2;
        std::ranges::set_intersection(
            smaps, r->vslot, std::inserter(smaps_inter2, smaps_inter2.begin()));
        //  判断是否被占据
        bool part_cover = false;
        for (const auto &cover : smaps_cover) {
          if (smaps_inter2.contains(cover)) {
            part_cover = true;
            break;
          }
        }
        if (!part_cover) {
          // 如果装备部分没有遮挡,占据cover栏
          smaps_inter.insert_range(smaps_inter2);
          // 放入渲染队列里面
          renders.emplace(pt.z, &pt);
        }
      }
      smaps_cover.insert_range(smaps_inter);
    }
  }

  for (const auto &index : character_game_instance::zmap) {
    if (renders.contains(index)) {
      auto [first, last] = renders.equal_range(index);
      for (auto it = first; it != last; ++it) {
        const character_avatar *avatar = it->second;
        // 处理 avatar
        auto texture = avatar->texture;
        auto origin = avatar->origin;
        SDL_FRect pos_rect = {
            .x = g_character.pos.x + avatar->pos.x - origin.x,
            .y = g_character.pos.y + avatar->pos.y - origin.y,
            .w = static_cast<float>(texture->w),
            .h = static_cast<float>(texture->h),
        };
        if (g_character.flip == 1) {
          pos_rect.x = g_character.pos.x - avatar->pos.x;
          pos_rect.x = (pos_rect.x - (texture->w - origin.x));
        }
        auto &camera = camera_game_instance::camera;
        if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
          pos_rect.x -= camera.x;
          pos_rect.y -= camera.y;
          if (&g_character == &character_game_instance::self) {
            auto cooldown = character_logic_system::self_invincible_cooldown;
            if (cooldown >= window::dt_now) {
              auto delta = cooldown - window::dt_now;
              bool dark = (delta % 200) > 100;
              Uint8 color = dark ? 128 : 255;
              SDL_SetTextureColorMod(texture, color, color, color);
            } else {
              SDL_SetTextureColorMod(texture, 255, 255, 255);
            }
          } else {
            SDL_SetTextureColorMod(texture, 255, 255, 255);
          }
          SDL_RenderTextureRotated(window::renderer, texture, nullptr,
                                   &pos_rect, 0, nullptr,
                                   (SDL_FlipMode)g_character.flip);
        }
      }
    }
  }
  return true;
}

bool character_render_system::render_afterimage(game_character &g_character) {
  // 如果是法杖，而且是技能状态，不显示
  auto action_type = character_logic_system::load_action_type(g_character);
  auto weapon_type = equip_game_instance::load_weapon_type(g_character);
  if (action_type == character_logic_system::action_enum::skill &&
      weapon_type == equip_game_instance::WAND) {
    return false;
  }
  // 通常情况下,attack和skill都会有afterimag的渲染
  auto r = afterimage_game_instance::load_data(g_character);
  if (r.has_value()) {
    auto texture = r.value().texture;
    auto alpha = r.value().alpha;
    auto origin = r.value().origin;
    auto &camera = camera_game_instance::camera;
    SDL_FRect pos_rect = {
        .x = g_character.pos.x - origin.x,
        .y = g_character.pos.y - origin.y,
        .w = static_cast<float>(texture->w),
        .h = static_cast<float>(texture->h),
    };
    if (g_character.flip == 1) {
      pos_rect.x = g_character.pos.x;
      pos_rect.x = (pos_rect.x - (texture->w - origin.x));
    }
    if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
      pos_rect.x -= camera.x;
      pos_rect.y -= camera.y;
      SDL_SetTextureAlphaMod(texture, alpha);
      SDL_RenderTextureRotated(window::renderer, texture, nullptr, &pos_rect, 0,
                               nullptr, (SDL_FlipMode)g_character.flip);
    }
  }
  return true;
}

void character_render_system::render_nametag(game_character &g_character) {
  const auto &camera = camera_game_instance::camera;
  for (auto &n : g_character.nametags) {
    nametag_render_system::render(n, g_character.pos);
  }
}

void character_render_system::render_tomb(game_character &g_character) {
  if (!g_character.tomb.has_value()) {
    return;
  }
  auto tomb = g_character.tomb.value();
  static auto tomb_node = wz_resource::effect->find(u"Tomb.img");
  auto type = tomb.ani_type;
  auto ani_node = tomb_node->get_child(type);
  auto ani_index = std::to_string(tomb.ani_index);
  auto texture_node = ani_node->get_child(ani_index);
  if (texture_node->type == wz::Type::UOL) {
    texture_node =
        static_cast<wz::Property<wz::WzUOL> *>(texture_node)->get_uol();
  }
  auto texture = wz_resource::load_texture(texture_node);
  auto origin = wz_resource::load_fpoint(texture_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = tomb.pos.x - origin.x,
      .y = tomb.pos.y - origin.y,
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

bool character_render_system::render(game_character &g_character) {
  render_afterimage(g_character);
  render_tomb(g_character);
  render_character(g_character);
  render_nametag(g_character);
  // auto r = character_logic_system::load_rect(g_character);
  // auto &camera = camera_game_instance::camera;
  // r.x -= camera.x;
  // r.y -= camera.y;
  // SDL_RenderFillRect(window::renderer, &r);
  return true;
}