#include "minimap_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/worldmap_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <flat_map>
#include <ranges>
#include <string>
#include <vector>

void minimap_ui_system::render_mini() {
  static auto w =
      wz_resource::load_texture(wz_resource::ui->find(u"MiniMap.img/Min/w"));
  static auto c =
      wz_resource::load_texture(wz_resource::ui->find(u"MiniMap.img/Min/c"));
  static auto e =
      wz_resource::load_texture(wz_resource::ui->find(u"MiniMap.img/Min/e"));
  auto map_id = scene_system_instance::map_id;
  auto map_name = load_map_name(map_id);
  auto s1 = map_name.street_name;
  auto s2 = map_name.map_name;
  auto s = s1 + u":" + s2;
  freetype::load_size(14);
  freetype::load_aligned(true);
  freetype::load_color(0, 0, 0, 255);
  auto l = load_wh().x;
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(w->w),
      static_cast<float>(w->h),
  };
  SDL_SetTextureAlphaMod(w, 162);
  SDL_RenderTexture(window::renderer, w, nullptr, &pos_rect);

  pos_rect.x += w->w;
  pos_rect.w = l - w->w - e->w;
  SDL_SetTextureAlphaMod(c, 162);
  SDL_RenderTexture(window::renderer, c, nullptr, &pos_rect);
  freetype::draw_line(s, pos_rect.x, pos_rect.y);

  pos_rect.x = pos.x + l - e->w;
  pos_rect.w = e->w;
  SDL_SetTextureAlphaMod(e, 162);
  SDL_RenderTexture(window::renderer, e, nullptr, &pos_rect);
}

static SDL_FPoint backgrnd_min_wh;

static SDL_FPoint backgrnd_max_wh;

SDL_Texture *minimap_ui_system::load_canvas_texture() {
  auto map_node = wz_resource::load_map_node(scene_system_instance::map_id);
  if (auto minimap_node = map_node->get_child(u"miniMap")) {
    return wz_resource::load_texture(minimap_node->get_child(u"canvas"));
  }
  return nullptr;
}

