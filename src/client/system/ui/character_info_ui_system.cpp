#include "character_info_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <ranges>

SDL_FPoint character_info_ui_system::load_wh() { return SDL_FPoint{277, 183}; }

void character_info_ui_system::render_backgrnd() {
  static auto backgrnd_top = wz_resource::load_texture(
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/backgrnd_top"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(backgrnd_top->w),
                     static_cast<float>(backgrnd_top->h)};
  SDL_RenderTexture(window::renderer, backgrnd_top, nullptr, &pos_rect);

  static auto backgrnd_bottom = wz_resource::load_texture(
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/backgrnd_bottom"));
  pos_rect = {pos.x, pos.y + backgrnd_top->h,
              static_cast<float>(backgrnd_bottom->w),
              static_cast<float>(backgrnd_bottom->h)};
  SDL_RenderTexture(window::renderer, backgrnd_bottom, nullptr, &pos_rect);
}

void character_info_ui_system::render_button() {
  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:party"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:trade"),
      wz_resource::ui->find(
          u"CharacterInfo.img/RemoteInfo/button:toggleItemlist"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:wishlist"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:upPop"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:downPop"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:showPet"),
      wz_resource::ui->find(u"Basic.img/BtClose"),
  };
  auto wh = load_wh();
  std::array buttons_rect = {
      SDL_FRect{117, 25, 80, 18},      //
      SDL_FRect{117, 45, 80, 18},      //
      SDL_FRect{210, 25, 60, 18},      //
      SDL_FRect{210, 45, 60, 18},      //
      SDL_FRect{240, 115, 12, 12},     //
      SDL_FRect{253, 115, 12, 12},     //
      SDL_FRect{188, 158, 81, 18},     //
      SDL_FRect{wh.x - 20, 8, 12, 12}, //
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
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render) {
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

void character_info_ui_system::render_character() {
  auto &camera = camera_game_instance::camera;
  character.pos.x = int(camera.x + pos.x + 57);
  character.pos.y = int(camera.y + pos.y + 122);
  character_render_system::render_character(character);
}

bool character_info_ui_system::render() {
  render_backgrnd();
  render_button();
  render_character();
  return true;
}

void character_info_ui_system::open() {
  auto wh = load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void character_info_ui_system::close() {
  std::erase(system::render_systems, render);
}

void character_info_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void character_info_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void character_info_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void character_info_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void character_info_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool character_info_ui_system::cursor_in() {
  bool r = false;
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, 277, 183};
  r = SDL_PointInRectFloat(&mouse, &pos_rect);
  if (r) {
    return r;
  }
  if (pet) {
  }
  return r;
}

bool character_info_ui_system::event_open_check(game_character &g_character) {
  auto &camera = camera_game_instance::camera;
  auto r = character_logic_system::load_rect(g_character);
  r.x -= camera.x;
  r.y -= camera.y;
  return SDL_PointInRectFloat(&window::mouse_pos, &r);
}

void character_info_ui_system::load_avatar(game_character &c, std::u16string &j,
                                           int32_t f) {

  character = c;
  character.flip = 0;
  character_job = j;
  character_fame = f;

  character_logic_system::run_stand_action(character);
  character_logic_system::run_face_action(character, u"default");
}

bool character_info_ui_system::event_open(SDL_Event *event) {
  bool r = false;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT && event->button.clicks == 2) {
      if (cursor_game_instance::cursor_ui != nullptr) {
        break;
      }
      // 双击
      if (event_open_check(character_game_instance::self)) {
        r = true;
        load_avatar(character_game_instance::self,
                    job_skill_game_instance::self_job, 0);
        break;
      }
      for (auto &other_data :
           character_game_instance::others | std::views::values) {
        if (event_open_check(other_data.g_character)) {
          r = false;
          break;
        }
      }
    }
    break;
  }
  default: {
    break;
  }
  }
  if (r) {
    auto fn = &render;
    if (std::ranges::contains(system::render_systems, fn)) {
      close();
    }
    open();
  }
  return r;
}

void character_info_ui_system::event_button_party() {}

void character_info_ui_system::event_button_trade() {}

void character_info_ui_system::event_button_toggleitem() {}

void character_info_ui_system::event_button_wishlist() {}

void character_info_ui_system::event_button_up_fame() {}

void character_info_ui_system::event_button_dowm_fame() {}

void character_info_ui_system::event_button_show_pet() {}

void character_info_ui_system::event_close() { close(); }

bool character_info_ui_system::event_button(SDL_Event *event) {
  std::vector<void (*)()> fns;
  auto wh = load_wh();
  std::array buttons_rect = {
      SDL_FRect{117, 25, 80, 18},      //
      SDL_FRect{117, 45, 80, 18},      //
      SDL_FRect{210, 25, 60, 18},      //
      SDL_FRect{210, 45, 60, 18},      //
      SDL_FRect{240, 115, 12, 12},     //
      SDL_FRect{253, 115, 12, 12},     //
      SDL_FRect{188, 158, 81, 18},     //
      SDL_FRect{wh.x - 20, 8, 12, 12}, //
  };
  fns = {
      event_button_party,    event_button_trade,   event_button_toggleitem,
      event_button_wishlist, event_button_up_fame, event_button_dowm_fame,
      event_button_show_pet, event_close,
  };

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

bool character_info_ui_system::event(SDL_Event *event) {
  if (event_open(event)) {
    return false;
  }
  auto fn = &render;
  if (!std::ranges::contains(system::render_systems, fn)) {
    return true;
  }
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
        r = !event_button(event);
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