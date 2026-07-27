#include "worldmap_ui_system.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "minimap_ui_system.h"
#include "notice_ui_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "tooltip_ui_system.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>

void worldmap_ui_system::render_backgrnd() {
  auto wh = load_wh();
  auto w = wh.x;
  auto h = wh.y;
  static auto bord_node = wz_resource::ui->find(u"WorldMap.img/Border");
  static auto lt = wz_resource::load_texture(bord_node->get_child(u"0"));
  static auto t = wz_resource::load_texture(bord_node->get_child(u"1"));
  static auto rt = wz_resource::load_texture(bord_node->get_child(u"2"));
  static auto lm = wz_resource::load_texture(bord_node->get_child(u"3"));
  static auto rm = wz_resource::load_texture(bord_node->get_child(u"4"));
  static auto lb = wz_resource::load_texture(bord_node->get_child(u"5"));
  static auto b = wz_resource::load_texture(bord_node->get_child(u"6"));
  static auto rb = wz_resource::load_texture(bord_node->get_child(u"7"));
  static auto title =
      wz_resource::load_texture(wz_resource::ui->find(u"WorldMap.img/title"));

  SDL_FRect pos_rect = {pos.x, pos.y, (float)lt->w, (float)lt->h};
  SDL_RenderTexture(window::renderer, lt, nullptr, &pos_rect);
  pos_rect = {pos.x + 7, pos.y, static_cast<float>(w - 14), (float)t->h};
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  pos_rect = {pos.x + w - 7, pos.y, static_cast<float>(rt->w), (float)t->h};
  SDL_RenderTexture(window::renderer, rt, nullptr, &pos_rect);
  pos_rect = {pos.x, pos.y + 32, static_cast<float>(lm->w),
              (float)h - b->h - 32};
  SDL_RenderTexture(window::renderer, lm, nullptr, &pos_rect);
  pos_rect = {pos.x + w - 7, pos.y + 32, static_cast<float>(rm->w),
              (float)h - b->h - 32};
  SDL_RenderTexture(window::renderer, rm, nullptr, &pos_rect);
  pos_rect = {pos.x, pos.y + h - b->h, static_cast<float>(lb->w), (float)lb->h};
  SDL_RenderTexture(window::renderer, lb, nullptr, &pos_rect);
  pos_rect = {pos.x + 7, pos.y + h - b->h, static_cast<float>(w - 14),
              (float)b->h};
  SDL_RenderTexture(window::renderer, b, nullptr, &pos_rect);
  pos_rect = {pos.x + w - 7, pos.y + h - b->h, static_cast<float>(rb->w),
              (float)rb->h};
  SDL_RenderTexture(window::renderer, rb, nullptr, &pos_rect);
  pos_rect = {static_cast<float>((int32_t)pos.x + 10),
              static_cast<float>((int32_t)pos.y + 10),
              static_cast<float>(title->w), (float)title->h};
  SDL_RenderTexture(window::renderer, title, nullptr, &pos_rect);
}

static const SDL_FPoint map_offset = {6, 30};

static uint32_t spot_info_id;

void worldmap_ui_system::render_spot() {
  struct worldmap_spot {
    SDL_FPoint pos;
    uint8_t type;
    std::flat_set<uint32_t> map_id;
  };
  spot_info_id = 0;
  static std::flat_map<std::u16string, std::vector<worldmap_spot>> spot_cache;
  if (!spot_cache.contains(path)) {
    auto spot_node = wz_resource::map->find(u"WorldMap/" + path + u"/MapList");
    for (auto [k, v] : *spot_node->get_children()) {
      worldmap_spot wsp;
      auto vec =
          static_cast<wz::Property<wz::WzVec2D> *>(v[0]->get_child(u"spot"))
              ->get();
      wsp.pos = {static_cast<float>(vec.x), static_cast<float>(vec.y)};
      wsp.type =
          static_cast<wz::Property<int> *>(v[0]->get_child("type"))->get();
      for (auto [m, mid] : *v[0]->get_child(u"mapNo")) {
        auto map_id = static_cast<wz::Property<int> *>(mid[0])->get();
        wsp.map_id.insert(map_id);
      }
      spot_cache[path].push_back(wsp);
    }
  }
  auto spots = spot_cache.at(path);
  static auto texture_node = wz_resource::map->find("MapHelper.img/worldMap");
  static std::array map_img_array = {
      wz_resource::load_texture(texture_node->find(u"mapImage/0")),
      wz_resource::load_texture(texture_node->find(u"mapImage/1")),
      wz_resource::load_texture(texture_node->find(u"mapImage/2")),
  };

  auto wh = load_wh();
  auto map_ox = (wh.x - 12) / 2;
  auto map_oy = (wh.y - 44) / 2;

  auto &mouse_pos = window::mouse_pos;
  std::optional<SDL_FPoint> spot_point;
  for (auto &spot : spots) {
    auto texture = map_img_array[spot.type];
    SDL_FRect pos_rect = {
        static_cast<float>(int(pos.x + map_ox + spot.pos.x + map_offset.x -
                               (float)texture->w / 2)),
        static_cast<float>(int(pos.y + map_oy + spot.pos.y + map_offset.y -
                               (float)texture->h / 2)),
        static_cast<float>(texture->w), (float)texture->h};
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
    pos_rect.x -= 5;
    pos_rect.y -= 5;
    pos_rect.w += 5;
    pos_rect.h += 5;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      spot_info_id = *spot.map_id.begin();
    }
    if (spot.map_id.contains(scene_system_instance::map_id)) {
      spot_point = {
          pos_rect.x + 5 + (float)texture->w / 2,
          pos_rect.y + 5,
      };
    }
  }
  if (spot_point.has_value()) {
    render_cur_pos(spot_point.value());
  }
  if (cursor_game_instance::modal_overlay == nullptr &&
      cursor_game_instance::cursor_ui == render && spot_info_id != 0) {
    render_spot_info(spot_info_id, mouse_pos.x, mouse_pos.y);
  }
}

