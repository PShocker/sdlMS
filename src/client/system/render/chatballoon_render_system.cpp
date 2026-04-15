#include "chatballoon_render_system.h"
#include "src/common/wz/wz_resource.h"

bool chatballoon_render_system::render(game_chatballoon &g_chatballoon,
                                       SDL_FPoint base) {
  auto path = g_chatballoon.path;
  auto node = wz_resource::ui->find(path);
  auto texture_c = wz_resource::load_texture(node->get_child(u"c"));
  return true;
}