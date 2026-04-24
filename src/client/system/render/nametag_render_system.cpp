#include "nametag_render_system.h"
#include "src/client/game/game_nametag.h"
#include "src/common/wz/wz_resource.h"

void nametag_render_system::render_backgrnd(game_nametag &g_nametag,
                                            SDL_FPoint base) {
  auto path = g_nametag.path;
  if (!path.empty()) {
    auto nametag_node = wz_resource::ui->find(u"NameTag.img/" + path);
    auto w = wz_resource::load_texture(nametag_node->get_child(u"w"));
    auto c = wz_resource::load_texture(nametag_node->get_child(u"c"));
    auto e = wz_resource::load_texture(nametag_node->get_child(u"e"));
  }
}

bool nametag_render_system::render(game_nametag &g_nametag, SDL_FPoint base) {
  auto text = g_nametag.text;

  return true;
}