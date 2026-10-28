#include "cursor_render_system.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/keybinding_ui_system.h"
#include "src/client/system/ui/quick_slot_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <string>

void CursorRenderSystem::render_hand() {
  if (CursorGameInstance::cursor_hand_net.has_value()) {
    return;
  }
  if (CursorGameInstance::modal_overlay) {
    return;
  }
  if (CursorGameInstance::cursor_hand.has_value()) {
    auto hand = CursorGameInstance::cursor_hand.value();
    const auto &self = CharacterGameInstance::self;
    // 统一的渲染函数
    auto render_icon = [&](SDL_Texture *icon, uint8_t alpha = 128) {
      SDL_FRect pos_rect{
          Window::mouse_pos.x - (float)icon->w / 2,
          Window::mouse_pos.y - (float)icon->h / 2,
          static_cast<float>(icon->w),
          static_cast<float>(icon->h),
      };
      pos_rect.x = (int)pos_rect.x;
      pos_rect.y = (int)pos_rect.y;
      SDL_SetTextureAlphaMod(icon, alpha);
      SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);
      SDL_SetTextureAlphaMod(icon, 255);
    };
    switch (hand.type) {
    case CursorGameInstance::CursorHandType::kEquipment: {
      auto index = hand.sub_val;
      auto eqp = EquipGameInstance::load_equip(index);
      auto info = EquipGameInstance::load_equip_info(eqp->value().id);
      auto t = WzResource::load_texture(info->get_child(u"iconRaw"));
      render_icon(t);
      break;
    }
    case CursorGameInstance::CursorHandType::kDeco: {
      auto index = hand.sub_val;
      auto deco = EquipGameInstance::load_deco(index);
      auto info = EquipGameInstance::load_equip_info(deco->value().id);
      auto t = WzResource::load_texture(info->get_child(u"iconRaw"));
      render_icon(t);
      break;
    }
    case CursorGameInstance::CursorHandType::kPackage: {
      SDL_Texture *icon = nullptr;
      if (hand.val == (int)ItemEnum::kEquip ||
          hand.val == (int)ItemEnum::kDeco) {
        auto &equip = PackageGameInstance::data[hand.val][hand.sub_val];
        auto info = EquipGameInstance::load_equip_info(equip->id);
        icon = WzResource::load_texture(info->get_child(u"iconRaw"));
        render_icon(icon);
      } else {
        auto &r = PackageGameInstance::data[hand.val];
        const auto &item = r.at(hand.sub_val);
        auto info = ItemGameInstance::load_item_info(item->id, 0);
        icon = WzResource::load_texture(info->get_child(u"iconRaw"));
        render_icon(icon);
      }
      break;
    }
    case CursorGameInstance::CursorHandType::kSkill: {
      auto tmp = std::format("{:07d}", hand.sub_val);
      std::u16string ski_id{tmp.begin(), tmp.end()};
      auto ski_node = SkillGameInstance::load_ski_node(ski_id);
      auto icon = WzResource::load_texture(ski_node->get_child(u"icon"));
      render_icon(icon, 172); // 技能透明度不同
      break;
    }
    case CursorGameInstance::CursorHandType::kKeybind: {
      if (KeyboardGameInstance::data.contains((SDL_Scancode)hand.val)) {
        auto &input = KeyboardGameInstance::data.at((SDL_Scancode)hand.val);
        if (input.type == "skill") {
          std::u16string ski_id{input.val.begin(), input.val.end()};
          auto ski_node = SkillGameInstance::load_ski_node(ski_id);
          auto icon = WzResource::load_texture(ski_node->get_child(u"icon"));
          render_icon(icon, 172); // 技能透明度不同
        } else if (input.type == "item") {
          std::u16string item_id{input.val.begin(), input.val.end()};
          auto info = ItemGameInstance::load_item_info(item_id, 0);
          auto icon = WzResource::load_texture(info->get_child(u"icon"));
          render_icon(icon, 172); // 技能透明度不同
        } else {
          auto x = Window::mouse_pos.x - 16;
          auto y = Window::mouse_pos.y - 16;
          QuickSlotUiSystem::render_input_icon(input, x, y, 172);
        }
      } else {
        auto x = Window::mouse_pos.x - 16;
        auto y = Window::mouse_pos.y - 16;
        KeybindingUiSystem::render_remain_icon(hand.sub_val, x, y);
      }
      break;
    }
    default: {
      break;
    }
    }
  }
}

void CursorRenderSystem::render_cursor() {
  static auto cursor_node = WzResource::ui->find(u"Cursor.img");
  auto cursor_index_node =
      cursor_node->get_child(CursorGameInstance::cursor_type);
  auto texture_index = std::to_string(CursorGameInstance::cursor_index);
  auto texture_node = cursor_index_node->get_child(texture_index);
  if (texture_node->type == wz::Type::UOL) {
    texture_node =
        static_cast<wz::Property<wz::WzUOL> *>(texture_node)->get_uol();
  }
  auto texture = WzResource::load_texture(texture_node);
  auto origin = WzResource::load_fpoint(texture_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = Window::mouse_pos.x - origin.x,
      .y = Window::mouse_pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
}

bool CursorRenderSystem::render() {
  render_hand();
  render_cursor();
  return true;
}