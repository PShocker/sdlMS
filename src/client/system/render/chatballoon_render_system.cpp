#include "chatballoon_render_system.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

bool chatballoon_render_system::render(game_chatballoon &g_chatballoon,
                                       SDL_FPoint base) {
  auto path = g_chatballoon.path;
  auto node = wz_resource::ui->find(path);
  auto texture_c = wz_resource::load_texture(node->get_child(u"c"));
  auto texture_e = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"e")));
  auto texture_ne = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"ne")));
  auto texture_n = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"n")));
  auto texture_nw = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"nw")));
  auto texture_w = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"w")));
  auto texture_sw = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"sw")));
  auto texture_s = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"s")));
  auto texture_se = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"se")));
  auto texture_arrow = wz_resource::load_texture(
      static_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"arrow")));

  const uint32_t width = 100;

  return true;
}