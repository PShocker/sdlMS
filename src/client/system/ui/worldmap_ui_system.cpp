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

void WorldmapUiSystem::render_backgrnd() {
  auto wh = load_wh();
  auto w = wh.x;
  auto h = wh.y;
  static auto bord_node = WzResource::ui->find(u"WorldMap.img/Border");
  static auto lt = WzResource::load_texture(bord_node->get_child(u"0"));
  static auto t = WzResource::load_texture(bord_node->get_child(u"1"));
  static auto rt = WzResource::load_texture(bord_node->get_child(u"2"));
  static auto lm = WzResource::load_texture(bord_node->get_child(u"3"));
  static auto rm = WzResource::load_texture(bord_node->get_child(u"4"));
  static auto lb = WzResource::load_texture(bord_node->get_child(u"5"));
  static auto b = WzResource::load_texture(bord_node->get_child(u"6"));
  static auto rb = WzResource::load_texture(bord_node->get_child(u"7"));
  static auto title =
      WzResource::load_texture(WzResource::ui->find(u"WorldMap.img/title"));

  SDL_FRect pos_rect = {pos.x, pos.y, (float)lt->w, (float)lt->h};
  SDL_RenderTexture(Window::renderer, lt, nullptr, &pos_rect);
  pos_rect = {pos.x + 7, pos.y, static_cast<float>(w - 14), (float)t->h};
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  pos_rect = {pos.x + w - 7, pos.y, static_cast<float>(rt->w), (float)t->h};
  SDL_RenderTexture(Window::renderer, rt, nullptr, &pos_rect);
  pos_rect = {pos.x, pos.y + 32, static_cast<float>(lm->w),
              (float)h - b->h - 32};
  SDL_RenderTexture(Window::renderer, lm, nullptr, &pos_rect);
  pos_rect = {pos.x + w - 7, pos.y + 32, static_cast<float>(rm->w),
              (float)h - b->h - 32};
  SDL_RenderTexture(Window::renderer, rm, nullptr, &pos_rect);
  pos_rect = {pos.x, pos.y + h - b->h, static_cast<float>(lb->w), (float)lb->h};
  SDL_RenderTexture(Window::renderer, lb, nullptr, &pos_rect);
  pos_rect = {pos.x + 7, pos.y + h - b->h, static_cast<float>(w - 14),
              (float)b->h};
  SDL_RenderTexture(Window::renderer, b, nullptr, &pos_rect);
  pos_rect = {pos.x + w - 7, pos.y + h - b->h, static_cast<float>(rb->w),
              (float)rb->h};
  SDL_RenderTexture(Window::renderer, rb, nullptr, &pos_rect);
  pos_rect = {static_cast<float>((int32_t)pos.x + 10),
              static_cast<float>((int32_t)pos.y + 10),
              static_cast<float>(title->w), (float)title->h};
  SDL_RenderTexture(Window::renderer, title, nullptr, &pos_rect);
}

static const SDL_FPoint map_offset = {6, 30};

static uint32_t spot_info_id;

