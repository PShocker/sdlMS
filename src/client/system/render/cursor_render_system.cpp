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

    // 获取装备图标
    auto get_equip_icon = [&](int index) -> SDL_Texture * {
      const game_equip_item *eqp = nullptr;
      switch ((equip_ui_system::equip_mouse_index)(index)) {
      case equip_ui_system::cap:
        eqp = self.cap.has_value() ? &self.cap.value() : nullptr;
        break;
      case equip_ui_system::earcc:
        eqp = self.accessory.has_value() ? &self.accessory.value() : nullptr;
        break;
      case equip_ui_system::clothes:
        eqp = self.coat.has_value() ? &self.coat.value() : nullptr;
        break;
      case equip_ui_system::pants:
        eqp = self.pant.has_value() ? &self.pant.value() : nullptr;
        break;
      case equip_ui_system::shoes:
        eqp = self.shoes.has_value() ? &self.shoes.value() : nullptr;
        break;
      case equip_ui_system::gloves:
        eqp = self.glove.has_value() ? &self.glove.value() : nullptr;
        break;
      case equip_ui_system::cape:
        eqp = self.cape.has_value() ? &self.cape.value() : nullptr;
        break;
      case equip_ui_system::shield:
        eqp = self.shield.has_value() ? &self.shield.value() : nullptr;
        break;
      case equip_ui_system::weapon:
        eqp = self.weapon.has_value() ? &self.weapon.value() : nullptr;
        break;
      case equip_ui_system::ring0:
        eqp = self.ring0.has_value() ? &self.ring0.value() : nullptr;
        break;
      case equip_ui_system::ring1:
        eqp = self.ring1.has_value() ? &self.ring1.value() : nullptr;
        break;
      case equip_ui_system::ring2:
        eqp = self.ring2.has_value() ? &self.ring2.value() : nullptr;
        break;
      case equip_ui_system::ring3:
        eqp = self.ring3.has_value() ? &self.ring3.value() : nullptr;
        break;
      default:
        return nullptr;
      }
      auto info = equip_game_instance::load_equip_info(eqp->id);
      return wz_resource::load_texture(info->get_child(u"iconRaw"));
    };

    switch (hand.type) {
    case cursor_game_instance::equipment: {
      render_icon(get_equip_icon(hand.sub_val));
      break;
    }
    case cursor_game_instance::package: {
      SDL_Texture *icon = nullptr;
      if (hand.val == 0) {
        auto &equip = package_game_instance::data[hand.val][hand.sub_val];
        auto info = equip_game_instance::load_equip_info(equip->id);
        icon = wz_resource::load_texture(info->get_child(u"iconRaw"));
        render_icon(icon);
      } else {
        auto &r = package_game_instance::data[hand.val];
        const auto &item = r.at(hand.sub_val);
        auto info = item_game_instance::load_item_info(item->id);
        icon = wz_resource::load_texture(info->get_child(u"iconRaw"));
        render_icon(icon);
      }
      break;
    }
    case cursor_game_instance::skill: {
      auto tmp = std::format("{:07d}", hand.sub_val);
      std::u16string ski_id{tmp.begin(), tmp.end()};
      auto ski_node = skill_game_instance::load_skill_node(ski_id);
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