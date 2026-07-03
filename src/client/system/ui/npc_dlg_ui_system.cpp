#include "npc_dlg_ui_system.h"
#include "src/client/game/game_npc.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>

void npc_dlg_ui_system::render_backgrnd() {
  auto [w, h] = load_wh();
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/t"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  static auto c = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/c"));
  static auto s = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/s"));
  pos_rect = {
      pos.x,
      pos.y + t->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t->h - s->h),
  };
  SDL_RenderTextureTiled(window::renderer, c, nullptr, 1, &pos_rect);

  pos_rect = {
      pos.x,
      pos.y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_RenderTexture(window::renderer, s, nullptr, &pos_rect);
}

void npc_dlg_ui_system::render_npc() {
  auto [w, h] = load_wh();
  game_npc npc;
  npc.action = u"stand";
  const auto &camera = camera_game_instance::camera;
  npc.pos.x = camera.x + pos.x + 80;
  npc.pos.y = camera.y + pos.y + 15 + h / 2;
  npc.ani_index = 0;
  npc.id = npc_id;
  npc_render_system::render_npc(npc);

  //   render bar
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/bar"));
  SDL_FRect pos_rect{
      pos.x + 80 - t->w / 2,
      pos.y + 20 + h / 2,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
}

bool npc_dlg_ui_system::render() {
  render_backgrnd();
  render_npc();
  return true;
}

SDL_FPoint npc_dlg_ui_system::load_wh() { return {529, 300}; }

void npc_dlg_ui_system::open() {
  auto wh = load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.begin(), event);
}

void npc_dlg_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void npc_dlg_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void npc_dlg_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.begin(), event);
}

void npc_dlg_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void npc_dlg_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void npc_dlg_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

bool npc_dlg_ui_system::event(SDL_Event *event) {
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