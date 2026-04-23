#include "worldmap_ui_system.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
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

static SDL_FPoint map_offset = {6, 30};

void worldmap_ui_system::render_spot() {
  struct worldmap_spot {
    SDL_FPoint pos;
    uint8_t type;
    std::flat_set<uint32_t> map_id;
  };
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

  for (auto &spot : spots) {
    auto texture = map_img_array[spot.type];
    SDL_FRect pos_rect = {
        static_cast<float>(int(pos.x + map_ox + spot.pos.x + map_offset.x -
                               (float)texture->w / 2)),
        static_cast<float>(int(pos.y + map_oy + spot.pos.y + map_offset.y -
                               (float)texture->h / 2)),
        static_cast<float>(texture->w), (float)texture->h};
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
}

void worldmap_ui_system::render_map() {
  auto back_node = wz_resource::map->find(u"WorldMap/" + path + u"/BaseImg/0");
  auto texture = wz_resource::load_texture(back_node);
  SDL_FRect pos_rect = {pos.x + map_offset.x, pos.y + map_offset.y,
                        static_cast<float>(texture->w), (float)texture->h};
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

bool worldmap_ui_system::render() {
  render_backgrnd();
  render_map();
  render_spot();
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
  worldmap_ui_system::path = u"WorldMap001.img";

  auto wh = worldmap_ui_system::load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void worldmap_ui_system::close() {
  worldmap_ui_system::path.clear();
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

bool worldmap_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        event_top();
        event_drag(event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      event_drag(event);
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_drag(event);
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
  auto fn = &worldmap_ui_system::render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void worldmap_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void worldmap_ui_system::event_drag(SDL_Event *event) {
  static std::optional<SDL_FPoint> drag;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      auto wh = load_wh();
      SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 14};
      if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
        drag = {pos.x - event->button.x, pos.y - event->button.y};
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      drag = std::nullopt;
    }
  }
  case SDL_EVENT_MOUSE_MOTION: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    }
  }
  }
  return;
}