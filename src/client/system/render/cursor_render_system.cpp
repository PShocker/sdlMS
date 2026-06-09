#include "cursor_render_system.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <string>

void cursor_render_system::render_item() {
  if (cursor_game_instance::cursor_hand_drop_id.has_value()) {
    return;
  }
  if (cursor_game_instance::cursor_hand.has_value()) {
    auto hand = cursor_game_instance::cursor_hand.value();
    switch (hand.type) {
    case cursor_game_instance::equipment: {
      // hand.index
      break;
    }
    case cursor_game_instance::package: {
      if (hand.val == 0) {
        auto equip = package_game_instance::equips[hand.val2];
        auto info = equip_game_instance::load_equip_info(equip.value().id);
        auto icon = wz_resource::load_texture(info->get_child(u"iconRaw"));
        SDL_FRect pos_rect{
            window::mouse_pos.x - icon->w / 2,
            window::mouse_pos.y - icon->h / 2,
            static_cast<float>(icon->w),
            static_cast<float>(icon->h),
        };
        SDL_SetTextureAlphaMod(icon, 128);
        SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
        SDL_SetTextureAlphaMod(icon, 255);
      } else {
        std::vector<std::optional<game_item>> *r;
        switch (hand.val) {
        case 1: {
          r = &package_game_instance::cosumes;
          break;
        }
        case 2: {
          r = &package_game_instance::etc;
          break;
        }
        case 3: {
          r = &package_game_instance::install;
          break;
        }
        case 4: {
          r = &package_game_instance::cash;
          break;
        }
        default: {
          break;
        }
        }
        auto item = r->at(hand.val2).value();
        auto info = item_game_instance::load_item_info(item.id);
        auto icon = wz_resource::load_texture(info->get_child(u"iconRaw"));
        SDL_FRect pos_rect{
            window::mouse_pos.x - icon->w / 2,
            window::mouse_pos.y - icon->h / 2,
            static_cast<float>(icon->w),
            static_cast<float>(icon->h),
        };
        SDL_SetTextureAlphaMod(icon, 128);
        SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
        SDL_SetTextureAlphaMod(icon, 255);
      }
      break;
    }
    case cursor_game_instance::skill: {
      break;
    }
    case cursor_game_instance::keybind: {
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
  SDL_FRect pos_rect = {
      .x = window::mouse_pos.x,
      .y = window::mouse_pos.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

bool cursor_render_system::render() {
  render_item();
  render_cursor();
  return true;
}