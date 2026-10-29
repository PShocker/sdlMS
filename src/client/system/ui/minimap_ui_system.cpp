#include "src/client/system/ui/minimap_ui_system.h"

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
#include "src/client/system/ui/component/ui_component.h"
#include "src/client/system/ui/worldmap_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <flat_map>
#include <ranges>
#include <string>
#include <vector>

namespace sdlms {

void MinimapUiSystem::RenderMini() {
  static auto w =
      WzResource::LoadTexture(WzResource::ui_->find(u"MiniMap.img/Min/w"));
  static auto c =
      WzResource::LoadTexture(WzResource::ui_->find(u"MiniMap.img/Min/c"));
  static auto e =
      WzResource::LoadTexture(WzResource::ui_->find(u"MiniMap.img/Min/e"));
  auto map_id = SceneSystemInstance::map_id_;
  auto MapName = LoadMapName(map_id);
  auto s1 = MapName.street_name;
  auto s2 = MapName.map_name;
  auto s = s1 + u":" + s2;
  Freetype::LoadSize(14);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(0, 0, 0, 255);
  auto l = LoadWh().x;
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(w->w),
      static_cast<float>(w->h),
  };
  SDL_SetTextureAlphaMod(w, 162);
  SDL_RenderTexture(Window::renderer_, w, nullptr, &pos_rect);

  pos_rect.x += w->w;
  pos_rect.w = l - w->w - e->w;
  SDL_SetTextureAlphaMod(c, 162);
  SDL_RenderTexture(Window::renderer_, c, nullptr, &pos_rect);
  Freetype::DrawLine(s, pos_rect.x, pos_rect.y);

  pos_rect.x = pos_.x + l - e->w;
  pos_rect.w = e->w;
  SDL_SetTextureAlphaMod(e, 162);
  SDL_RenderTexture(Window::renderer_, e, nullptr, &pos_rect);
}

static SDL_FPoint backgrnd_min_wh;

static SDL_FPoint backgrnd_max_wh;

SDL_Texture* MinimapUiSystem::LoadCanvasTexture() {
  auto map_node = WzResource::LoadMapNode(SceneSystemInstance::map_id_);
  if (auto minimap_node = map_node->get_child(u"miniMap")) {
    return WzResource::LoadTexture(minimap_node->get_child(u"canvas"));
  }
  return nullptr;
}

