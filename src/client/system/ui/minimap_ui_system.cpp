#include "minimap_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
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

void MinimapUiSystem::render_mini() {
  static auto w =
      WzResource::load_texture(WzResource::ui->find(u"MiniMap.img/Min/w"));
  static auto c =
      WzResource::load_texture(WzResource::ui->find(u"MiniMap.img/Min/c"));
  static auto e =
      WzResource::load_texture(WzResource::ui->find(u"MiniMap.img/Min/e"));
  auto map_id = SceneSystemInstance::map_id;
  auto MapName = load_map_name(map_id);
  auto s1 = MapName.street_name;
  auto s2 = MapName.map_name;
  auto s = s1 + u":" + s2;
  Freetype::load_size(14);
  Freetype::load_aligned(true);
  Freetype::load_color(0, 0, 0, 255);
  auto l = load_wh().x;
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(w->w),
      static_cast<float>(w->h),
  };
  SDL_SetTextureAlphaMod(w, 162);
  SDL_RenderTexture(Window::renderer, w, nullptr, &pos_rect);

  pos_rect.x += w->w;
  pos_rect.w = l - w->w - e->w;
  SDL_SetTextureAlphaMod(c, 162);
  SDL_RenderTexture(Window::renderer, c, nullptr, &pos_rect);
  Freetype::draw_line(s, pos_rect.x, pos_rect.y);

  pos_rect.x = pos.x + l - e->w;
  pos_rect.w = e->w;
  SDL_SetTextureAlphaMod(e, 162);
  SDL_RenderTexture(Window::renderer, e, nullptr, &pos_rect);
}

static SDL_FPoint backgrnd_min_wh;

static SDL_FPoint backgrnd_max_wh;

SDL_Texture *MinimapUiSystem::load_canvas_texture() {
  auto map_node = WzResource::load_map_node(SceneSystemInstance::map_id);
  if (auto minimap_node = map_node->get_child(u"miniMap")) {
    return WzResource::load_texture(minimap_node->get_child(u"canvas"));
  }
  return nullptr;
}

