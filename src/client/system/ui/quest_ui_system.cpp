#include "quest_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>

void quest_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd"));
  static auto backgrnd2 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd2"));
  static auto backgrnd3 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd3"));
  static auto backgrnd4 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd4"));
  static auto backgrnd5 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd5"));
  static auto notice0 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/notice0"));
  static auto notice1 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/notice1"));
  static auto notice2 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/notice2"));
  static auto notice3 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/notice3"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
}

void quest_ui_system::render_button() {
  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"Basic.img/BtClose"),
  };
  auto wh = load_wh();
  const std::array buttons_rect = {
      SDL_FRect{wh.x - 18, 6, 12, 12}, //
  };

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        !cursor_game_instance::modal_overlay) {
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
}

void quest_ui_system::render_quests() {
  int i = 1;
  while (i <= 15) {
    switch (active_tab) {
    case 0: {
      auto quests = quest_game_instance::load_avaliable_quest(i);
      break;
    }
    }
    i++;
  }
}

static const SDL_FPoint detail_lt = {245, 0};

void quest_ui_system::render_quest_detail() {
  if (!detail) {
    return;
  }
  static auto backgrnd2 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd2"));
  SDL_FRect pos_rect{
      pos.x + detail_lt.x,
      pos.y,
      static_cast<float>(backgrnd2->w),
      static_cast<float>(backgrnd2->h),
  };
  SDL_RenderTexture(window::renderer, backgrnd2, nullptr, &pos_rect);
  // name
  freetype::load_size(12);
  freetype::load_bold(true);
  auto quest_name =
      wz_resource::quest->find(u"QuestData/" + quest + u"/QuestInfo");

  freetype::load_bold(false);
}

void quest_ui_system::render_tab() {
  const static std::array tab_pos = {
      SDL_FPoint{6, 24},   //
      SDL_FPoint{78, 24},  //
      SDL_FPoint{150, 24}, //
  };
  const static auto tab_node =
      wz_resource::ui->find(u"Quest.img/Quest/list/Tab");
  const static std::array active_texture = {
      wz_resource::load_texture(tab_node->find(u"enabled/0")),
      wz_resource::load_texture(tab_node->find(u"enabled/1")),
      wz_resource::load_texture(tab_node->find(u"enabled/2")),
  };
  const static std::array disabled_texture = {
      wz_resource::load_texture(tab_node->find(u"disabled/0")),
      wz_resource::load_texture(tab_node->find(u"disabled/1")),
      wz_resource::load_texture(tab_node->find(u"disabled/2")),
  };
  for (uint8_t i = 0; i < tab_pos.size(); i++) {
    SDL_Texture *t = active_tab == i ? active_texture[i] : disabled_texture[i];
    SDL_FRect pos_rect{
        static_cast<float>(int(pos.x + tab_pos[i].x)),
        static_cast<float>(int(pos.y + tab_pos[i].y)),
        static_cast<float>(t->w),
        static_cast<float>(t->h),
    };
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  }
}

bool quest_ui_system::render() {
  render_backgrnd();
  render_tab();
  render_button();
  render_quest_detail();
  render_quests();
  return true;
}

SDL_FPoint quest_ui_system::load_wh() { return {247, 398}; }

void quest_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    detail = true;

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void quest_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void quest_ui_system::event_tab(SDL_Event *event) {
  const static std::array tab_rect = {
      SDL_FRect{6, 24, 72, 19},   //
      SDL_FRect{78, 24, 72, 19},  //
      SDL_FRect{150, 24, 72, 19}, //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      if (active_tab == i) {
        return;
      }
      quest = u"";
      return;
    }
  }
}

void quest_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void quest_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void quest_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void quest_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void quest_ui_system::toggle() {
  audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool quest_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool quest_ui_system::event(SDL_Event *event) {
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
        event_tab(event);
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