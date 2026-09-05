#include "character_render_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "chatballoon_render_system.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/effect_render_system.h"
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
  if (g_character.coat_deco.has_value()) {
    const auto &coat =
        character_game_instance::avatar_data.at(g_character.coat_deco->id);
    render_parts.emplace(coat.islot, &coat);
  } else if (g_character.coat.has_value()) {
    const auto &coat =
        character_game_instance::avatar_data.at(g_character.coat->id);
    render_parts.emplace(coat.islot, &coat);
  } else if (!(g_character.longcoat.has_value() ||
               g_character.longcoat_deco.has_value())) {
    const auto &coat = character_game_instance::avatar_data.at(u"01040000");
    render_parts.emplace(coat.islot, &coat);
  }

  if (g_character.cap_deco.has_value()) {
    const auto &cap =
        character_game_instance::avatar_data.at(g_character.cap_deco->id);
    render_parts.emplace(cap.islot, &cap);
  } else if (g_character.cap.has_value()) {
    const auto &cap =
        character_game_instance::avatar_data.at(g_character.cap->id);
    render_parts.emplace(cap.islot, &cap);
  }

  bool weapon_deco = false;
  if (g_character.weapon_deco.has_value()) {
    if (g_character.weapon.has_value()) {
      std::u16string sub = g_character.weapon->id.substr(2, 2);
      std::u16string deco_val = g_character.weapon_deco->id + u"/" + sub;
      if (character_game_instance::avatar_data.contains(deco_val)) {
        weapon_deco = true;
      }
    }
  }
  if (weapon_deco) {
    std::u16string sub = g_character.weapon->id.substr(2, 2);
    std::u16string deco_val = g_character.weapon_deco->id + u"/" + sub;
    const auto &weapon = character_game_instance::avatar_data.at(deco_val);
    render_parts.emplace(weapon.islot, &weapon);
  } else if (g_character.weapon.has_value()) {
    const auto &weapon =
        character_game_instance::avatar_data.at(g_character.weapon->id);
    render_parts.emplace(weapon.islot, &weapon);
  }
  if (g_character.shield_deco.has_value()) {
    const auto &shield =
        character_game_instance::avatar_data.at(g_character.shield_deco->id);
    render_parts.emplace(shield.islot, &shield);
  } else if (g_character.shield.has_value()) {
    const auto &shield =
        character_game_instance::avatar_data.at(g_character.shield->id);
    render_parts.emplace(shield.islot, &shield);
  }
  if (g_character.pant_deco.has_value()) {
    const auto &pant =
        character_game_instance::avatar_data.at(g_character.pant_deco->id);
    render_parts.emplace(pant.islot, &pant);
  } else if (g_character.pant.has_value()) {
    const auto &pant =
        character_game_instance::avatar_data.at(g_character.pant->id);
    render_parts.emplace(pant.islot, &pant);
  } else if (!(g_character.longcoat.has_value() ||
               g_character.longcoat_deco.has_value())) {
    const auto &pant = character_game_instance::avatar_data.at(u"01060000");
    render_parts.emplace(pant.islot, &pant);
  }

  if (g_character.glove_deco.has_value()) {
    const auto &glove =
        character_game_instance::avatar_data.at(g_character.glove_deco->id);
    render_parts.emplace(glove.islot, &glove);
  } else if (g_character.glove.has_value()) {
    const auto &glove =
        character_game_instance::avatar_data.at(g_character.glove->id);
    render_parts.emplace(glove.islot, &glove);
  }
  if (g_character.shoes_deco.has_value()) {
    const auto &shoes =
        character_game_instance::avatar_data.at(g_character.shoes_deco->id);
    render_parts.emplace(shoes.islot, &shoes);
  } else if (g_character.shoes.has_value()) {
    const auto &shoes =
        character_game_instance::avatar_data.at(g_character.shoes->id);
    render_parts.emplace(shoes.islot, &shoes);
  }
  std::u16string action = g_character.action;
  auto action_index = g_character.action_index;
  if (character_game_instance::extern_action.contains(action)) {
    action_index =
        character_game_instance::extern_action[action][action_index].frame;
    action =
        character_game_instance::extern_action[action][action_index].action;
  }

  auto face = character_game_instance::face_data.at(g_character.face.id)
                  .data.at(g_character.face.action);
  if (character_game_instance::bone_data.at(action)[action_index].face) {
    face.data.at(action)[action_index] = {
        face.data.at(action)[action_index][g_character.face.index],
    };
    render_parts.emplace(face.islot, &face);
  }

  std::flat_set<std::u16string> smaps_cover;
  for (const auto &index : character_game_instance::zmap2) {
    if (render_parts.contains(index)) {
      auto r = render_parts.at(index);
      if (!r->data.contains(action)) {
        continue;
      }
      const auto &pts = r->data.at(action)[action_index];
      std::flat_set<std::u16string> smaps_inter;
      for (const auto &pt : pts) {
        // 获取默认的smap
        const auto &smaps = character_game_instance::smap.at(pt.z);
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
          if (!g_character.hide_part.contains(pt.name)) {
            renders.emplace(pt.z, &pt);
          }
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

        // 获取缩放倍数
        float scale = g_character.scale;

        // 应用缩放 - 只有尺寸和相对偏移乘以 scale，位置不变
        SDL_FRect pos_rect = {
            .x = g_character.pos.x + avatar->pos.x * scale - origin.x * scale,
            .y = g_character.pos.y + avatar->pos.y * scale - origin.y * scale,
            .w = static_cast<float>(texture->w * scale),
            .h = static_cast<float>(texture->h * scale),
        };

        // 翻转处理
        if (g_character.flip == 1) {
          // 以角色位置为中心进行水平翻转
          float centerX = g_character.pos.x;
          // 计算翻转后的位置
          pos_rect.x = centerX - (pos_rect.x - centerX) - pos_rect.w;
        }

        auto &camera = camera_game_instance::camera;
        // 相机不缩放（因为 pos_rect 的位置没缩放）
        if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
          pos_rect.x -= camera.x;
          pos_rect.y -= camera.y;
          SDL_SetTextureColorMod(texture, g_character.color.r,
                                 g_character.color.g, g_character.color.b);
          SDL_SetTextureAlphaMod(texture, g_character.color.a);
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
      weapon_type == equip_game_instance::weapon_type::WAND) {
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
  freetype::load_aligned(true);
  freetype::load_size(13);
  auto w = freetype::load_w(g_character.name);
  auto h = freetype::load_lh();

  auto x = g_character.pos.x;
  auto y = g_character.pos.y;

  std::u16string nametag = u"";
  for (auto r : {g_character.ring0_deco, g_character.ring1_deco}) {
    if (r.has_value()) {
      auto info = equip_game_instance::load_equip_info(r->id);
      if (info->get_child(u"nameTag")) {
        auto tmp = static_cast<wz::Property<int> *>(info->get_child(u"nameTag"))
                       ->get();
        auto tmp2 = std::to_string(tmp);
        nametag = {tmp2.begin(), tmp2.end()};
        break;
      }
    }
  }
  if (nametag.empty()) {
    SDL_FRect rect;
    rect.w = w + 4;
    rect.h = h + 4;
    rect.x = x - rect.w / 2;
    rect.y = y;
    if (SDL_HasRectIntersectionFloat(&rect, &camera)) {
      rect.x -= camera.x;
      rect.y -= camera.y;
      SDL_SetRenderDrawBlendMode(window::renderer, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(window::renderer, 0, 0, 0, 178);
      SDL_RenderFillRect(window::renderer, &rect);
      freetype::load_color(255, 255, 255, 255);
      freetype::draw_line(g_character.name, x - camera.x - w / 2, y - camera.y);
    }
  } else {
    static auto nametag_node = wz_resource::ui->find(u"NameTag.img");
    auto texture_node = nametag_node->get_child(nametag);
    auto texture_w = wz_resource::load_texture(texture_node->get_child("w"));
    auto texture_c = wz_resource::load_texture(texture_node->get_child("c"));
    auto texture_e = wz_resource::load_texture(texture_node->get_child("e"));

    auto ow = wz_resource::load_fpoint(texture_node->find(u"w/origin"));
    auto oc = wz_resource::load_fpoint(texture_node->find(u"c/origin"));

    auto a = (float)texture_c->w;
    auto b = w;
    auto result = a * ((b + a - 1) / a); // 整数向上取整技巧

    SDL_FRect rect;
    rect.w = result + texture_w->w + texture_e->w;
    rect.h = texture_c->h;
    rect.x = x - rect.w / 2;
    rect.y = y;
    if (SDL_HasRectIntersectionFloat(&rect, &camera)) {
      rect.x -= camera.x;
      rect.y -= camera.y;
      rect.x = (int)rect.x;
      rect.y = (int)(rect.y - (ow.y - oc.y));
      rect.w = texture_w->w;
      rect.h = texture_w->h;
      SDL_RenderTexture(window::renderer, texture_w, nullptr, &rect);
      rect.x += (int)texture_w->w;
      rect.y = (int)(y - camera.y);
      rect.w = result;
      rect.h = texture_c->h;
      SDL_RenderTextureTiled(window::renderer, texture_c, nullptr, 1, &rect);
      rect.x += result;
      rect.w = texture_e->w;
      SDL_RenderTexture(window::renderer, texture_e, nullptr, &rect);
      auto clr =
          static_cast<wz::Property<int32_t> *>(texture_node->get_child(u"clr"))
              ->get();
      uint8_t a, r, g, b;
      a = (clr >> 24) & 0xFF;
      r = (clr >> 16) & 0xFF;
      g = (clr >> 8) & 0xFF;
      b = clr & 0xFF;
      freetype::load_color(r, g, b, a);
      freetype::draw_line(g_character.name, rect.x - result, y - camera.y);
    }
  }
  return;
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

bool character_render_system::render_effect_back(game_character &g_character) {
  return effect_render_system::render_character_back(&g_character);
}

void character_render_system::render_chatballoon(game_character &g_character) {
  if (!g_character.chatballoon.has_value()) {
    return;
  }
  auto chatballoon = g_character.chatballoon.value();
  auto name = g_character.name + u":";
  chatballoon.text = name + chatballoon.text;
  freetype::load_size(chatballoon.size);
  SDL_FPoint pos{
      .x = g_character.pos.x,
      .y = g_character.pos.y - 70,
  };
  chatballoon_render_system::render(chatballoon, pos);
}

void character_render_system::render_chair(game_character &g_character) {
  auto chair_id = g_character.chair->id;
  auto chair_node =
      wz_resource::item->find(u"Install/0301.img/" + chair_id + u"/effect");
  chair_node =
      chair_node->get_child(std::to_string(g_character.chair->ani_index));
  auto t = wz_resource::load_texture(chair_node);
  auto origin = wz_resource::load_fpoint(chair_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = g_character.pos.x - origin.x,
      .y = g_character.pos.y - origin.y - (t->h - origin.y),
      .w = static_cast<float>(t->w),
      .h = static_cast<float>(t->h),
  };
  if (g_character.flip == 1) {
    pos_rect.x = g_character.pos.x;
    pos_rect.x = (pos_rect.x - (t->w - origin.x));
  }
  auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTextureRotated(window::renderer, t, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)g_character.flip);
  }
}

void character_render_system::render_chair(game_character &g_character,
                                           bool front) {
  if (!g_character.chair.has_value()) {
    return;
  }

  auto chair_id = g_character.chair->id;
  auto z_node =
      wz_resource::item->find(u"Install/0301.img/" + chair_id + u"/effect/z");
  auto z = static_cast<wz::Property<int> *>(z_node)->get();

  if ((front && z < 0) || (!front && z > 0)) {
    render_chair(g_character);
  }
}

void character_render_system::render_morph(game_character &g_character) {
  auto node = wz_resource::morph->find(g_character.morph + u".img");
  auto action = character_logic_system::load_morph_type(g_character);
  node = node->get_child(action);
  node = node->get_child(std::to_string(g_character.action_index));
  auto t = wz_resource::load_texture(node);
  auto origin = wz_resource::load_fpoint(node->get_child(u"origin"));
  SDL_FRect pos_rect{
      .x = g_character.pos.x - origin.x,
      .y = g_character.pos.y - origin.y - (t->h - origin.y),
      .w = static_cast<float>(t->w),
      .h = static_cast<float>(t->h),
  };
  if (g_character.flip == 1) {
    pos_rect.x = g_character.pos.x;
    pos_rect.x = (pos_rect.x - (t->w - origin.x));
  }
  auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_SetTextureColorMod(t, g_character.color.r, g_character.color.g,
                           g_character.color.b);
    SDL_SetTextureAlphaMod(t, g_character.color.a);
    SDL_RenderTextureRotated(window::renderer, t, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)g_character.flip);
  }
}