void MinimapUiSystem::render_min_backgrnd() {
  static auto node = WzResource::ui->find(u"MiniMap.img/MinMap");
  static auto c = WzResource::load_texture(node->get_child(u"c"));
  static auto e = WzResource::load_texture(node->get_child(u"e"));
  static auto ne = WzResource::load_texture(node->get_child(u"ne"));
  static auto n = WzResource::load_texture(node->get_child(u"n"));
  static auto nw = WzResource::load_texture(node->get_child(u"nw"));
  static auto w = WzResource::load_texture(node->get_child(u"w"));
  static auto sw = WzResource::load_texture(node->get_child(u"sw"));
  static auto s = WzResource::load_texture(node->get_child(u"s"));
  static auto se = WzResource::load_texture(node->get_child(u"se"));

  SDL_FRect pos_rect;
  // 左上
  pos_rect = {pos.x, pos.y, (float)nw->w, (float)nw->h};
  SDL_RenderTexture(Window::renderer, nw, nullptr, &pos_rect);
  // 左下
  pos_rect = {pos.x, pos.y + (float)backgrnd_max_wh.y - sw->h, (float)sw->w,
              (float)sw->h};
  SDL_RenderTexture(Window::renderer, sw, nullptr, &pos_rect);
  // 右上
  pos_rect = {pos.x + (float)backgrnd_max_wh.x - ne->w, pos.y, (float)ne->w,
              (float)ne->h};
  SDL_RenderTexture(Window::renderer, ne, nullptr, &pos_rect);
  // 右下
  pos_rect = {pos.x + (float)backgrnd_max_wh.x - se->w,
              pos.y + (float)backgrnd_max_wh.y - se->h, (float)se->w,
              (float)se->h};
  SDL_RenderTexture(Window::renderer, se, nullptr, &pos_rect);

  // 竖着的两边
  pos_rect = {pos.x, pos.y + nw->h, (float)w->w,
              (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_RenderTexture(Window::renderer, w, nullptr, &pos_rect);
  pos_rect = {pos.x + (float)backgrnd_max_wh.x - e->w, pos.y + nw->h,
              (float)e->w, (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_RenderTexture(Window::renderer, e, nullptr, &pos_rect);

  // 横着的两边
  pos_rect = {pos.x + nw->w, pos.y, (float)backgrnd_max_wh.x - nw->w * 2,
              (float)n->h};
  SDL_RenderTexture(Window::renderer, n, nullptr, &pos_rect);

  pos_rect = {pos.x + sw->w, pos.y + (float)backgrnd_max_wh.y - s->h,
              (float)backgrnd_max_wh.x - sw->w * 2, (float)s->h};
  SDL_RenderTexture(Window::renderer, s, nullptr, &pos_rect);

  //
  pos_rect = {pos.x + nw->w, pos.y + nw->h,
              (float)backgrnd_max_wh.x - nw->w * 2,
              (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 100);
  SDL_SetRenderDrawBlendMode(Window::renderer, SDL_BLENDMODE_BLEND);
  SDL_RenderFillRect(Window::renderer, &pos_rect);

  // 标题
  static auto title =
      WzResource::load_texture(WzResource::ui->find(u"MiniMap.img/title"));
  pos_rect = {pos.x + 6, pos.y + 10, (float)title->w, (float)title->h};
  SDL_RenderTexture(Window::renderer, title, nullptr, &pos_rect);
}

void MinimapUiSystem::render_max_backgrnd() {
  static auto node = WzResource::ui->find(u"MiniMap.img/MaxMap");
  static auto e = WzResource::load_texture(node->get_child(u"e"));
  static auto ne = WzResource::load_texture(node->get_child(u"ne"));
  static auto n = WzResource::load_texture(node->get_child(u"n"));
  static auto nw = WzResource::load_texture(node->get_child(u"nw"));
  static auto w = WzResource::load_texture(node->get_child(u"w"));
  static auto sw = WzResource::load_texture(node->get_child(u"sw"));
  static auto s = WzResource::load_texture(node->get_child(u"s"));
  static auto se = WzResource::load_texture(node->get_child(u"se"));

  SDL_FRect pos_rect;
  // 左上
  pos_rect = {pos.x, pos.y, (float)nw->w, (float)nw->h};
  SDL_RenderTexture(Window::renderer, nw, nullptr, &pos_rect);
  // 左下
  pos_rect = {pos.x, pos.y + (float)backgrnd_max_wh.y - sw->h, (float)sw->w,
              (float)sw->h};
  SDL_RenderTexture(Window::renderer, sw, nullptr, &pos_rect);
  // 右上
  pos_rect = {pos.x + (float)backgrnd_max_wh.x - ne->w, pos.y, (float)ne->w,
              (float)ne->h};
  SDL_RenderTexture(Window::renderer, ne, nullptr, &pos_rect);
  // 右下
  pos_rect = {pos.x + (float)backgrnd_max_wh.x - se->w,
              pos.y + (float)backgrnd_max_wh.y - se->h, (float)se->w,
              (float)se->h};
  SDL_RenderTexture(Window::renderer, se, nullptr, &pos_rect);

  // 竖着的两边
  pos_rect = {pos.x, pos.y + nw->h, (float)w->w,
              (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_RenderTexture(Window::renderer, w, nullptr, &pos_rect);
  pos_rect = {pos.x + (float)backgrnd_max_wh.x - e->w, pos.y + nw->h,
              (float)e->w, (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_RenderTexture(Window::renderer, e, nullptr, &pos_rect);

  // 横着的两边
  pos_rect = {pos.x + nw->w, pos.y, (float)backgrnd_max_wh.x - nw->w * 2,
              (float)n->h};
  SDL_RenderTexture(Window::renderer, n, nullptr, &pos_rect);

  pos_rect = {pos.x + sw->w, pos.y + (float)backgrnd_max_wh.y - s->h,
              (float)backgrnd_max_wh.x - sw->w * 2, (float)s->h};
  SDL_RenderTexture(Window::renderer, s, nullptr, &pos_rect);

  //
  pos_rect = {pos.x + nw->w, pos.y + nw->h,
              (float)backgrnd_max_wh.x - nw->w * 2,
              (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 100);
  SDL_SetRenderDrawBlendMode(Window::renderer, SDL_BLENDMODE_BLEND);
  SDL_RenderFillRect(Window::renderer, &pos_rect);

  // 标题
  static auto title =
      WzResource::load_texture(WzResource::ui->find(u"MiniMap.img/title"));
  pos_rect = {pos.x + 6, pos.y + 10, (float)title->w, (float)title->h};
  SDL_RenderTexture(Window::renderer, title, nullptr, &pos_rect);
}

void MinimapUiSystem::render_button() {
  const static std::array buttons_nodes = {
      WzResource::ui->find(u"MiniMap.img/BtMap"),
      WzResource::ui->find(u"MiniMap.img/BtMin"),
      WzResource::ui->find(u"MiniMap.img/BtMax"),
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
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = CursorGameInstance::cursor_ui;
    if (r[i] == false) {
      auto d = WzResource::load_texture(k->find(u"disabled/0"));
      SDL_RenderTexture(Window::renderer, d, nullptr, &pos_rect);
    } else if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) &&
               cursor_in == render) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
};

SDL_FPoint MinimapUiSystem::load_canvas_point(SDL_FPoint point, int8_t ox,
                                                int8_t oy) {
  struct MapMinimapInfo {
    int32_t width;
    int32_t height;
    int32_t centerX;
    int32_t centerY;
    int8_t mag;
    int8_t scale;
  };
  static std::flat_map<uint32_t, MapMinimapInfo> map_minimap_cache;
  auto map_id = SceneSystemInstance::map_id;
  if (!map_minimap_cache.contains(map_id)) {
    MapMinimapInfo m_info;
    auto map_node = WzResource::load_map_node(SceneSystemInstance::map_id);
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

SDL_FPoint MinimapUiSystem::load_canvas_wh() {
  SDL_FPoint r;
  if (max) {
    static auto node = WzResource::ui->find(u"MiniMap.img/MaxMap");
    static auto nw = WzResource::load_texture(node->get_child(u"nw"));
    static auto se = WzResource::load_texture(node->get_child(u"se"));
    r.x = backgrnd_max_wh.x - nw->w * 2;
    r.y = backgrnd_max_wh.y - nw->h - se->h;
  } else {
    static auto node = WzResource::ui->find(u"MiniMap.img/MinMap");
    static auto nw = WzResource::load_texture(node->get_child(u"nw"));
    static auto se = WzResource::load_texture(node->get_child(u"se"));
    r.x = backgrnd_min_wh.x - nw->w * 2;
    r.y = backgrnd_min_wh.y - nw->h - se->h;
  }
  return r;
}

SDL_FPoint MinimapUiSystem::load_canvas_o() {
  SDL_FPoint r;
  auto canvas_texture = load_canvas_texture();
  auto canvas_wh = load_canvas_wh();
  if (max) {
    static auto node = WzResource::ui->find(u"MiniMap.img/MaxMap");
    static auto nw = WzResource::load_texture(node->get_child(u"nw"));
    r.x = nw->w;
    r.x += std::max(0.0f, (canvas_wh.x - canvas_texture->w) / 2);
    r.y = nw->h;
    r.y += std::max(0.0f, (canvas_wh.y - canvas_texture->h) / 2);
  } else {
    static auto node = WzResource::ui->find(u"MiniMap.img/MinMap");
    static auto nw = WzResource::load_texture(node->get_child(u"nw"));
    r.x = nw->w;
    r.x += std::max(0.0f, (canvas_wh.x - canvas_texture->w) / 2);
    r.y = nw->h;
    r.y += std::max(0.0f, (canvas_wh.y - canvas_texture->h) / 2);
  }
  return r;
}

SDL_FPoint MinimapUiSystem::load_canvas_lt() {
  SDL_FPoint r;
  auto canvas_texture = load_canvas_texture();
  auto canvas_wh = load_canvas_wh();
  if (max) {
    static auto node = WzResource::ui->find(u"MiniMap.img/MaxMap");
    static auto nw = WzResource::load_texture(node->get_child(u"nw"));
    r.x = nw->w;
    r.y = nw->h;
  } else {
    static auto node = WzResource::ui->find(u"MiniMap.img/MinMap");
    static auto nw = WzResource::load_texture(node->get_child(u"nw"));
    r.x = nw->w;
    r.y = nw->h;
  }
  return r;
}

SDL_FRect MinimapUiSystem::load_canvas_viewport() {
  SDL_FRect viewport;
  auto &self = CharacterGameInstance::self;
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

void MinimapUiSystem::render_canvas_life() {
  auto canvas_wh = load_canvas_wh();
  auto canvas_lt = load_canvas_lt();
  SDL_Rect clip_r = {static_cast<int>(pos.x + canvas_lt.x),
                     static_cast<int>(pos.y + canvas_lt.y),
                     static_cast<int>(canvas_wh.x),
                     static_cast<int>(canvas_wh.y)};
  SDL_SetRenderClipRect(Window::renderer, &clip_r);

  const auto backgrnd_w = backgrnd_min_wh.x;
  const auto backgrnd_h = backgrnd_min_wh.y;
  static auto npc_texture = WzResource::load_texture(
      WzResource::map->find(u"MapHelper.img/minimap/npc"));
  static auto npc_texture2 = WzResource::load_texture(
      WzResource::map->find(u"MapHelper.img/minimap/startnpc"));
  static auto npc_texture3 = WzResource::load_texture(
      WzResource::map->find(u"MapHelper.img/minimap/endnpc"));
  static auto por_texture = WzResource::load_texture(
      WzResource::map->find(u"MapHelper.img/minimap/portal"));
  static auto user_texture = WzResource::load_texture(
      WzResource::map->find(u"MapHelper.img/minimap/user"));
  static auto other_texture = WzResource::load_texture(
      WzResource::map->find(u"MapHelper.img/minimap/another"));
  // render npc
  auto canvas_o = load_canvas_o();
  auto canvas_viewport = load_canvas_viewport();
  for (auto &npcs : NpcGameInstance::data) {
    for (auto &npc : npcs) {
      auto npc_pos = load_canvas_point(npc.pos, 4, 2);
      if (!SDL_PointInRectFloat(&npc_pos, &canvas_viewport)) {
        continue;
      }
      SDL_Texture *texture = npc_texture;
      auto progress_complete =
          NpcGameInstance::load_progress_complete_quest(npc.id);
      if (!progress_complete.empty()) {
        texture = npc_texture3;
      } else {
        auto avaliable = NpcGameInstance::load_avaliable_quest(npc.id);
        if (!avaliable.empty()) {
          texture = npc_texture2;
        }
      }

      npc_pos.x = npc_pos.x - canvas_viewport.x;
      npc_pos.y = npc_pos.y - canvas_viewport.y;
      SDL_FRect pos_rect = {
          pos.x + npc_pos.x + canvas_o.x - (float)texture->w,
          pos.y + npc_pos.y + canvas_o.y - (float)texture->h,
          (float)texture->w,
          (float)texture->h,
      };
      SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
    }
  }
  // render portal
  for (auto &portal : PortalGameInstance::data | std::views::values) {
    if (portal.pt != 2) {
      continue;
    }
    auto portal_pos = load_canvas_point(portal.pos, 4, 2);
    if (!SDL_PointInRectFloat(&portal_pos, &canvas_viewport)) {
      continue;
    }
    portal_pos.x = portal_pos.x - (canvas_viewport.x);
    portal_pos.y = portal_pos.y - (canvas_viewport.y);
    SDL_FRect pos_rect = {
        pos.x + portal_pos.x + canvas_o.x - (float)por_texture->w,
        pos.y + portal_pos.y + canvas_o.y - (float)por_texture->h,
        (float)por_texture->w, (float)por_texture->h};
    SDL_RenderTexture(Window::renderer, por_texture, nullptr, &pos_rect);
  }
  // // render others
  for (auto &other : CharacterGameInstance::others | std::views::values) {
    auto &o_character = other.g_character;
    auto character_pos = load_canvas_point(o_character.pos, 4, 2);
    if (!SDL_PointInRectFloat(&character_pos, &canvas_viewport)) {
      continue;
    }
    character_pos.x = character_pos.x - (canvas_viewport.x);
    character_pos.y = character_pos.y - (canvas_viewport.y);
    SDL_FRect pos_rect = {
        pos.x + character_pos.x + canvas_o.x - (float)other_texture->w,
        pos.y + character_pos.y + canvas_o.y - (float)other_texture->h,
        (float)other_texture->w,
        (float)other_texture->h,
    };
    SDL_RenderTexture(Window::renderer, other_texture, nullptr, &pos_rect);
  }
  // // render self
  auto &self = CharacterGameInstance::self;
  auto self_pos = load_canvas_point(self.pos, 4, 2);
  self_pos.x = self_pos.x - (canvas_viewport.x);
  self_pos.y = self_pos.y - (canvas_viewport.y);
  SDL_FRect pos_rect = {
      pos.x + self_pos.x + canvas_o.x - (float)user_texture->w,
      pos.y + self_pos.y + canvas_o.y - (float)user_texture->h,
      (float)user_texture->w,
      (float)user_texture->h,
  };
  SDL_RenderTexture(Window::renderer, user_texture, nullptr, &pos_rect);

  SDL_SetRenderClipRect(Window::renderer, NULL);
}

void MinimapUiSystem::render_mark() {
  auto info_node = MapInfoGameInstance::load(SceneSystemInstance::map_id);
  auto mark_str = static_cast<wz::Property<std::u16string> *>(
                      info_node->get_child(u"mapMark"))
                      ->get();
  auto mark_node = WzResource::map->find(u"MapHelper.img/mark/" + mark_str);
  auto mark_texture = WzResource::load_texture(mark_node);

  SDL_FRect pos_rect{pos.x + 6, pos.y + 24, static_cast<float>(mark_texture->w),
                     static_cast<float>(mark_texture->h)};
  SDL_RenderTexture(Window::renderer, mark_texture, nullptr, &pos_rect);
}

MinimapUiSystem::MapName MinimapUiSystem::load_map_name(uint32_t map_id) {
  static std::flat_map<uint32_t, MapName> cache;
  if (!cache.contains(map_id)) {
    auto node = WzResource::string->find(u"Map.img");
    for (auto [k, v] : *node->get_children()) {
      for (auto [k2, v2] : *v[0]->get_children()) {
        auto id = std::stoi(std::string{k2.begin(), k2.end()});
        MapName desc;
        desc.street_name = static_cast<wz::Property<std::u16string> *>(
                               v2[0]->get_child(u"streetName"))
                               ->get();
        desc.map_name = static_cast<wz::Property<std::u16string> *>(
                            v2[0]->get_child(u"mapName"))
                            ->get();
        if (auto d = v2[0]->get_child(u"mapDesc")) {
          desc.map_desc = TextGameInstance::load_rstr(d);
        }
        cache[id] = desc;
      }
    }
  }
  return cache.at(map_id);
}

void MinimapUiSystem::render_name() {
  auto map_id = SceneSystemInstance::map_id;
  auto map_d = load_map_name(map_id);
  Freetype::load_aligned(true);
  Freetype::load_size(14);
  auto w = Freetype::load_w(map_d.street_name);
  auto h = Freetype::load_lh();
  Freetype::load_color(0, 0, 0, 255);
  auto x = pos.x + 49;
  auto y = pos.y + 25;
  Freetype::draw_line(map_d.street_name, x, y);
  x = pos.x + 48;
  y = pos.y + 24;
  Freetype::load_color(255, 255, 255, 255);
  Freetype::draw_line(map_d.street_name, x, y);

  Freetype::load_color(0, 0, 0, 255);
  x = pos.x + 49;
  y += h;
  Freetype::draw_line(map_d.map_name, x, y);

  x = pos.x + 48;
  y -= 1;
  Freetype::load_color(255, 255, 255, 255);
  Freetype::draw_line(map_d.map_name, x, y);
  Freetype::load_aligned(false);
}

void MinimapUiSystem::render_canvas() {
  auto canvas_texture = load_canvas_texture();
  auto canvas_viewport = load_canvas_viewport();
  SDL_FRect src_rect{
      canvas_viewport.x,
      canvas_viewport.y,
      static_cast<float>(canvas_viewport.w),
      static_cast<float>(canvas_viewport.h),
  };

  auto canvas_o = load_canvas_o();
  SDL_FRect pos_rect{
      pos.x + canvas_o.x,
      pos.y + canvas_o.y,
      static_cast<float>(canvas_viewport.w),
      static_cast<float>(canvas_viewport.h),
  };

  SDL_RenderTexture(Window::renderer, canvas_texture, &src_rect, &pos_rect);
}

bool MinimapUiSystem::render() {
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

bool MinimapUiSystem::cursor_in() {
  SDL_FRect pos_rect;
  if (disable) {
    auto wh = load_wh();
    pos_rect = {pos.x, pos.y, wh.x, wh.y};
  } else if (!max) {
    pos_rect = {pos.x, pos.y, backgrnd_min_wh.x, backgrnd_min_wh.y};
  } else {
    pos_rect = {pos.x, pos.y, backgrnd_max_wh.x, backgrnd_max_wh.y};
  }
  return SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect);
}

void MinimapUiSystem::event_world_map() {
  WorldmapUiSystem::toggle();
  return;
}

void MinimapUiSystem::event_min() {
  if (max) {
    max = false;
    disable = false;
  } else {
    disable = true;
  }
  return;
}

void MinimapUiSystem::event_max() {
  if (disable) {
    if (load_canvas_texture() == nullptr) {
      return;
    }
    disable = false;
    max = false;
  } else {
    max = true;
  }
  auto &camera = CameraGameInstance::camera;
  auto [w, h] = load_wh();
  pos.x = std::clamp(pos.x, (float)0, camera.w - w);
  pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  return;
}

bool MinimapUiSystem::event_button(SDL_Event *event) {
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
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      fns[i]();
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }

  return false;
}

void MinimapUiSystem::event_top() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void MinimapUiSystem::event_drag_start(SDL_Event *event) {
  auto w = load_wh().x;
  SDL_FRect pos_rect = {pos.x, pos.y, w, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void MinimapUiSystem::event_drag_end() {
  drag = std::nullopt;
  return;
}

SDL_FPoint MinimapUiSystem::load_wh() {
  if (disable) {
    auto map_id = SceneSystemInstance::map_id;
    auto MapName = load_map_name(map_id);
    auto s1 = MapName.street_name;
    auto s2 = MapName.map_name;
    auto s = s1 + u":" + s2;
    Freetype::load_size(14);
    SDL_FPoint r{Freetype::load_w(s) + 72, 20};
    return r;
  } else if (max) {
    return backgrnd_max_wh;
  } else {
    return backgrnd_min_wh;
  }
}

void MinimapUiSystem::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void MinimapUiSystem::event_motion_bt_audio(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  std::vector<SDL_FRect> buttons_rect;
  if (disable) {
    auto wh = load_wh();
    buttons_rect = {
        SDL_FRect{wh.x - 34, 4, 12, 12}, //
        SDL_FRect{wh.x - 20, 4, 12, 12}, //
    };
  } else {
    buttons_rect = {
        SDL_FRect{backgrnd_max_wh.x - 42, 6, 36, 12}, //
        SDL_FRect{backgrnd_max_wh.x - 72, 6, 12, 12}, //
        SDL_FRect{backgrnd_max_wh.x - 58, 6, 12, 12}, //
    };
  }
  static int8_t bt_audio = -1;
  auto &mouse_pos = Window::mouse_pos;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      bt_in = i;
      break;
    }
  }
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::load_audio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool MinimapUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        event_top();
        event_drag_start(event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        r = !event_button(event);
      }
      event_drag_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_motion_bt_audio(event);
    event_drag_move(event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

void MinimapUiSystem::load() {
  auto map_id = SceneSystemInstance::map_id;
  auto MapName = load_map_name(map_id);
  auto m_name = MapName.map_name;
  auto s_name = MapName.street_name;

  Freetype::load_size(14);
  auto m_w = Freetype::load_w(m_name) + 53;
  auto s_w = Freetype::load_w(s_name) + 53;

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