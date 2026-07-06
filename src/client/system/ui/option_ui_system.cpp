#include "option_ui_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

void option_ui_system::render_backgrnd() {
  static auto texture = wz_resource::load_texture(
      wz_resource::ui->find(u"Options.img/OptionMenu/backgrnd"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

void option_ui_system::render_option() {
  switch (active_tab) {
  case 0: {
    static auto graphic = wz_resource::load_texture(wz_resource::ui->find(
        u"Options.img/OptionMenu/graphic/layer:backgrnd"));
    SDL_FRect pos_rect{
        pos.x + 9,
        pos.y + 51,
        static_cast<float>(graphic->w),
        static_cast<float>(graphic->h),
    };
    SDL_RenderTexture(window::renderer, graphic, nullptr, &pos_rect);
    break;
  }
  }
}

void option_ui_system::render_button() {
  const static std::array buttons_node = {
      wz_resource::ui->find(u"KeyBindings.img/quickslot/button:confirm"),
      wz_resource::ui->find(u"ChannelChange.img/Channel/BtCancel"),
      wz_resource::ui->find(u"Basic.img/BtClose"),
  };
  const std::array buttons_rect = {
      SDL_FRect{176, 407, 53, 18}, //
      SDL_FRect{232, 407, 47, 18}, //
      SDL_FRect{266, 7, 12, 12},   //
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
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

void option_ui_system::render_tab() {
  const static std::array tab_pos = {
      SDL_FPoint{6, 23},   //
      SDL_FPoint{65, 23},  //
      SDL_FPoint{124, 23}, //
      SDL_FPoint{183, 23}, //
  };
  const static auto tab_node =
      wz_resource::ui->find(u"Options.img/OptionMenu/tab");
  const static std::array active_texture = {
      wz_resource::load_texture(tab_node->find(u"enabled/0")),
      wz_resource::load_texture(tab_node->find(u"enabled/1")),
      wz_resource::load_texture(tab_node->find(u"enabled/2")),
      wz_resource::load_texture(tab_node->find(u"enabled/3")),
  };
  const static std::array disabled_texture = {
      wz_resource::load_texture(tab_node->find(u"disabled/0")),
      wz_resource::load_texture(tab_node->find(u"disabled/1")),
      wz_resource::load_texture(tab_node->find(u"disabled/2")),
      wz_resource::load_texture(tab_node->find(u"disabled/3")),
  };
  for (uint8_t i = 0; i < tab_pos.size(); i++) {
    SDL_Texture *t = active_tab == i ? active_texture[i] : disabled_texture[i];
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos[i].x)),
                       static_cast<float>(int(pos.y + tab_pos[i].y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  }
}

bool option_ui_system::render() {
  render_backgrnd();
  render_button();
  render_tab();
  render_option();
  return true;
}

SDL_FPoint option_ui_system::load_wh() { return {287, 432}; }

void option_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void option_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void option_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void option_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void option_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void option_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void option_ui_system::event_button_ok() { close(); }

bool option_ui_system::event_button(SDL_Event *event) {
  const static std::array buttons_rect = {
      SDL_FRect{119, 115, 47, 18}, //
  };
  const static std::array buttons_func = {
      event_button_ok,
  };

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      buttons_func[i]();
      audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }
  return false;
}

bool option_ui_system::event(SDL_Event *event) {
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

void option_ui_system::toggle() {
  audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}
