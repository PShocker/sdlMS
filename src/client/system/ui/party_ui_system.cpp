#include "party_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/party_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <algorithm>

void party_ui_system::render_backgrnd(float x, float y, float w, float h) {
  // backgrnd
  static auto backgrnd_node =
      wz_resource::ui->find(u"UIToolTip.img/Skill/Frame");

  auto texture_c = wz_resource::load_texture(backgrnd_node->get_child(u"c"));
  auto texture_e =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"e")));
  auto texture_ne =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"ne")));
  auto texture_n =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"n")));
  auto texture_nw =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"nw")));
  auto texture_w =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"w")));
  auto texture_sw =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"sw")));
  auto texture_s =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"s")));
  auto texture_se =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"se")));

  SDL_FRect rect;
  rect.x = x;
  rect.y = y;

  // 左上
  SDL_FPoint p_nw{x, y};
  rect.x = (p_nw.x);
  rect.y = (p_nw.y);
  rect.w = texture_nw->w;
  rect.h = texture_nw->h;
  SDL_RenderTexture(window::renderer, texture_nw, nullptr, &rect);

  // 左下
  SDL_FPoint p_sw{
      x,
      y + h - texture_sw->h,
  };
  rect.x = (p_sw.x);
  rect.y = (p_sw.y);
  rect.w = texture_sw->w;
  rect.h = texture_sw->h;
  SDL_RenderTexture(window::renderer, texture_sw, nullptr, &rect);

  // 右上
  SDL_FPoint p_ne{
      x + w - texture_ne->w,
      y,
  };
  rect.x = (p_ne.x);
  rect.y = (p_ne.y);
  rect.w = texture_ne->w;
  rect.h = texture_ne->h;
  SDL_RenderTexture(window::renderer, texture_ne, nullptr, &rect);

  // 右下
  SDL_FPoint p_se{
      x + w - texture_ne->w,
      y + h - texture_sw->h,
  };
  rect.x = (p_se.x);
  rect.y = (p_se.y);
  rect.w = texture_se->w;
  rect.h = texture_se->h;
  SDL_RenderTexture(window::renderer, texture_se, nullptr, &rect);

  rect.x = (x + texture_nw->w);
  rect.y = (y + texture_nw->h);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = p_sw.y - (p_nw.y + texture_nw->h);
  SDL_RenderTextureTiled(window::renderer, texture_c, nullptr, 1, &rect);

  // 竖着的两边
  rect.x = (p_nw.x);
  rect.y = (p_nw.y + texture_nw->h);
  rect.w = texture_w->w;
  rect.h = p_sw.y - (p_nw.y + texture_nw->h);
  SDL_RenderTextureTiled(window::renderer, texture_w, nullptr, 1, &rect);

  rect.x = (p_se.x);
  rect.y = (p_ne.y + texture_ne->h);
  rect.w = texture_e->w;
  rect.h = p_se.y - (p_ne.y + texture_ne->h);
  SDL_RenderTextureTiled(window::renderer, texture_e, nullptr, 1, &rect);

  // 横着的两边
  rect.x = (p_nw.x + texture_nw->w);
  rect.y = (p_ne.y);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = texture_n->h;
  SDL_RenderTextureTiled(window::renderer, texture_n, nullptr, 1, &rect);

  rect.x = (p_nw.x + texture_nw->w);
  rect.y = (p_sw.y);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = texture_s->h;
  SDL_RenderTextureTiled(window::renderer, texture_s, nullptr, 1, &rect);
}

void party_ui_system::render_party_hp() {
  for (int i = 0; i < party_game_instance::data->clients.size(); i++) {
    const auto &player = party_game_instance::data->clients[i];
    static auto texture = wz_resource::load_texture(wz_resource::ui->find(u""));
  }
}

bool party_ui_system::render() {
  auto [w, h] = load_wh();
  render_backgrnd(0, 0, w, h);
  return true;
}

SDL_FPoint party_ui_system::load_wh() { return {209, 289}; }

void party_ui_system::open() {
  auto it = std::ranges::find(system::render_systems, &render);
  if (it != system::render_systems.end()) {
    return;
  }
  it = std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void party_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void party_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void party_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void party_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void party_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void party_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool party_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool party_ui_system::event(SDL_Event *event) {
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