#include "src/client/system/render/character_render_system.h"

#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/chatballoon_render_system.h"
#include "src/client/system/render/effect_render_system.h"
#include "src/client/system/ui/craft_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <wz/Property.h>
#include <wz/Wz.h>

#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
#include <vector>


namespace sdlms {

bool CharacterRenderSystem::RenderCharacter(GameCharacter& g_character) {
  std::flat_multimap<std::u16string, const CharacterAvatar*> renders;
  std::flat_map<std::u16string, const CharacterAvatarRender*> render_parts;
  auto& camera = CameraGameInstance::camera_;
  const auto& avatar_data = CharacterGameInstance::avatar_data_;

  // 把某个部位加进渲染表：avatar 缓存里没有就跳过（避免 .at 抛异常）。
  auto add_part = [&render_parts, &avatar_data](const std::u16string& id,
                                                const char16_t* slot = nullptr) {
    if (id.empty()) {
      return false;
    }
    auto it = avatar_data.find(id);
    if (it == avatar_data.end() || it->second.data.empty()) {
      return false;
    }
    // 有些装备的 islot 是组合值（如武器 WpSi、长袍 MaPn），要显式指定槽位，
    // 否则 SplitIslot 可能落到错误的分组上（武器会被盾牌挤掉）。
    render_parts.emplace(slot ? std::u16string(slot) : it->second.islot,
                         &it->second);
    return true;
  };

  add_part(g_character.head);
  add_part(g_character.body);
  add_part(g_character.hair);

  // 长袍会同时覆盖上衣和裤子；没有长袍才画上衣/裤子（点装优先）。
  bool longcoat = false;
  if (g_character.longcoat_deco.has_value()) {
    longcoat = add_part(g_character.longcoat_deco->id, u"Ma");
  }
  if (!longcoat && g_character.longcoat.has_value()) {
    longcoat = add_part(g_character.longcoat->id, u"Ma");
  }
  if (!longcoat) {
    bool coat = false;
    if (g_character.coat_deco.has_value()) {
      coat = add_part(g_character.coat_deco->id);
    }
    if (!coat && g_character.coat.has_value()) {
      coat = add_part(g_character.coat->id);
    }
    if (!coat) {
      add_part(u"01040000");
    }
    bool pant = false;
    if (g_character.pant_deco.has_value()) {
      pant = add_part(g_character.pant_deco->id);
    }
    if (!pant && g_character.pant.has_value()) {
      pant = add_part(g_character.pant->id);
    }
    if (!pant) {
      add_part(u"01060000");
    }
  }

  // 帽子 / 披风 / 手套 / 鞋 / 盾牌 / 饰品：点装优先，其次普通装备。
  auto add_slot = [&add_part](const std::optional<GameDecoItem>& deco,
                              const std::optional<GameEquipItem>& equip) {
    if (deco.has_value() && add_part(deco->id)) {
      return;
    }
    if (equip.has_value()) {
      add_part(equip->id);
    }
  };

  add_slot(g_character.cap_deco, g_character.cap);
  add_slot(g_character.cape_deco, g_character.cape);
  add_slot(g_character.glove_deco, g_character.glove);
  add_slot(g_character.shoes_deco, g_character.shoes);
  add_slot(g_character.shield_deco, g_character.shield);
  add_slot(g_character.ring0_deco, g_character.ring0);
  add_slot(g_character.ring1_deco, g_character.ring1);
  if (g_character.ring2.has_value()) {
    add_part(g_character.ring2->id);
  }
  if (g_character.ring3.has_value()) {
    add_part(g_character.ring3->id);
  }

  // 武器点装要按当前武器的动作子目录取（<点装id>/<武器子类>）。
  bool weapon_deco = false;
  if (g_character.weapon_deco.has_value() && g_character.weapon.has_value()) {
    std::u16string sub = g_character.weapon->id.substr(2, 2);
    weapon_deco = add_part(g_character.weapon_deco->id + u"/" + sub, u"Wp");
  }
  if (!weapon_deco && g_character.weapon.has_value()) {
    add_part(g_character.weapon->id, u"Wp");
  }
  std::u16string action = g_character.action;
  auto action_index = g_character.action_index;
  if (CharacterGameInstance::extern_action_.contains(action)) {
    action = CharacterGameInstance::extern_action_[action][action_index].action;
    action_index =
        CharacterGameInstance::extern_action_[g_character.action][action_index]
            .frame;
  }

  auto face = CharacterGameInstance::face_data_.at(g_character.face.id)
                  .data.at(g_character.face.action);
  if (CharacterGameInstance::bone_data_.at(action)[action_index].face) {
    face.data.at(action)[action_index] = {
        face.data.at(action)[action_index][g_character.face.index],
    };
    render_parts.emplace(face.islot, &face);
  }

  // 饰品（Af 脸饰 / Ay 眼饰 / Ae 耳饰 …）的数据由 LoadAccessoryFace 解析，
  // 这里按当前表情取出来放进 render_parts，和脸一样交给 zmap 顺序渲染。
  std::optional<CharacterAvatarRender> accessory_part;
  const std::u16string acc_id =
      g_character.accessory_deco.has_value()
          ? g_character.accessory_deco->id
          : (g_character.accessory.has_value() ? g_character.accessory->id
                                               : std::u16string{});
  if (!acc_id.empty() &&
      CharacterGameInstance::bone_data_.at(action)[action_index].face) {
    CharacterGameInstance::LoadAccessoryFace(acc_id);
    auto fit = CharacterGameInstance::face_data_.find(acc_id);
    if (fit != CharacterGameInstance::face_data_.end()) {
      auto& emotions = fit->second.data;
      // 有些脸饰没有 blink / 其它表情帧，用 default 顶上，
      // 否则人物眨眼的那几帧脸饰会突然消失。
      std::u16string acc_emotion = g_character.face.action;
      if (!emotions.contains(acc_emotion)) {
        acc_emotion = u"default";
      }
      if (emotions.contains(acc_emotion)) {
        const auto& acc = emotions.at(acc_emotion);
        if (acc.data.contains(action) &&
            action_index < acc.data.at(action).size()) {
          accessory_part = acc;
          render_parts.emplace(accessory_part->islot, &*accessory_part);
        }
      }
    }
  }

  std::flat_set<std::u16string> smaps_cover;
  for (const auto& index : CharacterGameInstance::zmap2_) {
    if (render_parts.contains(index)) {
      auto r = render_parts.at(index);
      if (!r->data.contains(action)) {
        continue;
      }
      const auto& pts = r->data.at(action)[action_index];
      std::flat_set<std::u16string> smaps_inter;
      for (const auto& pt : pts) {
        // 获取默认的smap
        // 有些点装的 z 不在遮挡表里（例如 pantsOverShoesBelowMailChest），
        // 查不到就把这件直接排进渲染队列，不能 at() 抛异常。
        auto smap_it = CharacterGameInstance::smap_.find(pt.z);
        if (smap_it == CharacterGameInstance::smap_.end()) {
          if (!g_character.hide_part.contains(pt.name)) {
            renders.emplace(pt.z, &pt);
          }
          continue;
        }
        const auto& smaps = smap_it->second;
        // 求交集
        std::flat_set<std::u16string> smaps_inter2;
        std::ranges::set_intersection(
            smaps, r->vslot, std::inserter(smaps_inter2, smaps_inter2.begin()));
        //  判断是否被占据
        bool part_cover = false;
        for (const auto& cover : smaps_cover) {
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

  for (const auto& index : CharacterGameInstance::zmap_) {
    if (renders.contains(index)) {
      auto [first, last] = renders.equal_range(index);
      for (auto it = first; it != last; ++it) {
        const CharacterAvatar* avatar = it->second;
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
          float center_x = g_character.pos.x;
          // 计算翻转后的位置
          pos_rect.x = center_x - (pos_rect.x - center_x) - pos_rect.w;
        }

        auto& camera = CameraGameInstance::camera_;
        // 相机不缩放（因为 pos_rect 的位置没缩放）
        if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
          pos_rect.x -= camera.x;
          pos_rect.y -= camera.y;
          SDL_SetTextureColorMod(texture, g_character.color.r,
                                 g_character.color.g, g_character.color.b);
          SDL_SetTextureAlphaMod(texture, g_character.color.a);
          SDL_RenderTextureRotated(Window::renderer_, texture, nullptr,
                                   &pos_rect, 0, nullptr,
                                   (SDL_FlipMode)g_character.flip);
        }
      }
    }
  }
  return true;
}

bool CharacterRenderSystem::RenderAfterimage(GameCharacter& g_character) {
  // 如果是法杖，而且是技能状态，不显示
  auto WeaponType = EquipGameInstance::LoadWeaponType(g_character);
  if (WeaponType == EquipGameInstance::WeaponType::kWand &&
      g_character.skill.has_value()) {
    return false;
  }
  // 通常情况下,attack和skill都会有afterimag的渲染
  auto r = AfterimageGameInstance::LoadData(g_character);
  if (r.has_value()) {
    auto texture = r.value().texture;
    auto alpha = r.value().alpha;
    auto origin = r.value().origin;
    auto& camera = CameraGameInstance::camera_;
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
      SDL_RenderTextureRotated(Window::renderer_, texture, nullptr, &pos_rect,
                               0, nullptr, (SDL_FlipMode)g_character.flip);
    }
  }
  return true;
}

void CharacterRenderSystem::RenderNametag(GameCharacter& g_character) {
  const auto& camera = CameraGameInstance::camera_;
  Freetype::LoadAligned(true);
  Freetype::LoadSize(13);
  auto w = Freetype::LoadW(g_character.name);
  auto h = Freetype::LoadLh();

  auto x = g_character.pos.x;
  auto y = g_character.pos.y;

  std::u16string nametag = u"";
  for (auto r : {g_character.ring0_deco, g_character.ring1_deco}) {
    if (r.has_value()) {
      auto info = EquipGameInstance::LoadEquipInfo(r->id);
      if (info->get_child(u"nameTag")) {
        auto tmp =
            static_cast<wz::Property<int>*>(info->get_child(u"nameTag"))->get();
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
      SDL_SetRenderDrawBlendMode(Window::renderer_, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, 178);
      SDL_RenderFillRect(Window::renderer_, &rect);
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawLine(g_character.name, x - camera.x - w / 2, y - camera.y);
    }
  } else {
    static auto nametag_node = WzResource::ui_->find(u"NameTag.img");
    auto texture_node = nametag_node->get_child(nametag);
    auto texture_w = WzResource::LoadTexture(texture_node->get_child("w"));
    auto texture_c = WzResource::LoadTexture(texture_node->get_child("c"));
    auto texture_e = WzResource::LoadTexture(texture_node->get_child("e"));

    auto ow = WzResource::LoadFpoint(texture_node->find(u"w/origin"));
    auto oc = WzResource::LoadFpoint(texture_node->find(u"c/origin"));

    auto a = (float)texture_c->w;
    auto b = w;
    auto result = a * ((b + a - 1) / a);  // 整数向上取整技巧

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
      SDL_RenderTexture(Window::renderer_, texture_w, nullptr, &rect);
      rect.x += (int)texture_w->w;
      rect.y = (int)(y - camera.y);
      rect.w = result;
      rect.h = texture_c->h;
      SDL_RenderTextureTiled(Window::renderer_, texture_c, nullptr, 1, &rect);
      rect.x += result;
      rect.w = texture_e->w;
      SDL_RenderTexture(Window::renderer_, texture_e, nullptr, &rect);
      auto clr =
          static_cast<wz::Property<int32_t>*>(texture_node->get_child(u"clr"))
              ->get();
      uint8_t a, r, g, b;
      a = (clr >> 24) & 0xFF;
      r = (clr >> 16) & 0xFF;
      g = (clr >> 8) & 0xFF;
      b = clr & 0xFF;
      Freetype::LoadColor(r, g, b, a);
      Freetype::DrawLine(g_character.name, rect.x - result, y - camera.y);
    }
  }
  return;
}

void CharacterRenderSystem::RenderTomb(GameCharacter& g_character) {
  if (!g_character.tomb.has_value()) {
    return;
  }
  auto tomb = g_character.tomb.value();
  static auto tomb_node = WzResource::effect_->find(u"Tomb.img");
  auto type = tomb.ani_type;
  auto ani_node = tomb_node->get_child(type);
  auto ani_index = std::to_string(tomb.ani_index);
  auto texture_node = ani_node->get_child(ani_index);
  if (texture_node->type == wz::Type::UOL) {
    texture_node =
        static_cast<wz::Property<wz::WzUOL>*>(texture_node)->get_uol();
  }
  auto texture = WzResource::LoadTexture(texture_node);
  auto origin = WzResource::LoadFpoint(texture_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = tomb.pos.x - origin.x,
      .y = tomb.pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  auto& camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
}

bool CharacterRenderSystem::RenderEffectFront(GameCharacter& g_character) {
  return EffectRenderSystem::RenderCharacterEffects(&g_character, true);
}

bool CharacterRenderSystem::RenderEffectBack(GameCharacter& g_character) {
  return EffectRenderSystem::RenderCharacterEffects(&g_character, false);
}

void CharacterRenderSystem::RenderChatballoon(GameCharacter& g_character) {
  if (!g_character.chatballoon.has_value()) {
    return;
  }
  auto chatballoon = g_character.chatballoon.value();
  auto name = g_character.name + u":";
  chatballoon.text = name + chatballoon.text;
  Freetype::LoadSize(chatballoon.size);
  SDL_FPoint pos{
      .x = g_character.pos.x,
      .y = g_character.pos.y - 70,
  };
  ChatballoonRenderSystem::Render(chatballoon, pos);
}

void CharacterRenderSystem::RenderChair(GameCharacter& g_character) {
  auto chair_id = g_character.chair->id;
  auto chair_node =
      WzResource::item_->find(u"Install/0301.img/" + chair_id + u"/effect");
  chair_node =
      chair_node->get_child(std::to_string(g_character.chair->ani_index));
  auto t = WzResource::LoadTexture(chair_node);
  auto origin = WzResource::LoadFpoint(chair_node->get_child(u"origin"));
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
  auto& camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTextureRotated(Window::renderer_, t, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)g_character.flip);
  }
}

void CharacterRenderSystem::RenderChair(GameCharacter& g_character,
                                        bool front) {
  if (!g_character.chair.has_value()) {
    return;
  }

  auto chair_id = g_character.chair->id;
  auto z_node =
      WzResource::item_->find(u"Install/0301.img/" + chair_id + u"/effect/z");
  auto z = static_cast<wz::Property<int>*>(z_node)->get();

  if ((front && z < 0) || (!front && z > 0)) {
    RenderChair(g_character);
  }
}

void CharacterRenderSystem::RenderMorph(GameCharacter& g_character) {
  auto node = WzResource::morph_->find(g_character.morph + u".img");
  auto action = CharacterLogicSystem::LoadMorphType(g_character);
  node = node->get_child(action);
  node = node->get_child(std::to_string(g_character.action_index));
  auto t = WzResource::LoadTexture(node);
  auto origin = WzResource::LoadFpoint(node->get_child(u"origin"));
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
  auto& camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_SetTextureColorMod(t, g_character.color.r, g_character.color.g,
                           g_character.color.b);
    SDL_SetTextureAlphaMod(t, g_character.color.a);
    SDL_RenderTextureRotated(Window::renderer_, t, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)g_character.flip);
  }
}


