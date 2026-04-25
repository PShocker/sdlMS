#include "character_stat_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/system.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>

std::vector<SDL_FPoint> character_stat_ui_system::load_wh() {
  if (detail) {
    return {{185, 281}, {187, 223}};
  } else {
    return {{185, 281}};
  }
}

void character_stat_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"CharacterStat.img/Main/backgrnd"));
}

bool character_stat_ui_system::render() { return true; }

void character_stat_ui_system::open() {
  SDL_FPoint wh = {0, 0};
  auto whs = load_wh();
  for (auto w : whs) {
    wh.x += w.x;
    wh.y = std::max(w.y, wh.y);
  }
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void character_stat_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  event_drag_end();
}

bool character_stat_ui_system::event(SDL_Event *event) { return false; }

void character_stat_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void character_stat_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh()[0];
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 18};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void character_stat_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void character_stat_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh()[0];
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}