void minimap_ui_system::render_min_backgrnd() {
  static auto node = wz_resource::ui->find(u"MiniMap.img/MinMap");
  static auto c = wz_resource::load_texture(node->get_child(u"c"));
  static auto e = wz_resource::load_texture(node->get_child(u"e"));
  static auto ne = wz_resource::load_texture(node->get_child(u"ne"));
  static auto n = wz_resource::load_texture(node->get_child(u"n"));
  static auto nw = wz_resource::load_texture(node->get_child(u"nw"));
  static auto w = wz_resource::load_texture(node->get_child(u"w"));
  static auto sw = wz_resource::load_texture(node->get_child(u"sw"));
  static auto s = wz_resource::load_texture(node->get_child(u"s"));
  static auto se = wz_resource::load_texture(node->get_child(u"se"));

  SDL_FRect pos_rect;
  // 左上
  pos_rect = {pos.x, pos.y, (float)nw->w, (float)nw->h};
  SDL_RenderTexture(window::renderer, nw, nullptr, &pos_rect);
  // 左下
  pos_rect = {pos.x, pos.y + (float)backgrnd_max_wh.y - sw->h, (float)sw->w,
              (float)sw->h};
  SDL_RenderTexture(window::renderer, sw, nullptr, &pos_rect);
  // 右上
  pos_rect = {pos.x + (float)backgrnd_max_wh.x - ne->w, pos.y, (float)ne->w,
              (float)ne->h};
  SDL_RenderTexture(window::renderer, ne, nullptr, &pos_rect);
  // 右下
  pos_rect = {pos.x + (float)backgrnd_max_wh.x - se->w,
              pos.y + (float)backgrnd_max_wh.y - se->h, (float)se->w,
              (float)se->h};
  SDL_RenderTexture(window::renderer, se, nullptr, &pos_rect);

  // 竖着的两边
  pos_rect = {pos.x, pos.y + nw->h, (float)w->w,
              (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_RenderTexture(window::renderer, w, nullptr, &pos_rect);
  pos_rect = {pos.x + (float)backgrnd_max_wh.x - e->w, pos.y + nw->h,
              (float)e->w, (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_RenderTexture(window::renderer, e, nullptr, &pos_rect);

  // 横着的两边
  pos_rect = {pos.x + nw->w, pos.y, (float)backgrnd_max_wh.x - nw->w * 2,
              (float)n->h};
  SDL_RenderTexture(window::renderer, n, nullptr, &pos_rect);

  pos_rect = {pos.x + sw->w, pos.y + (float)backgrnd_max_wh.y - s->h,
              (float)backgrnd_max_wh.x - sw->w * 2, (float)s->h};
  SDL_RenderTexture(window::renderer, s, nullptr, &pos_rect);

  //
  pos_rect = {pos.x + nw->w, pos.y + nw->h,
              (float)backgrnd_max_wh.x - nw->w * 2,
              (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_SetRenderDrawColor(window::renderer, 0, 0, 0, 100);
  SDL_SetRenderDrawBlendMode(window::renderer, SDL_BLENDMODE_BLEND);
  SDL_RenderFillRect(window::renderer, &pos_rect);

  // 标题
  static auto title =
      wz_resource::load_texture(wz_resource::ui->find(u"MiniMap.img/title"));
  pos_rect = {pos.x + 6, pos.y + 10, (float)title->w, (float)title->h};
  SDL_RenderTexture(window::renderer, title, nullptr, &pos_rect);
}

void minimap_ui_system::render_max_backgrnd() {
  static auto node = wz_resource::ui->find(u"MiniMap.img/MaxMap");
  static auto e = wz_resource::load_texture(node->get_child(u"e"));
  static auto ne = wz_resource::load_texture(node->get_child(u"ne"));
  static auto n = wz_resource::load_texture(node->get_child(u"n"));
  static auto nw = wz_resource::load_texture(node->get_child(u"nw"));
  static auto w = wz_resource::load_texture(node->get_child(u"w"));
  static auto sw = wz_resource::load_texture(node->get_child(u"sw"));
  static auto s = wz_resource::load_texture(node->get_child(u"s"));
  static auto se = wz_resource::load_texture(node->get_child(u"se"));

  SDL_FRect pos_rect;
  // 左上
  pos_rect = {pos.x, pos.y, (float)nw->w, (float)nw->h};
  SDL_RenderTexture(window::renderer, nw, nullptr, &pos_rect);
  // 左下
  pos_rect = {pos.x, pos.y + (float)backgrnd_max_wh.y - sw->h, (float)sw->w,
              (float)sw->h};
  SDL_RenderTexture(window::renderer, sw, nullptr, &pos_rect);
  // 右上
  pos_rect = {pos.x + (float)backgrnd_max_wh.x - ne->w, pos.y, (float)ne->w,
              (float)ne->h};
  SDL_RenderTexture(window::renderer, ne, nullptr, &pos_rect);
  // 右下
  pos_rect = {pos.x + (float)backgrnd_max_wh.x - se->w,
              pos.y + (float)backgrnd_max_wh.y - se->h, (float)se->w,
              (float)se->h};
  SDL_RenderTexture(window::renderer, se, nullptr, &pos_rect);

  // 竖着的两边
  pos_rect = {pos.x, pos.y + nw->h, (float)w->w,
              (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_RenderTexture(window::renderer, w, nullptr, &pos_rect);
  pos_rect = {pos.x + (float)backgrnd_max_wh.x - e->w, pos.y + nw->h,
              (float)e->w, (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_RenderTexture(window::renderer, e, nullptr, &pos_rect);

  // 横着的两边
  pos_rect = {pos.x + nw->w, pos.y, (float)backgrnd_max_wh.x - nw->w * 2,
              (float)n->h};
  SDL_RenderTexture(window::renderer, n, nullptr, &pos_rect);

  pos_rect = {pos.x + sw->w, pos.y + (float)backgrnd_max_wh.y - s->h,
              (float)backgrnd_max_wh.x - sw->w * 2, (float)s->h};
  SDL_RenderTexture(window::renderer, s, nullptr, &pos_rect);

  //
  pos_rect = {pos.x + nw->w, pos.y + nw->h,
              (float)backgrnd_max_wh.x - nw->w * 2,
              (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_SetRenderDrawColor(window::renderer, 0, 0, 0, 100);
  SDL_SetRenderDrawBlendMode(window::renderer, SDL_BLENDMODE_BLEND);
  SDL_RenderFillRect(window::renderer, &pos_rect);

  // 标题
  static auto title =
      wz_resource::load_texture(wz_resource::ui->find(u"MiniMap.img/title"));
  pos_rect = {pos.x + 6, pos.y + 10, (float)title->w, (float)title->h};
  SDL_RenderTexture(window::renderer, title, nullptr, &pos_rect);
}

void minimap_ui_system::render_button() {
  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"MiniMap.img/BtMap"),
      wz_resource::ui->find(u"MiniMap.img/BtMin"),
      wz_resource::ui->find(u"MiniMap.img/BtMax"),
  };
  std::array buttons_rect = {
      SDL_FRect{backgrnd_max_wh.x - 42, 6, 36, 12}, //
      SDL_FRect{backgrnd_max_wh.x - 72, 6, 12, 12}, //
      SDL_FRect{backgrnd_max_wh.x - 58, 6, 12, 12}, //
  };
  std::vector<wz::Node *> buttons_node;
  std::vector<bool> r = {true, true, true};
  if (disable) {
    buttons_node = {
        buttons_nodes[1],
        buttons_nodes[2],
    };
    auto wh = load_wh();
    buttons_rect = {
        SDL_FRect{wh.x - 34, 4, 12, 12}, //
        SDL_FRect{wh.x - 20, 4, 12, 12}, //
    };
    if (load_canvas_texture() == nullptr) {
      r = {false, false};
    }
  } else {
    buttons_node = {
        buttons_nodes[0],
        buttons_nodes[1],
        buttons_nodes[2],
    };
  }

  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    auto &mouse_pos = window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (r[i] == false) {
      auto normal = wz_resource::load_texture(k->find(u"disabled/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    } else if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) &&
               cursor_in == render) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = wz_resource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
};

SDL_FPoint minimap_ui_system::load_canvas_point(SDL_FPoint point, int8_t ox,
                                                int8_t oy) {
  struct map_minimap_info {
    int32_t width;
    int32_t height;
    int32_t centerX;
    int32_t centerY;
    int8_t mag;
    int8_t scale;
  };
  static std::flat_map<uint32_t, map_minimap_info> map_minimap_cache;
  auto map_id = scene_system_instance::map_id;
  if (!map_minimap_cache.contains(map_id)) {
    map_minimap_info m_info;
    auto map_node = wz_resource::load_map_node(scene_system_instance::map_id);
    auto minimap_node = map_node->get_child(u"miniMap");
    m_info.width =
        static_cast<wz::Property<int> *>(minimap_node->get_child(u"width"))
            ->get();
    m_info.height =
        static_cast<wz::Property<int> *>(minimap_node->get_child(u"height"))
            ->get();
    m_info.centerX =
        static_cast<wz::Property<int> *>(minimap_node->get_child(u"centerX"))
            ->get();
    m_info.centerY =
        static_cast<wz::Property<int> *>(minimap_node->get_child(u"centerY"))
            ->get();
    m_info.mag =
        static_cast<wz::Property<int> *>(minimap_node->get_child(u"mag"))
            ->get();
    m_info.scale = std::pow(2, m_info.mag);
    map_minimap_cache[map_id] = m_info;
  }
  auto m_info = map_minimap_cache.at(map_id);

  SDL_FPoint position = {m_info.centerX + point.x, m_info.centerY + point.y};

  position.x = (position.x / m_info.scale) + ox;
  position.y = (position.y / m_info.scale) + oy;

  return position;
}

SDL_FPoint minimap_ui_system::load_canvas_wh() {
  SDL_FPoint r;
  if (max) {
    static auto node = wz_resource::ui->find(u"MiniMap.img/MaxMap");
    static auto nw = wz_resource::load_texture(node->get_child(u"nw"));
    static auto se = wz_resource::load_texture(node->get_child(u"se"));
    r.x = backgrnd_max_wh.x - nw->w * 2;
    r.y = backgrnd_max_wh.y - nw->h - se->h;
  } else {
    static auto node = wz_resource::ui->find(u"MiniMap.img/MinMap");
    static auto nw = wz_resource::load_texture(node->get_child(u"nw"));
    static auto se = wz_resource::load_texture(node->get_child(u"se"));
    r.x = backgrnd_min_wh.x - nw->w * 2;
    r.y = backgrnd_min_wh.y - nw->h - se->h;
  }
  return r;
}

SDL_FPoint minimap_ui_system::load_canvas_o() {
  SDL_FPoint r;
  auto canvas_texture = load_canvas_texture();
  auto canvas_wh = load_canvas_wh();
  if (max) {
    static auto node = wz_resource::ui->find(u"MiniMap.img/MaxMap");
    static auto nw = wz_resource::load_texture(node->get_child(u"nw"));
    r.x = nw->w;
    r.x += std::max(0.0f, (canvas_wh.x - canvas_texture->w) / 2);
    r.y = nw->h;
    r.y += std::max(0.0f, (canvas_wh.y - canvas_texture->h) / 2);
  } else {
    static auto node = wz_resource::ui->find(u"MiniMap.img/MinMap");
    static auto nw = wz_resource::load_texture(node->get_child(u"nw"));
    r.x = nw->w;
    r.x += std::max(0.0f, (canvas_wh.x - canvas_texture->w) / 2);
    r.y = nw->h;
    r.y += std::max(0.0f, (canvas_wh.y - canvas_texture->h) / 2);
  }
  return r;
}

SDL_FPoint minimap_ui_system::load_canvas_lt() {
  SDL_FPoint r;
  auto canvas_texture = load_canvas_texture();
  auto canvas_wh = load_canvas_wh();
  if (max) {
    static auto node = wz_resource::ui->find(u"MiniMap.img/MaxMap");
    static auto nw = wz_resource::load_texture(node->get_child(u"nw"));
    r.x = nw->w;
    r.y = nw->h;
  } else {
    static auto node = wz_resource::ui->find(u"MiniMap.img/MinMap");
    static auto nw = wz_resource::load_texture(node->get_child(u"nw"));
    r.x = nw->w;
    r.y = nw->h;
  }
  return r;
}

SDL_FRect minimap_ui_system::load_canvas_viewport() {
  SDL_FRect viewport;
  auto &self = character_game_instance::self;
  // 获取人在小地图的偏移
  auto self_pos = load_canvas_point(self.pos, -2, -4);
  auto canvas_wh = load_canvas_wh();
  auto canvas_texture = load_canvas_texture();

  viewport.w = std::min((float)canvas_texture->w, canvas_wh.x);
  viewport.h = std::min((float)canvas_texture->h, canvas_wh.y);

  viewport.x = std::max(self_pos.x - viewport.w / 2, (float)0);
  viewport.y = std::max(self_pos.y - viewport.h / 2, (float)0);

  viewport.x = std::min(viewport.x, canvas_texture->w - viewport.w);
  viewport.y = std::min(viewport.y, canvas_texture->h - viewport.h);

  return viewport;
}

void minimap_ui_system::render_canvas_life() {
  auto canvas_wh = load_canvas_wh();
  auto canvas_lt = load_canvas_lt();
  SDL_Rect clip_r = {static_cast<int>(pos.x + canvas_lt.x),
                     static_cast<int>(pos.y + canvas_lt.y),
                     static_cast<int>(canvas_wh.x),
                     static_cast<int>(canvas_wh.y)};
  SDL_SetRenderClipRect(window::renderer, &clip_r);

  const auto backgrnd_w = backgrnd_min_wh.x;
  const auto backgrnd_h = backgrnd_min_wh.y;
  static auto npc_texture = wz_resource::load_texture(
      wz_resource::map->find(u"MapHelper.img/minimap/npc"));
  static auto por_texture = wz_resource::load_texture(
      wz_resource::map->find(u"MapHelper.img/minimap/portal"));
  static auto user_texture = wz_resource::load_texture(
      wz_resource::map->find(u"MapHelper.img/minimap/user"));
  static auto other_texture = wz_resource::load_texture(
      wz_resource::map->find(u"MapHelper.img/minimap/another"));
  // render npc
  auto canvas_o = load_canvas_o();
  auto canvas_viewport = load_canvas_viewport();
  for (auto &npcs : npc_game_instance::data) {
    for (auto &npc : npcs) {
      auto npc_pos = load_canvas_point(npc.pos, -2, -4);
      if (!SDL_PointInRectFloat(&npc_pos, &canvas_viewport)) {
        continue;
      }
      npc_pos.x = npc_pos.x - canvas_viewport.x;
      npc_pos.y = npc_pos.y - canvas_viewport.y;
      SDL_FRect pos_rect = {
          pos.x + npc_pos.x + canvas_o.x - (float)npc_texture->w / 2,
          pos.y + npc_pos.y + canvas_o.y - (float)npc_texture->h / 2,
          (float)npc_texture->w, (float)npc_texture->h};
      SDL_RenderTexture(window::renderer, npc_texture, nullptr, &pos_rect);
    }
  }
  // render portal
  for (auto &portal : portal_game_instance::data | std::views::values) {
    if (portal.pt != 2) {
      continue;
    }
    auto portal_pos = load_canvas_point(portal.pos, -2, -6);
    if (!SDL_PointInRectFloat(&portal_pos, &canvas_viewport)) {
      continue;
    }
    portal_pos.x = portal_pos.x - (canvas_viewport.x);
    portal_pos.y = portal_pos.y - (canvas_viewport.y);
    SDL_FRect pos_rect = {
        pos.x + portal_pos.x + canvas_o.x - (float)por_texture->w / 2,
        pos.y + portal_pos.y + canvas_o.y - (float)por_texture->h / 2,
        (float)por_texture->w, (float)por_texture->h};
    SDL_RenderTexture(window::renderer, por_texture, nullptr, &pos_rect);
  }
  // // render others
  for (auto &other : character_game_instance::others | std::views::values) {
    auto &o_character = other.g_character;
    auto character_pos = load_canvas_point(o_character.pos, -2, -4);
    if (!SDL_PointInRectFloat(&character_pos, &canvas_viewport)) {
      continue;
    }
    character_pos.x = character_pos.x - (canvas_viewport.x);
    character_pos.y = character_pos.y - (canvas_viewport.y);
    SDL_FRect pos_rect = {
        pos.x + character_pos.x + canvas_o.x - (float)other_texture->w / 2,
        pos.y + character_pos.y + canvas_o.y - (float)other_texture->h / 2,
        (float)other_texture->w, (float)other_texture->h};
    SDL_RenderTexture(window::renderer, other_texture, nullptr, &pos_rect);
  }
  // // render self
  auto &self = character_game_instance::self;
  auto self_pos = load_canvas_point(self.pos, -2, -4);
  self_pos.x = self_pos.x - (canvas_viewport.x);
  self_pos.y = self_pos.y - (canvas_viewport.y);
  SDL_FRect pos_rect = {
      pos.x + self_pos.x + canvas_o.x - (float)user_texture->w / 2,
      pos.y + self_pos.y + canvas_o.y - (float)user_texture->h / 2,
      (float)user_texture->w, (float)user_texture->h};
  SDL_RenderTexture(window::renderer, user_texture, nullptr, &pos_rect);

  SDL_SetRenderClipRect(window::renderer, NULL);
}

void minimap_ui_system::render_mark() {
  auto info_node = map_info_game_instance::load(scene_system_instance::map_id);
  auto mark_str = static_cast<wz::Property<std::u16string> *>(
                      info_node->get_child(u"mapMark"))
                      ->get();
  auto mark_node = wz_resource::map->find(u"MapHelper.img/mark/" + mark_str);
  auto mark_texture = wz_resource::load_texture(mark_node);

  SDL_FRect pos_rect{pos.x + 6, pos.y + 24, static_cast<float>(mark_texture->w),
                     static_cast<float>(mark_texture->h)};
  SDL_RenderTexture(window::renderer, mark_texture, nullptr, &pos_rect);
}

minimap_ui_system::map_name minimap_ui_system::load_map_name(uint32_t map_id) {
  static std::flat_map<uint32_t, map_name> cache;
  if (!cache.contains(map_id)) {
    auto node = wz_resource::string->find(u"Map.img");
    for (auto [k, v] : *node->get_children()) {
      for (auto [k2, v2] : *v[0]->get_children()) {
        auto id = std::stoi(std::string{k2.begin(), k2.end()});
        map_name desc;
        desc.street_name = static_cast<wz::Property<std::u16string> *>(
                               v2[0]->get_child(u"streetName"))
                               ->get();
        desc.map_name = static_cast<wz::Property<std::u16string> *>(
                            v2[0]->get_child(u"mapName"))
                            ->get();
        if (auto d = v2[0]->get_child(u"mapDesc")) {
          desc.map_desc = static_cast<wz::Property<std::u16string> *>(d)->get();
        }
        cache[id] = desc;
      }
    }
  }
  return cache.at(map_id);
}

void minimap_ui_system::render_name() {
  auto map_id = scene_system_instance::map_id;
  auto map_d = load_map_name(map_id);
  freetype::load_aligned(true);
  freetype::load_size(14);
  auto w = freetype::load_w(map_d.street_name);
  auto h = freetype::load_lh();
  freetype::load_color(0, 0, 0, 255);
  auto x = pos.x + 49;
  auto y = pos.y + 25;
  freetype::draw_line(map_d.street_name, x, y);
  x = pos.x + 48;
  y = pos.y + 24;
  freetype::load_color(255, 255, 255, 255);
  freetype::draw_line(map_d.street_name, x, y);

  freetype::load_color(0, 0, 0, 255);
  x = pos.x + 49;
  y += h;
  freetype::draw_line(map_d.map_name, x, y);

  x = pos.x + 48;
  y -= 1;
  freetype::load_color(255, 255, 255, 255);
  freetype::draw_line(map_d.map_name, x, y);
  freetype::load_aligned(false);
}

void minimap_ui_system::render_canvas() {
  auto canvas_texture = load_canvas_texture();
  auto canvas_viewport = load_canvas_viewport();
  SDL_FRect src_rect{canvas_viewport.x, canvas_viewport.y,
                     static_cast<float>(canvas_viewport.w),
                     static_cast<float>(canvas_viewport.h)};

  auto canvas_o = load_canvas_o();
  SDL_FRect pos_rect{pos.x + canvas_o.x, pos.y + canvas_o.y,
                     static_cast<float>(canvas_viewport.w),
                     static_cast<float>(canvas_viewport.h)};

  SDL_RenderTexture(window::renderer, canvas_texture, &src_rect, &pos_rect);
}

bool minimap_ui_system::render() {
  if (!disable) {
    if (max) {
      render_max_backgrnd();
      render_mark();
      render_name();
    } else {
      render_min_backgrnd();
    }
    render_canvas();
    render_canvas_life();
  } else {
    render_mini();
  }
  render_button();
  return true;
}

bool minimap_ui_system::cursor_in() {
  SDL_FRect pos_rect;
  if (disable) {
    auto wh = load_wh();
    pos_rect = {pos.x, pos.y, wh.x, wh.y};
  } else if (!max) {
    pos_rect = {pos.x, pos.y, backgrnd_min_wh.x, backgrnd_min_wh.y};
  } else {
    pos_rect = {pos.x, pos.y, backgrnd_max_wh.x, backgrnd_max_wh.y};
  }
  return SDL_PointInRectFloat(&window::mouse_pos, &pos_rect);
}

void minimap_ui_system::event_world_map() {
  worldmap_ui_system::toggle();
  return;
}

void minimap_ui_system::event_min() {
  if (max) {
    max = false;
    disable = false;
  } else {
    disable = true;
  }
  return;
}

void minimap_ui_system::event_max() {
  if (disable) {
    if (load_canvas_texture() == nullptr) {
      return;
    }
    disable = false;
    max = false;
  } else {
    max = true;
  }
  auto &camera = camera_game_instance::camera;
  auto [w, h] = load_wh();
  pos.x = std::clamp(pos.x, (float)0, camera.w - w);
  pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  return;
}

bool minimap_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  if (disable) {
    auto wh = load_wh();
    r = {
        SDL_FRect{wh.x - 34, 4, 12, 12}, //
        SDL_FRect{wh.x - 20, 4, 12, 12}, //
    };
    fns = {
        event_min,
        event_max,
    };
  } else {
    r = {
        SDL_FRect{backgrnd_max_wh.x - 42, 6, 36, 12}, //
        SDL_FRect{backgrnd_max_wh.x - 72, 6, 12, 12}, //
        SDL_FRect{backgrnd_max_wh.x - 58, 6, 12, 12}, //
    };
    fns = {
        event_world_map,
        event_min,
        event_max,
    };
  }

  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

void minimap_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.begin(), event);
}

void minimap_ui_system::event_drag_start(SDL_Event *event) {
  auto w = load_wh().x;
  SDL_FRect pos_rect = {pos.x, pos.y, w, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void minimap_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

SDL_FPoint minimap_ui_system::load_wh() {
  if (disable) {
    auto map_id = scene_system_instance::map_id;
    auto map_name = load_map_name(map_id);
    auto s1 = map_name.street_name;
    auto s2 = map_name.map_name;
    auto s = s1 + u":" + s2;
    freetype::load_size(14);
    SDL_FPoint r{freetype::load_w(s) + 72, 20};
    return r;
  } else if (max) {
    return backgrnd_max_wh;
  } else {
    return backgrnd_min_wh;
  }
}

void minimap_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

bool minimap_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        event_top();
        event_drag_start(event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        r = !event_button(event);
      }
      event_drag_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_drag_move(event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

void minimap_ui_system::load() {
  auto map_id = scene_system_instance::map_id;
  auto map_name = load_map_name(map_id);
  auto m_name = map_name.map_name;
  auto s_name = map_name.street_name;

  freetype::load_size(14);
  auto m_w = freetype::load_w(m_name) + 53;
  auto s_w = freetype::load_w(s_name) + 53;

  auto w = std::max(m_w, s_w);
  backgrnd_max_wh.x = std::max(w, 240.0f);
  backgrnd_min_wh.x = std::max(w, 240.0f);

  if (auto texture = load_canvas_texture()) {
    backgrnd_max_wh.y = std::clamp(texture->h, 180, 220);
    backgrnd_min_wh.y = std::clamp(texture->h, 180, 220);
    max = true;
    disable = false;
  } else {
    max = false;
    disable = true;
  }
}