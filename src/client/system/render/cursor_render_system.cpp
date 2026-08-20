#include "cursor_render_system.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <string>

void cursor_render_system::render_hand() {
  if (cursor_game_instance::cursor_hand_net.has_value()) {
    return;
  }
  if (cursor_game_instance::modal_overlay) {
    return;
  }
  if (cursor_game_instance::cursor_hand.has_value()) {
    auto hand = cursor_game_instance::cursor_hand.value();
    const auto &self = character_game_instance::self;
    // 统一的渲染函数
    auto render_icon = [&](SDL_Texture *icon, uint8_t alpha = 128) {
      SDL_FRect pos_rect{
          window::mouse_pos.x - (float)icon->w / 2,
          window::mouse_pos.y - (float)icon->h / 2,
          static_cast<float>(icon->w),
          static_cast<float>(icon->h),
      };
      SDL_SetTextureAlphaMod(icon, alpha);
      SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
      SDL_SetTextureAlphaMod(icon, 255);
    };
    switch (hand.type) {
    case cursor_game_instance::equipment: {
      auto index = hand.sub_val;
      auto eqp = equip_ui_system::load_equip(
          (equip_ui_system::equip_mouse_index)index);
      auto info = equip_game_instance::load_equip_info(eqp->value().id);
      auto t = wz_resource::load_texture(info->get_child(u"iconRaw"));
      render_icon(t);
      break;
    }
    case cursor_game_instance::deco: {
      auto index = hand.sub_val;
      auto deco =
          equip_ui_system::load_deco((equip_ui_system::equip_mouse_index)index);
      auto info = equip_game_instance::load_equip_info(deco->value().id);
      auto t = wz_resource::load_texture(info->get_child(u"iconRaw"));
      render_icon(t);
      break;
    }
    case cursor_game_instance::package: {
      SDL_Texture *icon = nullptr;
      if (hand.val == (int)item_enum::equip ||
          hand.val == (int)item_enum::deco) {
        auto &equip = package_game_instance::data[hand.val][hand.sub_val];
        auto info = equip_game_instance::load_equip_info(equip->id);
        icon = wz_resource::load_texture(info->get_child(u"iconRaw"));
        render_icon(icon);
      } else {
        auto &r = package_game_instance::data[hand.val];
        const auto &item = r.at(hand.sub_val);
        auto info = item_game_instance::load_item_info(item->id, 0);
        icon = wz_resource::load_texture(info->get_child(u"iconRaw"));
        render_icon(icon);
      }
      break;
    }
    case cursor_game_instance::skill: {
      auto tmp = std::format("{:07d}", hand.sub_val);
      std::u16string ski_id{tmp.begin(), tmp.end()};
      auto ski_node = skill_game_instance::load_ski_node(ski_id);
      auto icon = wz_resource::load_texture(ski_node->get_child(u"icon"));
      render_icon(icon, 172); // 技能透明度不同
      break;
    }
    case cursor_game_instance::keybind: {
      // keybind类型暂无处理逻辑
      break;
    }
    default: {
      break;
    }
    }
  }
}

void cursor_render_system::render_cursor() {
  static auto cursor_node = wz_resource::ui->find(u"Cursor.img");
  auto cursor_index_node =
      cursor_node->get_child(cursor_game_instance::cursor_type);
  auto texture_index = std::to_string(cursor_game_instance::cursor_index);
  auto texture_node = cursor_index_node->get_child(texture_index);
  if (texture_node->type == wz::Type::UOL) {
    texture_node =
        static_cast<wz::Property<wz::WzUOL> *>(texture_node)->get_uol();
  }
  auto texture = wz_resource::load_texture(texture_node);
  auto origin = wz_resource::load_fpoint(texture_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = window::mouse_pos.x - origin.x,
      .y = window::mouse_pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

bool cursor_render_system::render() {
  render_hand();
  render_cursor();
  return true;
}