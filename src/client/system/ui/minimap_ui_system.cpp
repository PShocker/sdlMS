#include "minimap_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <array>
#include <cstdint>
#include <flat_map>

void minimap_ui_system::render_mini() {}

static SDL_FPoint backgrnd_min_wh = {250, 165};

static SDL_FPoint backgrnd_max_wh = {250, 220};

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

SDL_FPoint minimap_ui_system::load_canvas_point(SDL_FPoint point, int8_t ox,
                                                int8_t oy) {
  struct map_minimap_info {
    int32_t width;
    int32_t height;
    int32_t centerX;
    int32_t centerY;
    int32_t mag;
    int32_t scale;
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
  auto map_node = wz_resource::load_map_node(scene_system_instance::map_id);
  auto minimap_node = map_node->get_child(u"miniMap");
  auto canvas_texture =
      wz_resource::load_texture(minimap_node->get_child(u"canvas"));
  if (max) {
    static auto node = wz_resource::ui->find(u"MiniMap.img/MaxMap");
    static auto nw = wz_resource::load_texture(node->get_child(u"nw"));
    r.x = (backgrnd_max_wh.x - canvas_texture->w) / 2 ;
    r.y = nw->h;
  } else {
    static auto node = wz_resource::ui->find(u"MiniMap.img/MinMap");
    static auto nw = wz_resource::load_texture(node->get_child(u"nw"));
    r.x = (backgrnd_min_wh.x - canvas_texture->w) / 2;
    r.y = nw->h;
  }
  return r;
}

void minimap_ui_system::render_canvas_life() {
  auto map_node = wz_resource::load_map_node(scene_system_instance::map_id);
  auto minimap_node = map_node->get_child(u"miniMap");

  const auto backgrnd_w = backgrnd_min_wh.x;
  const auto backgrnd_h = backgrnd_min_wh.y;
  auto canvas_texture =
      wz_resource::load_texture(minimap_node->get_child(u"canvas"));
  static auto npc_texture = wz_resource::load_texture(
      wz_resource::map->find(u"MapHelper.img/minimap/npc"));
  static auto por_texture = wz_resource::load_texture(
      wz_resource::map->find(u"MapHelper.img/minimap/portal"));
  static auto user_texture = wz_resource::load_texture(
      wz_resource::map->find(u"MapHelper.img/minimap/user"));
  // render npc
  auto canvas_o = load_canvas_o();
  for (auto &npcs : npc_game_instance::data) {
    for (auto &npc : npcs) {
      auto n_pos = load_canvas_point(npc.pos, -2, -4);
      SDL_FRect pos_rect = {pos.x + n_pos.x + canvas_o.x - npc_texture->w / 2,
                            pos.y + n_pos.y + canvas_o.y - npc_texture->h / 2,
                            (float)npc_texture->w, (float)npc_texture->h};
      SDL_RenderTexture(window::renderer, npc_texture, nullptr, &pos_rect);
    }
  }
  // render portal
  for (auto &portal : portal_game_instance::data) {
    if (portal.pt != 2) {
      continue;
    }
    auto p_pos = load_canvas_point(portal.pos, -2, -6);
    SDL_FRect pos_rect = {pos.x + p_pos.x + canvas_o.x - por_texture->w / 2,
                          pos.y + p_pos.y + canvas_o.y - por_texture->h / 2,
                          (float)por_texture->w, (float)por_texture->h};
    SDL_RenderTexture(window::renderer, por_texture, nullptr, &pos_rect);
  }
  // // render users
  // // todo

  // // render self
  auto &self = character_game_instance::self;
  auto s_pos = load_canvas_point(self.pos, -2, -4);
  SDL_FRect pos_rect = {pos.x + s_pos.x + canvas_o.x - user_texture->w / 2,
                        pos.y + s_pos.y + canvas_o.y - user_texture->h / 2,
                        (float)user_texture->w, (float)user_texture->h};
  SDL_RenderTexture(window::renderer, user_texture, nullptr, &pos_rect);
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

  auto canvas_o = load_canvas_o();

  SDL_FRect pos_rect{pos.x + canvas_o.x, pos.y + canvas_o.y,
                     static_cast<float>(canvas_texture->w),
                     static_cast<float>(canvas_texture->h)};

  SDL_RenderTexture(window::renderer, canvas_texture, nullptr, &pos_rect);
}

bool minimap_ui_system::render() {
  if (!disable) {
    if (max) {
      render_max_backgrnd();
    } else {
      render_min_backgrnd();
    }
    render_button();
    render_canvas();
    render_canvas_life();
  } else {
    render_mini();
  }
  return true;
}

bool minimap_ui_system::cursor_in() {
  SDL_FRect pos_rect = {pos.x, pos.y, backgrnd_min_wh.x, backgrnd_min_wh.y};
  return SDL_PointInRectFloat(&window::mouse_pos, &pos_rect);
}

bool minimap_ui_system::event_button(SDL_Event *event) { return true; }

void minimap_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void minimap_ui_system::event_drag_start(SDL_Event *event) {
  SDL_FRect pos_rect = {pos.x, pos.y, backgrnd_min_wh.x, 18};
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

void minimap_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
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