void worldmap_ui_system::render_map() {
  auto back_node = wz_resource::map->find(u"WorldMap/" + path + u"/BaseImg/0");
  auto texture = wz_resource::load_texture(back_node);
  SDL_FRect pos_rect = {pos.x + map_offset.x, pos.y + map_offset.y,
                        static_cast<float>(texture->w), (float)texture->h};
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

void worldmap_ui_system::render_cur_pos(SDL_FPoint point) {
  auto now = window::dt_now;
  static auto point_node =
      wz_resource::map->find(u"MapHelper.img/worldMap/curPos");
  auto sum = point_node->children_count() * 200;
  auto offset = now % sum; // 取余，得到周期内偏移
  auto i = offset / 200;
  auto index = std::to_string(i);
  auto node = point_node->get_child(index);
  auto origin = wz_resource::load_fpoint(node->get_child(u"origin"));
  auto texture = wz_resource::load_texture(node);
  SDL_FRect pos_rect{
      point.x - origin.x,
      point.y - origin.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

void worldmap_ui_system::render_spot_info(uint32_t id, float x, float y) {
  tooltip_ui_system::render_world_map_info(id, x, y);
}

void worldmap_ui_system::render_button() {
  // 构建按钮列表
  std::vector<wz::Node *> nodes = {
      wz_resource::ui->find(u"Basic.img/BtClose"),
  };
  auto [w, h] = load_wh();
  std::vector<SDL_FRect> rects = {
      {w - 18, 6, 12, 12}, //
  };
  // 渲染所有按钮
  bool mouse_down = window::mouse_state & SDL_BUTTON_LMASK;
  bool cursor_on_ui = cursor_game_instance::cursor_ui == render;
  bool modal_blocked = cursor_game_instance::modal_overlay;

  for (size_t i = 0; i < nodes.size(); ++i) {
    SDL_FRect rect = {
        rects[i].x + (int)pos.x,
        rects[i].y + (int)pos.y,
        rects[i].w,
        rects[i].h,
    };

    std::u16string state = u"normal";
    if (cursor_on_ui && !modal_blocked &&
        SDL_PointInRectFloat(&window::mouse_pos, &rect)) {
      state = mouse_down ? u"pressed" : u"mouseOver";
    }
    auto texture = wz_resource::load_texture(nodes[i]->find((state + u"/0")));
    SDL_RenderTexture(window::renderer, texture, nullptr, &rect);
  }
}

bool worldmap_ui_system::render() {
  render_backgrnd();
  render_map();
  render_spot();
  render_button();
  return true;
}

SDL_FPoint worldmap_ui_system::load_wh() {
  static std::flat_map<std::u16string, SDL_FPoint> whs;
  if (!whs.contains(path)) {
    auto back_node =
        wz_resource::map->find(u"WorldMap/" + path + u"/BaseImg/0");
    auto texture = wz_resource::load_texture(back_node);
    whs[path] = SDL_FPoint{static_cast<float>(texture->w + 12),
                           static_cast<float>(texture->h + 44)};
  }
  return whs.at(path);
}

void worldmap_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    if (minimap_ui_system::disable) {
      notice_ui_system::type = notice_ui_system::notice_enum::worldmap_disable;
      notice_ui_system::close();
      notice_ui_system::open();
      return;
    }
    worldmap_ui_system::path = u"WorldMap000.img";

    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void worldmap_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  event_drag_end();
}

void worldmap_ui_system::event_close() { close(); }

void worldmap_ui_system::event_click_spot() {
  if (spot_info_id != 0) {
    notice_ui_system::type = notice_ui_system::notice_enum::worldmap_teleport;
    notice_ui_system::data = spot_info_id;
    notice_ui_system::close();
    notice_ui_system::open();
  }
}

bool worldmap_ui_system::event_button(SDL_Event *event) {
  auto [w, h] = load_wh();
  std::vector<SDL_FRect> buttons_rect = {
      {w - 18, 6, 12, 12}, //
  };
  const static std::array buttons_func = {
      event_close,
  };

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      buttons_func[i]();
      return true;
    }
  }
  return false;
}

bool worldmap_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      event_close();
      return false;
      break;
    }
    default: {
      break;
    }
    }
    break;
  }
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
      r = !event_button(event);
      event_drag_end();
      event_click_spot();
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

bool worldmap_ui_system::cursor_in() {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, wh.y};
  return SDL_PointInRectFloat(&window::mouse_pos, &pos_rect);
}

void worldmap_ui_system::toggle() {
  audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void worldmap_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void worldmap_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void worldmap_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void worldmap_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}