namespace {
void RenderCraftGauge(GameCharacter& g_character) {
  auto* graduation_node =
      WzResource::ui_->find(u"Crafting.img/Guage/1/graduation");
  auto* gauge_node = WzResource::ui_->find(u"Crafting.img/Guage/1/gauge");
  auto* success_node =
      WzResource::ui_->find(u"Crafting.img/Guage/2/graduation");
  if (graduation_node == nullptr || gauge_node == nullptr) {
    return;
  }
  static auto graduation = WzResource::LoadTexture(graduation_node);
  static auto gauge = WzResource::LoadTexture(gauge_node);
  static auto success =
      success_node != nullptr ? WzResource::LoadTexture(success_node) : nullptr;
  if (graduation == nullptr || gauge == nullptr) {
    return;
  }
  SDL_Texture* gr = graduation;
  const auto& camera = CameraGameInstance::camera_;
  float x = g_character.pos.x - camera.x - 21;
  float y = g_character.pos.y - camera.y - 32 - 45;
  int fill_w =
      static_cast<int>(std::lround(36 * CraftUiSystem::CraftProgress()));
  SDL_FRect fill{x + 3, y + 3, static_cast<float>(fill_w),
                 static_cast<float>(gauge->h)};
  if (!CraftUiSystem::Crafting()) {
    if (success != nullptr) {
      gr = success;
      x = g_character.pos.x - camera.x - 30;
      y = g_character.pos.y - camera.y - 41 - 45;
      fill = {x + 3, y + 3, static_cast<float>(fill_w),
              static_cast<float>(gauge->h)};
      fill.x += 9;
      fill.y += 9;
      fill_w = 36;
      fill.w = 36;
    }
  }
  SDL_FRect bg{x, y, static_cast<float>(gr->w), static_cast<float>(gr->h)};
  SDL_RenderTexture(Window::renderer_, gr, nullptr, &bg);
  if (fill_w <= 0) {
    return;
  }
  SDL_RenderTexture(Window::renderer_, gauge, nullptr, &fill);
}
}  // namespace
bool CharacterRenderSystem::Render(GameCharacter& g_character) {
  if (g_character.morph.empty()) {
    RenderEffectFront(g_character);
    RenderChair(g_character, true);
    RenderAfterimage(g_character);
    RenderTomb(g_character);
    RenderCharacter(g_character);
    RenderChair(g_character, false);
    RenderEffectBack(g_character);
  } else {
    RenderMorph(g_character);
    RenderEffectBack(g_character);
  }
  RenderNametag(g_character);
  if (&g_character == &CharacterGameInstance::self_ &&
      (CraftUiSystem::Crafting() || CraftUiSystem::CraftSuccess())) {
    RenderCraftGauge(g_character);
  }
  // 气泡由 SceneSystemInstance 在场景全部绘制完后再统一画一遍，
  // 保证它盖在前景 / 其他角色之上，这里不能再画第二次。
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

}  // namespace sdlms