bool character_render_system::render(game_character &g_character) {
  if (g_character.morph.empty()) {
    render_chair(g_character, true);
    render_afterimage(g_character);
    render_tomb(g_character);
    render_character(g_character);
    render_chair(g_character, false);
    render_effect_back(g_character);
  } else {
    render_morph(g_character);
    render_effect_back(g_character);
  }
  render_nametag(g_character);
  // auto r = character_logic_system::load_rect(g_character);
  // auto &camera = camera_game_instance::camera;
  // r.x -= camera.x;
  // r.y -= camera.y;
  // SDL_RenderFillRect(window::renderer, &r);

  // auto pos = g_character.pos;
  // auto &camera = camera_game_instance::camera;
  // pos.x -= camera.x;
  // pos.y -= camera.y;
  // pos.y -= 30;
  // SDL_FPoint p1{pos.x - 350, pos.y - 100};
  // SDL_FPoint p2{pos.x - 350, pos.y + 100};
  // SDL_SetRenderDrawColor(window::renderer, 255, 0, 0, 255);
  // SDL_RenderLine(window::renderer, p1.x, p1.y, p2.x, p2.y);
  // SDL_RenderLine(window::renderer, p1.x, p1.y, pos.x, pos.y);
  // SDL_RenderLine(window::renderer, p2.x, p2.y, pos.x, pos.y);
  return true;
}