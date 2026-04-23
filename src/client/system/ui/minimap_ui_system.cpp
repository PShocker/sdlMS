#include "minimap_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <array>

void minimap_ui_system::render_mini() {}

void minimap_ui_system::render_backgrnd() {
  const auto width = 100;
  const auto height = 100;

  auto node = wz_resource::ui->find(u"MiniMap.img/MinMap");
  auto c = wz_resource::load_texture(node->get_child(u"c"));
  auto e = wz_resource::load_texture(node->get_child(u"e"));
  auto ne = wz_resource::load_texture(node->get_child(u"ne"));
  auto n = wz_resource::load_texture(node->get_child(u"n"));
  auto nw = wz_resource::load_texture(node->get_child(u"nw"));
  auto w = wz_resource::load_texture(node->get_child(u"w"));
  auto sw = wz_resource::load_texture(node->get_child(u"sw"));
  auto s = wz_resource::load_texture(node->get_child(u"s"));
  auto se = wz_resource::load_texture(node->get_child(u"se"));

  // 先以c填充整个texture,然后在填充四周的边框,最后是四个边角
  SDL_FRect pos_rect = {pos.x, pos.y, (float)width, (float)height};
  SDL_RenderTexture(window::renderer, c, nullptr, &pos_rect);

  // 竖着的两边
  pos_rect = {pos.x, pos.y, (float)w->w, (float)height};
  SDL_RenderTexture(window::renderer, w, nullptr, &pos_rect);
  pos_rect = {pos.x + (float)width - e->w, pos.y, (float)e->w, (float)height};
  SDL_RenderTexture(window::renderer, e, nullptr, &pos_rect);

  // 横着的两边
  pos_rect = {pos.x, pos.y, (float)width, (float)n->h};
  SDL_RenderTexture(window::renderer, n, nullptr, &pos_rect);
  pos_rect = {pos.x, pos.y + (float)height - s->h, (float)width, (float)s->h};
  SDL_RenderTexture(window::renderer, s, nullptr, &pos_rect);

  // 左上
  pos_rect = {pos.x, pos.y, (float)nw->w, (float)nw->h};
  SDL_RenderTexture(window::renderer, nw, nullptr, &pos_rect);
  // 左下
  pos_rect = {pos.x, pos.y + (float)height - sw->h, (float)sw->w, (float)sw->h};
  SDL_RenderTexture(window::renderer, sw, nullptr, &pos_rect);
  // 右上
  pos_rect = {pos.x + (float)width - ne->w, pos.y, (float)ne->w, (float)ne->h};
  SDL_RenderTexture(window::renderer, ne, nullptr, &pos_rect);
  // 右下
  pos_rect = {pos.x + (float)width - se->w, pos.y + (float)height - se->h,
              (float)se->w, (float)se->h};
  SDL_RenderTexture(window::renderer, se, nullptr, &pos_rect);

  // 标题
  node = wz_resource::ui->find(u"MiniMap.img/title");
  auto title = wz_resource::load_texture(node);
  pos_rect = {pos.x + 6, pos.y + 5, (float)title->w, (float)title->h};
  SDL_RenderTexture(window::renderer, title, nullptr, &pos_rect);
}

void minimap_ui_system::render_button() {
  const static std::array buttons_node = {
      wz_resource::ui->find(u"MiniMap.img/BtMap"),
      wz_resource::ui->find(u"MiniMap.img/BtMin"),
      wz_resource::ui->find(u"MiniMap.img/BtMax"),
  };
  const static std::array buttons_pos = {
      SDL_FPoint{0, 0},
      SDL_FPoint{50, 0},
      SDL_FPoint{80, 0},
  };

  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto v = buttons_pos[i];
    auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
    auto normal = wz_resource::load_texture(k->find(u"normal/0"));
    auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
    SDL_FRect pos_rect{pos.x + v.x, pos.y + v.y,
                       static_cast<float>(mouse_over->w),
                       static_cast<float>(mouse_over->h)};
    auto &mouse_pos = window::mouse_pos;
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
};