void WorldmapUiSystem::render_spot() {
  struct WorldmapSpot {
    SDL_FPoint pos;
    uint8_t type;
    std::flat_set<uint32_t> map_id;
  };
  spot_info_id = 0;
  static std::flat_map<std::u16string, std::vector<WorldmapSpot>> spot_cache;
  if (!spot_cache.contains(path)) {
    auto spot_node = WzResource::map->find(u"WorldMap/" + path + u"/MapList");
    for (auto [k, v] : *spot_node->get_children()) {
      WorldmapSpot wsp;
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
  static auto texture_node = WzResource::map->find("MapHelper.img/worldMap");
  static std::array map_img_array = {
      WzResource::load_texture(texture_node->find(u"mapImage/0")),
      WzResource::load_texture(texture_node->find(u"mapImage/1")),
      WzResource::load_texture(texture_node->find(u"mapImage/2")),
  };

  auto wh = load_wh();
  auto map_ox = (wh.x - 12) / 2;
  auto map_oy = (wh.y - 44) / 2;

  auto &mouse_pos = Window::mouse_pos;
  std::optional<SDL_FPoint> spot_point;
  for (auto &spot : spots) {
    auto texture = map_img_array[spot.type];
    SDL_FRect pos_rect = {
        static_cast<float>(int(pos.x + map_ox + spot.pos.x + map_offset.x -
                               (float)texture->w / 2)),
        static_cast<float>(int(pos.y + map_oy + spot.pos.y + map_offset.y -
                               (float)texture->h / 2)),
        static_cast<float>(texture->w), (float)texture->h};
    SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
    pos_rect.x -= 5;
    pos_rect.y -= 5;
    pos_rect.w += 5;
    pos_rect.h += 5;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      spot_info_id = *spot.map_id.begin();
    }
    if (spot.map_id.contains(SceneSystemInstance::map_id)) {
      spot_point = {
          pos_rect.x + 5 + (float)texture->w / 2,
          pos_rect.y + 5,
      };
    }
  }
  if (spot_point.has_value()) {
    render_cur_pos(spot_point.value());
  }
}

void WorldmapUiSystem::render_map() {
  auto back_node = WzResource::map->find(u"WorldMap/" + path + u"/BaseImg/0");
  auto texture = WzResource::load_texture(back_node);
  SDL_FRect pos_rect = {pos.x + map_offset.x, pos.y + map_offset.y,
                        static_cast<float>(texture->w), (float)texture->h};
  SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
}

void WorldmapUiSystem::render_cur_pos(SDL_FPoint point) {
  auto now = Window::dt_now;
  static auto point_node =
      WzResource::map->find(u"MapHelper.img/worldMap/curPos");
  auto sum = point_node->children_count() * 200;
  auto offset = now % sum; // 取余，得到周期内偏移
  auto i = offset / 200;
  auto index = std::to_string(i);
  auto node = point_node->get_child(index);
  auto origin = WzResource::load_fpoint(node->get_child(u"origin"));
  auto texture = WzResource::load_texture(node);
  SDL_FRect pos_rect{
      point.x - origin.x,
      point.y - origin.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
}

bool WorldmapUiSystem::render_spot_info() {
  if (CursorGameInstance::modal_overlay == nullptr &&
      CursorGameInstance::cursor_ui == render && spot_info_id != 0) {
    const auto &mouse_pos = Window::mouse_pos;
    TooltipUiSystem::render_world_map_info(spot_info_id, mouse_pos.x,
                                             mouse_pos.y);
  }
  return true;
}

void WorldmapUiSystem::render_button() {
  // 构建按钮列表
  std::vector<wz::Node *> nodes = {
      WzResource::ui->find(u"Basic.img/BtClose"),
  };
  auto [w, h] = load_wh();
  std::vector<SDL_FRect> rects = {
      {w - 18, 6, 12, 12}, //
  };
  // 渲染所有按钮
  bool mouse_down = Window::mouse_state & SDL_BUTTON_LMASK;
  bool cursor_on_ui = CursorGameInstance::cursor_ui == render;
  bool modal_blocked = CursorGameInstance::modal_overlay;

  for (size_t i = 0; i < nodes.size(); ++i) {
    SDL_FRect rect = {
        rects[i].x + (int)pos.x,
        rects[i].y + (int)pos.y,
        rects[i].w,
        rects[i].h,
    };

    std::u16string state = u"normal";
    if (cursor_on_ui && !modal_blocked &&
        SDL_PointInRectFloat(&Window::mouse_pos, &rect)) {
      state = mouse_down ? u"pressed" : u"mouseOver";
    }
    auto texture = WzResource::load_texture(nodes[i]->find((state + u"/0")));
    SDL_RenderTexture(Window::renderer, texture, nullptr, &rect);
  }
}

bool WorldmapUiSystem::render() {
  render_backgrnd();
  render_map();
  render_spot();
  render_button();
  return true;
}

SDL_FPoint WorldmapUiSystem::load_wh() {
  static std::flat_map<std::u16string, SDL_FPoint> whs;
  if (!whs.contains(path)) {
    auto back_node =
        WzResource::map->find(u"WorldMap/" + path + u"/BaseImg/0");
    auto texture = WzResource::load_texture(back_node);
    whs[path] = SDL_FPoint{static_cast<float>(texture->w + 12),
                           static_cast<float>(texture->h + 44)};
  }
  return whs.at(path);
}

void WorldmapUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    if (MinimapUiSystem::disable) {
      NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::worldmap_disable;
      NoticeUiSystem::close();
      NoticeUiSystem::open();
      return;
    }
    WorldmapUiSystem::path = u"WorldMap000.img";

    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void WorldmapUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::render_systems, render_spot_info);
  std::erase(System::event_systems, event);

  event_drag_end();
}

void WorldmapUiSystem::event_close() { close(); }

void WorldmapUiSystem::event_click_spot() {
  if (spot_info_id != 0) {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::worldmap_teleport;
    NoticeUiSystem::data = spot_info_id;
    NoticeUiSystem::close();
    NoticeUiSystem::open();
  }
}

bool WorldmapUiSystem::event_button(SDL_Event *event) {
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
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
      buttons_func[i]();
      return true;
    }
  }
  return false;
}

void WorldmapUiSystem::event_motion(SDL_Event *event) {
  auto &sys = System::render_systems;
  std::erase(sys, render_spot_info);
  auto it = std::ranges::find(sys, &CursorRenderSystem::render);
  if (it != sys.end()) {
    sys.insert(it, render_spot_info);
  }
}

bool WorldmapUiSystem::event(SDL_Event *event) {
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
      r = !event_button(event);
      event_drag_end();
      event_click_spot();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_motion(event);
    event_drag_move(event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

bool WorldmapUiSystem::cursor_in() {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, wh.y};
  return SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect);
}

void WorldmapUiSystem::toggle() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void WorldmapUiSystem::event_top() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void WorldmapUiSystem::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void WorldmapUiSystem::event_drag_end() {
  drag = std::nullopt;
  return;
}

void WorldmapUiSystem::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}