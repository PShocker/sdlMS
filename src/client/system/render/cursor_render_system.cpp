#include "cursor_render_system.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <string>

bool cursor_render_system::render() {
  static auto cursor_node = wz_resource::ui->find(u"Cursor.img");
  auto cursor_index_node =
      cursor_node->get_child(cursor_game_instance::cursor_type);
  auto texture_index = std::to_string(cursor_game_instance::cursor_index);
  auto texture_node = cursor_index_node->get_child(texture_index);
  auto texture = wz_resource::load_texture(texture_node);
  SDL_FRect pos_rect = {
      .x = window::mouse_pos.x,
      .y = window::mouse_pos.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  return true;
}