void minimap_ui_system::render_canvas_life() {
  auto map_node = wz_resource::load_map_node(scene_system_instance::map_id);
  auto minimap_node = map_node->get_child(u"miniMap");
  auto width =
      static_cast<wz::Property<int> *>(minimap_node->get_child(u"width"))
          ->get();
  auto height =
      static_cast<wz::Property<int> *>(minimap_node->get_child(u"height"))
          ->get();
  auto centerX =
      static_cast<wz::Property<int> *>(minimap_node->get_child(u"centerX"))
          ->get();
  auto centerY =
      static_cast<wz::Property<int> *>(minimap_node->get_child(u"centerY"))
          ->get();
  auto mag =
      static_cast<wz::Property<int> *>(minimap_node->get_child(u"mag"))->get();
  auto scale = std::pow(2, mag);

  const auto backgrnd_w = 100;
  const auto backgrnd_h = 100;
  auto canvas_texture =
      wz_resource::load_texture(minimap_node->get_child(u"canvas"));
  static auto npc_texture = wz_resource::load_texture(
      wz_resource::map->find(u"MapHelper.img/minimap/npc"));
  static auto por_texture = wz_resource::load_texture(
      wz_resource::map->find(u"MapHelper.img/minimap/portal"));
  static auto user_texture = wz_resource::load_texture(
      wz_resource::map->find(u"MapHelper.img/minimap/user"));
  // render npc
  for (auto &npcs : npc_game_instance::data) {
    for (auto &npc : npcs) {
      SDL_FPoint n_pos{static_cast<float>(npc.pos.x / scale),
                       static_cast<float>(npc.pos.y / scale)};
      n_pos.x = pos.x + (backgrnd_w - canvas_texture->w) / 2 + n_pos.x - 2;
      n_pos.y = pos.y + 72 + n_pos.y - 4;
      SDL_FRect pos_rect = {n_pos.x, n_pos.y, (float)npc_texture->w,
                            (float)npc_texture->h};
      SDL_RenderTexture(window::renderer, npc_texture, nullptr, &pos_rect);
    }
  }
  // render portal
  for (auto &portal : portal_game_instance::data) {
    SDL_FPoint p_pos{static_cast<float>(portal.pos.x / scale),
                     static_cast<float>(portal.pos.y / scale)};
    p_pos.x = pos.x + (backgrnd_w - canvas_texture->w) / 2 + p_pos.x - 2;
    p_pos.y = pos.y + 72 + p_pos.y - 6;
    SDL_FRect pos_rect = {p_pos.x, p_pos.y, (float)npc_texture->w,
                          (float)npc_texture->h};
    SDL_RenderTexture(window::renderer, por_texture, nullptr, &pos_rect);
  }
  // render users
  // todo

  // render self
  auto &self = character_game_instance::self;
  SDL_FPoint u_pos{static_cast<float>(self.pos.x / scale),
                   static_cast<float>(self.pos.y / scale)};
  u_pos.x = pos.x + (backgrnd_w - canvas_texture->w) / 2 + u_pos.x - 2;
  u_pos.y = pos.y + 72 + u_pos.y - 4;
  SDL_FRect pos_rect = {u_pos.x, u_pos.y, (float)npc_texture->w,
                        (float)npc_texture->h};
  SDL_RenderTexture(window::renderer, por_texture, nullptr, &pos_rect);
}

void minimap_ui_system::render_mark() {
  auto info_node = map_info_game_instance::map_info;
  auto mark_str = static_cast<wz::Property<std::u16string> *>(
                      info_node->get_child(u"mapMark"))
                      ->get();
  auto mark_node = wz_resource::map->find(u"MapHelper.img/mark/" + mark_str);
  auto mark_texture = wz_resource::load_texture(mark_node);

  SDL_FRect pos_rect{pos.x + 6, pos.y + 24, static_cast<float>(mark_texture->w),
                     static_cast<float>(mark_texture->h)};
  SDL_RenderTexture(window::renderer, mark_texture, nullptr, &pos_rect);
}

void minimap_ui_system::render_canvas() {
  auto map_node = wz_resource::load_map_node(scene_system_instance::map_id);
  auto minimap_node = map_node->get_child(u"miniMap");
  auto canvas_texture =
      wz_resource::load_texture(minimap_node->get_child(u"canvas"));

  SDL_FRect pos_rect{pos.x + 6, pos.y + 24,
                     static_cast<float>(canvas_texture->w),
                     static_cast<float>(canvas_texture->h)};
  SDL_RenderTexture(window::renderer, canvas_texture, nullptr, &pos_rect);
}

bool minimap_ui_system::render() {
  if (!disable) {
    render_backgrnd();
    render_button();
    render_canvas_life();
  } else {
    render_mini();
  }
  return true;
}

bool minimap_ui_system::cursor_in() { return true; }

bool minimap_ui_system::event_button(SDL_Event *event) { return true; }

bool minimap_ui_system::event(SDL_Event *event) { return false; }