void MinimapUiSystem::RenderMinBackgrnd() {
  static auto node = WzResource::ui_->find(u"MiniMap.img/MinMap");
  static auto c = WzResource::LoadTexture(node->get_child(u"c"));
  static auto e = WzResource::LoadTexture(node->get_child(u"e"));
  static auto ne = WzResource::LoadTexture(node->get_child(u"ne"));
  static auto n = WzResource::LoadTexture(node->get_child(u"n"));
  static auto nw = WzResource::LoadTexture(node->get_child(u"nw"));
  static auto w = WzResource::LoadTexture(node->get_child(u"w"));
  static auto sw = WzResource::LoadTexture(node->get_child(u"sw"));
  static auto s = WzResource::LoadTexture(node->get_child(u"s"));
  static auto se = WzResource::LoadTexture(node->get_child(u"se"));

  SDL_FRect pos_rect;
  // 左上
  pos_rect = {pos_.x, pos_.y, (float)nw->w, (float)nw->h};
  SDL_RenderTexture(Window::renderer_, nw, nullptr, &pos_rect);
  // 左下
  pos_rect = {pos_.x, pos_.y + (float)backgrnd_max_wh.y - sw->h, (float)sw->w,
              (float)sw->h};
  SDL_RenderTexture(Window::renderer_, sw, nullptr, &pos_rect);
  // 右上
  pos_rect = {pos_.x + (float)backgrnd_max_wh.x - ne->w, pos_.y, (float)ne->w,
              (float)ne->h};
  SDL_RenderTexture(Window::renderer_, ne, nullptr, &pos_rect);
  // 右下
  pos_rect = {pos_.x + (float)backgrnd_max_wh.x - se->w,
              pos_.y + (float)backgrnd_max_wh.y - se->h, (float)se->w,
              (float)se->h};
  SDL_RenderTexture(Window::renderer_, se, nullptr, &pos_rect);

  // 竖着的两边
  pos_rect = {pos_.x, pos_.y + nw->h, (float)w->w,
              (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_RenderTexture(Window::renderer_, w, nullptr, &pos_rect);
  pos_rect = {pos_.x + (float)backgrnd_max_wh.x - e->w, pos_.y + nw->h,
              (float)e->w, (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_RenderTexture(Window::renderer_, e, nullptr, &pos_rect);

  // 横着的两边
  pos_rect = {pos_.x + nw->w, pos_.y, (float)backgrnd_max_wh.x - nw->w * 2,
              (float)n->h};
  SDL_RenderTexture(Window::renderer_, n, nullptr, &pos_rect);

  pos_rect = {pos_.x + sw->w, pos_.y + (float)backgrnd_max_wh.y - s->h,
              (float)backgrnd_max_wh.x - sw->w * 2, (float)s->h};
  SDL_RenderTexture(Window::renderer_, s, nullptr, &pos_rect);

  //
  pos_rect = {pos_.x + nw->w, pos_.y + nw->h,
              (float)backgrnd_max_wh.x - nw->w * 2,
              (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, 100);
  SDL_SetRenderDrawBlendMode(Window::renderer_, SDL_BLENDMODE_BLEND);
  SDL_RenderFillRect(Window::renderer_, &pos_rect);

  // 标题
  static auto title =
      WzResource::LoadTexture(WzResource::ui_->find(u"MiniMap.img/title"));
  pos_rect = {pos_.x + 6, pos_.y + 10, (float)title->w, (float)title->h};
  SDL_RenderTexture(Window::renderer_, title, nullptr, &pos_rect);
}

void MinimapUiSystem::RenderMaxBackgrnd() {
  static auto node = WzResource::ui_->find(u"MiniMap.img/MaxMap");
  static auto e = WzResource::LoadTexture(node->get_child(u"e"));
  static auto ne = WzResource::LoadTexture(node->get_child(u"ne"));
  static auto n = WzResource::LoadTexture(node->get_child(u"n"));
  static auto nw = WzResource::LoadTexture(node->get_child(u"nw"));
  static auto w = WzResource::LoadTexture(node->get_child(u"w"));
  static auto sw = WzResource::LoadTexture(node->get_child(u"sw"));
  static auto s = WzResource::LoadTexture(node->get_child(u"s"));
  static auto se = WzResource::LoadTexture(node->get_child(u"se"));

  SDL_FRect pos_rect;
  // 左上
  pos_rect = {pos_.x, pos_.y, (float)nw->w, (float)nw->h};
  SDL_RenderTexture(Window::renderer_, nw, nullptr, &pos_rect);
  // 左下
  pos_rect = {pos_.x, pos_.y + (float)backgrnd_max_wh.y - sw->h, (float)sw->w,
              (float)sw->h};
  SDL_RenderTexture(Window::renderer_, sw, nullptr, &pos_rect);
  // 右上
  pos_rect = {pos_.x + (float)backgrnd_max_wh.x - ne->w, pos_.y, (float)ne->w,
              (float)ne->h};
  SDL_RenderTexture(Window::renderer_, ne, nullptr, &pos_rect);
  // 右下
  pos_rect = {pos_.x + (float)backgrnd_max_wh.x - se->w,
              pos_.y + (float)backgrnd_max_wh.y - se->h, (float)se->w,
              (float)se->h};
  SDL_RenderTexture(Window::renderer_, se, nullptr, &pos_rect);

  // 竖着的两边
  pos_rect = {pos_.x, pos_.y + nw->h, (float)w->w,
              (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_RenderTexture(Window::renderer_, w, nullptr, &pos_rect);
  pos_rect = {pos_.x + (float)backgrnd_max_wh.x - e->w, pos_.y + nw->h,
              (float)e->w, (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_RenderTexture(Window::renderer_, e, nullptr, &pos_rect);

  // 横着的两边
  pos_rect = {pos_.x + nw->w, pos_.y, (float)backgrnd_max_wh.x - nw->w * 2,
              (float)n->h};
  SDL_RenderTexture(Window::renderer_, n, nullptr, &pos_rect);

  pos_rect = {pos_.x + sw->w, pos_.y + (float)backgrnd_max_wh.y - s->h,
              (float)backgrnd_max_wh.x - sw->w * 2, (float)s->h};
  SDL_RenderTexture(Window::renderer_, s, nullptr, &pos_rect);

  //
  pos_rect = {pos_.x + nw->w, pos_.y + nw->h,
              (float)backgrnd_max_wh.x - nw->w * 2,
              (float)backgrnd_max_wh.y - nw->h - se->h};
  SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, 100);
  SDL_SetRenderDrawBlendMode(Window::renderer_, SDL_BLENDMODE_BLEND);
  SDL_RenderFillRect(Window::renderer_, &pos_rect);

  // 标题
  static auto title =
      WzResource::LoadTexture(WzResource::ui_->find(u"MiniMap.img/title"));
  pos_rect = {pos_.x + 6, pos_.y + 10, (float)title->w, (float)title->h};
  SDL_RenderTexture(Window::renderer_, title, nullptr, &pos_rect);
}

UIComponents& MinimapUiSystem::GetComponent() {
  components_.clear();
  const static std::array buttons_nodes = {
      WzResource::ui_->find(u"MiniMap.img/BtMap"),
      WzResource::ui_->find(u"MiniMap.img/BtMin"),
      WzResource::ui_->find(u"MiniMap.img/BtMax"),
  };
  std::array buttons_rect = {
      SDL_FRect{backgrnd_max_wh.x - 72, 6, 12, 12},
      SDL_FRect{backgrnd_max_wh.x - 58, 6, 12, 12},
  };
  if (disable_) {
    auto wh = LoadWh();
    buttons_rect = {
        SDL_FRect{wh.x - 34, 4, 12, 12},
        SDL_FRect{wh.x - 20, 4, 12, 12},
    };
    const bool enable = LoadCanvasTexture() != nullptr;
    components_.push_back(
        Button{.node = buttons_nodes[1],
               .rect = SDL_FRect{buttons_rect[0].x + pos_.x,
                                 buttons_rect[0].y + pos_.y, 12, 12},
               .callback = EventMin,
               .enable = enable});
    components_.push_back(
        Button{.node = buttons_nodes[2],
               .rect = SDL_FRect{buttons_rect[1].x + pos_.x,
                                 buttons_rect[1].y + pos_.y, 12, 12},
               .callback = EventMax,
               .enable = enable});
  } else {
    components_.push_back(Button{
        .node = buttons_nodes[0],
        .rect = SDL_FRect{backgrnd_max_wh.x - 42 + pos_.x, 6 + pos_.y, 36, 12},
        .callback = EventWorldMap});
    components_.push_back(
        Button{.node = buttons_nodes[1],
               .rect = SDL_FRect{buttons_rect[0].x + pos_.x,
                                 buttons_rect[0].y + pos_.y, 12, 12},
               .callback = EventMin});
    components_.push_back(
        Button{.node = buttons_nodes[2],
               .rect = SDL_FRect{buttons_rect[1].x + pos_.x,
                                 buttons_rect[1].y + pos_.y, 12, 12},
               .callback = EventMax});
  }
  return components_;
}

void MinimapUiSystem::RenderUI() {
  RenderComponents(GetComponent());
};

SDL_FPoint MinimapUiSystem::LoadCanvasPoint(SDL_FPoint point, int8_t ox,
                                            int8_t oy) {
  struct MapMinimapInfo {
    int32_t width;
    int32_t height;
    int32_t center_x;
    int32_t center_y;
    int8_t mag;
    int8_t scale;
  };
  static std::flat_map<uint32_t, MapMinimapInfo> map_minimap_cache;
  auto map_id = SceneSystemInstance::map_id_;
  if (!map_minimap_cache.contains(map_id)) {
    MapMinimapInfo m_info;
    auto map_node = WzResource::LoadMapNode(SceneSystemInstance::map_id_);
    auto minimap_node = map_node->get_child(u"miniMap");
    m_info.width =
        static_cast<wz::Property<int>*>(minimap_node->get_child(u"width"))
            ->get();
    m_info.height =
        static_cast<wz::Property<int>*>(minimap_node->get_child(u"height"))
            ->get();
    m_info.center_x =
        static_cast<wz::Property<int>*>(minimap_node->get_child(u"centerX"))
            ->get();
    m_info.center_y =
        static_cast<wz::Property<int>*>(minimap_node->get_child(u"centerY"))
            ->get();
    m_info.mag =
        static_cast<wz::Property<int>*>(minimap_node->get_child(u"mag"))->get();
    m_info.scale = std::pow(2, m_info.mag);
    map_minimap_cache[map_id] = m_info;
  }
  auto m_info = map_minimap_cache.at(map_id);

  SDL_FPoint position = {m_info.center_x + point.x, m_info.center_y + point.y};

  position.x = (position.x / m_info.scale) + ox;
  position.y = (position.y / m_info.scale) + oy;

  return position;
}

SDL_FPoint MinimapUiSystem::LoadCanvasWh() {
  SDL_FPoint r;
  if (max_) {
    static auto node = WzResource::ui_->find(u"MiniMap.img/MaxMap");
    static auto nw = WzResource::LoadTexture(node->get_child(u"nw"));
    static auto se = WzResource::LoadTexture(node->get_child(u"se"));
    r.x = backgrnd_max_wh.x - nw->w * 2;
    r.y = backgrnd_max_wh.y - nw->h - se->h;
  } else {
    static auto node = WzResource::ui_->find(u"MiniMap.img/MinMap");
    static auto nw = WzResource::LoadTexture(node->get_child(u"nw"));
    static auto se = WzResource::LoadTexture(node->get_child(u"se"));
    r.x = backgrnd_min_wh.x - nw->w * 2;
    r.y = backgrnd_min_wh.y - nw->h - se->h;
  }
  return r;
}

SDL_FPoint MinimapUiSystem::LoadCanvasO() {
  SDL_FPoint r;
  auto canvas_texture = LoadCanvasTexture();
  auto canvas_wh = LoadCanvasWh();
  if (max_) {
    static auto node = WzResource::ui_->find(u"MiniMap.img/MaxMap");
    static auto nw = WzResource::LoadTexture(node->get_child(u"nw"));
    r.x = nw->w;
    r.x += std::max(0.0f, (canvas_wh.x - canvas_texture->w) / 2);
    r.y = nw->h;
    r.y += std::max(0.0f, (canvas_wh.y - canvas_texture->h) / 2);
  } else {
    static auto node = WzResource::ui_->find(u"MiniMap.img/MinMap");
    static auto nw = WzResource::LoadTexture(node->get_child(u"nw"));
    r.x = nw->w;
    r.x += std::max(0.0f, (canvas_wh.x - canvas_texture->w) / 2);
    r.y = nw->h;
    r.y += std::max(0.0f, (canvas_wh.y - canvas_texture->h) / 2);
  }
  return r;
}

SDL_FPoint MinimapUiSystem::LoadCanvasLt() {
  SDL_FPoint r;
  auto canvas_texture = LoadCanvasTexture();
  auto canvas_wh = LoadCanvasWh();
  if (max_) {
    static auto node = WzResource::ui_->find(u"MiniMap.img/MaxMap");
    static auto nw = WzResource::LoadTexture(node->get_child(u"nw"));
    r.x = nw->w;
    r.y = nw->h;
  } else {
    static auto node = WzResource::ui_->find(u"MiniMap.img/MinMap");
    static auto nw = WzResource::LoadTexture(node->get_child(u"nw"));
    r.x = nw->w;
    r.y = nw->h;
  }
  return r;
}

SDL_FRect MinimapUiSystem::LoadCanvasViewport() {
  SDL_FRect viewport;
  auto& self = CharacterGameInstance::self_;
  // 获取人在小地图的偏移
  auto self_pos = LoadCanvasPoint(self.pos, -2, -4);
  auto canvas_wh = LoadCanvasWh();
  auto canvas_texture = LoadCanvasTexture();

  viewport.w = std::min((float)canvas_texture->w, canvas_wh.x);
  viewport.h = std::min((float)canvas_texture->h, canvas_wh.y);

  viewport.x = std::max(self_pos.x - viewport.w / 2, (float)0);
  viewport.y = std::max(self_pos.y - viewport.h / 2, (float)0);

  viewport.x = std::min(viewport.x, canvas_texture->w - viewport.w);
  viewport.y = std::min(viewport.y, canvas_texture->h - viewport.h);

  return viewport;
}

void MinimapUiSystem::RenderCanvasLife() {
  auto canvas_wh = LoadCanvasWh();
  auto canvas_lt = LoadCanvasLt();
  SDL_Rect clip_r = {static_cast<int>(pos_.x + canvas_lt.x),
                     static_cast<int>(pos_.y + canvas_lt.y),
                     static_cast<int>(canvas_wh.x),
                     static_cast<int>(canvas_wh.y)};
  SDL_SetRenderClipRect(Window::renderer_, &clip_r);

  const auto backgrnd_w = backgrnd_min_wh.x;
  const auto backgrnd_h = backgrnd_min_wh.y;
  static auto npc_texture = WzResource::LoadTexture(
      WzResource::map_->find(u"MapHelper.img/minimap/npc"));
  static auto npc_texture2 = WzResource::LoadTexture(
      WzResource::map_->find(u"MapHelper.img/minimap/startnpc"));
  static auto npc_texture3 = WzResource::LoadTexture(
      WzResource::map_->find(u"MapHelper.img/minimap/endnpc"));
  static auto por_texture = WzResource::LoadTexture(
      WzResource::map_->find(u"MapHelper.img/minimap/portal"));
  static auto user_texture = WzResource::LoadTexture(
      WzResource::map_->find(u"MapHelper.img/minimap/user"));
  static auto other_texture = WzResource::LoadTexture(
      WzResource::map_->find(u"MapHelper.img/minimap/another"));
  // render npc
  auto canvas_o = LoadCanvasO();
  auto canvas_viewport = LoadCanvasViewport();
  for (auto& npcs : NpcGameInstance::data_) {
    for (auto& npc : npcs) {
      auto npc_pos = LoadCanvasPoint(npc.pos, 4, 2);
      if (!SDL_PointInRectFloat(&npc_pos, &canvas_viewport)) {
        continue;
      }
      SDL_Texture* texture = npc_texture;
      auto progress_complete =
          NpcGameInstance::LoadProgressCompleteQuest(npc.id);
      if (!progress_complete.empty()) {
        texture = npc_texture3;
      } else {
        auto avaliable = NpcGameInstance::LoadAvaliableQuest(npc.id);
        if (!avaliable.empty()) {
          texture = npc_texture2;
        }
      }

      npc_pos.x = npc_pos.x - canvas_viewport.x;
      npc_pos.y = npc_pos.y - canvas_viewport.y;
      SDL_FRect pos_rect = {
          pos_.x + npc_pos.x + canvas_o.x - (float)texture->w,
          pos_.y + npc_pos.y + canvas_o.y - (float)texture->h,
          (float)texture->w,
          (float)texture->h,
      };
      SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
    }
  }
  // render portal
  for (auto& portal : PortalGameInstance::data_ | std::views::values) {
    if (portal.pt != 2) {
      continue;
    }
    auto portal_pos = LoadCanvasPoint(portal.pos, 4, 2);
    if (!SDL_PointInRectFloat(&portal_pos, &canvas_viewport)) {
      continue;
    }
    portal_pos.x = portal_pos.x - (canvas_viewport.x);
    portal_pos.y = portal_pos.y - (canvas_viewport.y);
    SDL_FRect pos_rect = {
        pos_.x + portal_pos.x + canvas_o.x - (float)por_texture->w,
        pos_.y + portal_pos.y + canvas_o.y - (float)por_texture->h,
        (float)por_texture->w, (float)por_texture->h};
    SDL_RenderTexture(Window::renderer_, por_texture, nullptr, &pos_rect);
  }
  // // render others
  for (auto& other : CharacterGameInstance::others_ | std::views::values) {
    auto& o_character = other.g_character;
    auto character_pos = LoadCanvasPoint(o_character.pos, 4, 2);
    if (!SDL_PointInRectFloat(&character_pos, &canvas_viewport)) {
      continue;
    }
    character_pos.x = character_pos.x - (canvas_viewport.x);
    character_pos.y = character_pos.y - (canvas_viewport.y);
    SDL_FRect pos_rect = {
        pos_.x + character_pos.x + canvas_o.x - (float)other_texture->w,
        pos_.y + character_pos.y + canvas_o.y - (float)other_texture->h,
        (float)other_texture->w,
        (float)other_texture->h,
    };
    SDL_RenderTexture(Window::renderer_, other_texture, nullptr, &pos_rect);
  }
  // // render self
  auto& self = CharacterGameInstance::self_;
  auto self_pos = LoadCanvasPoint(self.pos, 4, 2);
  self_pos.x = self_pos.x - (canvas_viewport.x);
  self_pos.y = self_pos.y - (canvas_viewport.y);
  SDL_FRect pos_rect = {
      pos_.x + self_pos.x + canvas_o.x - (float)user_texture->w,
      pos_.y + self_pos.y + canvas_o.y - (float)user_texture->h,
      (float)user_texture->w,
      (float)user_texture->h,
  };
  SDL_RenderTexture(Window::renderer_, user_texture, nullptr, &pos_rect);

  SDL_SetRenderClipRect(Window::renderer_, NULL);
}

void MinimapUiSystem::RenderMark() {
  auto info_node = MapInfoGameInstance::Load(SceneSystemInstance::map_id_);
  auto mark_str = static_cast<wz::Property<std::u16string>*>(
                      info_node->get_child(u"mapMark"))
                      ->get();
  auto mark_node = WzResource::map_->find(u"MapHelper.img/mark/" + mark_str);
  auto mark_texture = WzResource::LoadTexture(mark_node);

  SDL_FRect pos_rect{pos_.x + 6, pos_.y + 24,
                     static_cast<float>(mark_texture->w),
                     static_cast<float>(mark_texture->h)};
  SDL_RenderTexture(Window::renderer_, mark_texture, nullptr, &pos_rect);
}

MinimapUiSystem::MapName MinimapUiSystem::LoadMapName(uint32_t map_id) {
  static std::flat_map<uint32_t, MapName> cache;
  if (!cache.contains(map_id)) {
    auto node = WzResource::string_->find(u"Map.img");
    for (auto [k, v] : *node->get_children()) {
      for (auto [k2, v2] : *v[0]->get_children()) {
        auto id = std::stoi(std::string{k2.begin(), k2.end()});
        MapName desc;
        desc.street_name = static_cast<wz::Property<std::u16string>*>(
                               v2[0]->get_child(u"streetName"))
                               ->get();
        desc.map_name = static_cast<wz::Property<std::u16string>*>(
                            v2[0]->get_child(u"mapName"))
                            ->get();
        if (auto d = v2[0]->get_child(u"mapDesc")) {
          desc.map_desc = TextGameInstance::LoadRstr(d);
        }
        cache[id] = desc;
      }
    }
  }
  return cache.at(map_id);
}

void MinimapUiSystem::RenderName() {
  auto map_id = SceneSystemInstance::map_id_;
  auto map_d = LoadMapName(map_id);
  Freetype::LoadAligned(true);
  Freetype::LoadSize(14);
  auto w = Freetype::LoadW(map_d.street_name);
  auto h = Freetype::LoadLh();
  Freetype::LoadColor(0, 0, 0, 255);
  auto x = pos_.x + 49;
  auto y = pos_.y + 25;
  Freetype::DrawLine(map_d.street_name, x, y);
  x = pos_.x + 48;
  y = pos_.y + 24;
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(map_d.street_name, x, y);

  Freetype::LoadColor(0, 0, 0, 255);
  x = pos_.x + 49;
  y += h;
  Freetype::DrawLine(map_d.map_name, x, y);

  x = pos_.x + 48;
  y -= 1;
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(map_d.map_name, x, y);
  Freetype::LoadAligned(false);
}

void MinimapUiSystem::RenderCanvas() {
  auto canvas_texture = LoadCanvasTexture();
  auto canvas_viewport = LoadCanvasViewport();
  SDL_FRect src_rect{
      canvas_viewport.x,
      canvas_viewport.y,
      static_cast<float>(canvas_viewport.w),
      static_cast<float>(canvas_viewport.h),
  };

  auto canvas_o = LoadCanvasO();
  SDL_FRect pos_rect{
      pos_.x + canvas_o.x,
      pos_.y + canvas_o.y,
      static_cast<float>(canvas_viewport.w),
      static_cast<float>(canvas_viewport.h),
  };

  SDL_RenderTexture(Window::renderer_, canvas_texture, &src_rect, &pos_rect);
}

bool MinimapUiSystem::Render() {
  if (!disable_) {
    if (max_) {
      RenderMaxBackgrnd();
      RenderMark();
      RenderName();
    } else {
      RenderMinBackgrnd();
    }
    RenderCanvas();
    RenderCanvasLife();
  } else {
    RenderMini();
  }
  RenderUI();
  return true;
}

bool MinimapUiSystem::CursorIn() {
  SDL_FRect pos_rect;
  if (disable_) {
    auto wh = LoadWh();
    pos_rect = {pos_.x, pos_.y, wh.x, wh.y};
  } else if (!max_) {
    pos_rect = {pos_.x, pos_.y, backgrnd_min_wh.x, backgrnd_min_wh.y};
  } else {
    pos_rect = {pos_.x, pos_.y, backgrnd_max_wh.x, backgrnd_max_wh.y};
  }
  return SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect);
}

void MinimapUiSystem::EventWorldMap() {
  WorldmapUiSystem::Toggle();
  return;
}

void MinimapUiSystem::EventMin() {
  if (max_) {
    max_ = false;
    disable_ = false;
  } else {
    disable_ = true;
  }
  return;
}

void MinimapUiSystem::EventMax() {
  if (disable_) {
    if (LoadCanvasTexture() == nullptr) {
      return;
    }
    disable_ = false;
    max_ = false;
  } else {
    max_ = true;
  }
  auto& camera = CameraGameInstance::camera_;
  auto [w, h] = LoadWh();
  pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
  pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  return;
}

bool MinimapUiSystem::EventButton(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), true);
}

void MinimapUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void MinimapUiSystem::EventDragStart(SDL_Event* event) {
  auto w = LoadWh().x;
  SDL_FRect pos_rect = {pos_.x, pos_.y, w, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
  }
  return;
}

void MinimapUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

SDL_FPoint MinimapUiSystem::LoadWh() {
  if (disable_) {
    auto map_id = SceneSystemInstance::map_id_;
    auto MapName = LoadMapName(map_id);
    auto s1 = MapName.street_name;
    auto s2 = MapName.map_name;
    auto s = s1 + u":" + s2;
    Freetype::LoadSize(14);
    SDL_FPoint r{Freetype::LoadW(s) + 72, 20};
    return r;
  } else if (max_) {
    return backgrnd_max_wh;
  } else {
    return backgrnd_min_wh;
  }
}

void MinimapUiSystem::EventDragMove(SDL_Event* event) {
  if (drag_.has_value()) {
    pos_ = {event->motion.x + drag_->x, event->motion.y + drag_->y};
    auto& camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  }
  return;
}

void MinimapUiSystem::EventMotionBtAudio(SDL_Event* event) {
  static int8_t bt_audio = -1;
  int8_t bt_in = static_cast<int8_t>(ButtonHoverIndex(GetComponent()));
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool MinimapUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          EventTop();
          EventDragStart(event);
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = !EventButton(event);
        }
        EventDragEnd();
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventMotionBtAudio(event);
      EventDragMove(event);
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

void MinimapUiSystem::Load() {
  auto map_id = SceneSystemInstance::map_id_;
  auto MapName = LoadMapName(map_id);
  auto m_name = MapName.map_name;
  auto s_name = MapName.street_name;

  Freetype::LoadSize(14);
  auto m_w = Freetype::LoadW(m_name) + 53;
  auto s_w = Freetype::LoadW(s_name) + 53;

  auto w = std::max(m_w, s_w);
  backgrnd_max_wh.x = std::max(w, 240.0f);
  backgrnd_min_wh.x = std::max(w, 240.0f);

  if (auto texture = LoadCanvasTexture()) {
    backgrnd_max_wh.y = std::clamp(texture->h, 180, 220);
    backgrnd_min_wh.y = std::clamp(texture->h, 180, 220);
    max_ = true;
    disable_ = false;
  } else {
    max_ = false;
    disable_ = true;
  }
}

}  // namespace sdlms
