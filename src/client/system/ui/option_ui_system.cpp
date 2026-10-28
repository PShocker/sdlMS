#include "option_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "slider_ui_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/combo_box_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include <string>
#include <vector>

enum option_type {
  graphic,
  sound,
  game,
  social,
  controller,
};

void option_ui_system::render_backgrnd() {
  static auto texture =
      wz_resource::load_texture(wz_resource::ui->find(u"Options.img/backgrnd"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

void option_ui_system::render_option() {
  std::vector<wz::Node *> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  auto top = cursor_game_instance::cursor_ui == render;

  switch (active_tab) {
  case graphic: {
    static auto backgrnd = wz_resource::load_texture(
        wz_resource::ui->find(u"Options.img/graphic/layer:backgrnd"));
    SDL_FRect pos_rect{
        pos.x + 9,
        pos.y + 53,
        static_cast<float>(backgrnd->w),
        static_cast<float>(backgrnd->h),
    };
    SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
    break;
  }
  case sound: {
    static auto backgrnd = wz_resource::load_texture(
        wz_resource::ui->find(u"Options.img/sound/layer:backgrnd"));
    SDL_FRect pos_rect{
        pos.x + 9,
        pos.y + 53,
        static_cast<float>(backgrnd->w),
        static_cast<float>(backgrnd->h),
    };
    SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
    // master volume
    // hp warn
    freetype::load_aligned(true);
    freetype::load_color(255, 255, 255, 255);
    freetype::draw_cstr(u"Master Volume", pos.x + 12, pos.y + 53, 105, 1);
    freetype::load_color(0, 0, 0, 255);

    freetype::draw_line(u"0%", pos.x + 136, pos.y + 91);
    slider_ui_system::render_hslider(pos.x + 163, pos.y + 95, hp_warn, 100, 167,
                                     top, false);
    freetype::draw_line(u"100%", pos.x + 335, pos.y + 91);
    SDL_Texture *t;
    if (master_volume_mute) {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    freetype::load_aligned(true);
    freetype::load_size(12);
    freetype::load_color(0, 0, 0, 255);
    freetype::draw_line(u"Mute", pos.x, pos.y);

    // BGM
    freetype::load_color(255, 255, 255, 255);
    freetype::draw_cstr(u"BGM", pos.x + 12, pos.y + 53, 105, 1);
    freetype::load_color(0, 0, 0, 255);

    freetype::draw_line(u"0%", pos.x + 136, pos.y + 91);
    slider_ui_system::render_hslider(pos.x + 163, pos.y + 95, hp_warn, 100, 167,
                                     top, false);
    freetype::draw_line(u"100%", pos.x + 335, pos.y + 91);
    if (bgm_volume_mute) {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    freetype::load_aligned(true);
    freetype::load_size(12);
    freetype::load_color(0, 0, 0, 255);
    freetype::draw_line(u"Mute", pos.x, pos.y);

    // Normal SFX
    freetype::load_color(255, 255, 255, 255);
    freetype::draw_cstr(u"Normal SFX", pos.x + 12, pos.y + 53, 105, 1);
    freetype::load_color(0, 0, 0, 255);

    freetype::draw_line(u"0%", pos.x + 136, pos.y + 91);
    slider_ui_system::render_hslider(pos.x + 163, pos.y + 95, hp_warn, 100, 167,
                                     top, false);
    freetype::draw_line(u"100%", pos.x + 335, pos.y + 91);
    if (normal_volume_mute) {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    freetype::load_aligned(true);
    freetype::load_size(12);
    freetype::load_color(0, 0, 0, 255);
    freetype::draw_line(u"Mute", pos.x, pos.y);

    // Monster SFX
    freetype::load_color(255, 255, 255, 255);
    freetype::draw_cstr(u"Monster SFX", pos.x + 12, pos.y + 53, 105, 1);
    freetype::load_color(0, 0, 0, 255);

    freetype::draw_line(u"0%", pos.x + 136, pos.y + 91);
    slider_ui_system::render_hslider(pos.x + 163, pos.y + 95, hp_warn, 100, 167,
                                     top, false);
    freetype::draw_line(u"100%", pos.x + 335, pos.y + 91);
    if (monster_volume_mute) {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    freetype::load_aligned(true);
    freetype::load_size(12);
    freetype::load_color(0, 0, 0, 255);
    freetype::draw_line(u"Mute", pos.x, pos.y);

    // Skill SFX
    freetype::load_color(255, 255, 255, 255);
    freetype::draw_cstr(u"Skill SFX", pos.x + 12, pos.y + 53, 105, 1);
    freetype::load_color(0, 0, 0, 255);

    freetype::draw_line(u"0%", pos.x + 136, pos.y + 91);
    slider_ui_system::render_hslider(pos.x + 163, pos.y + 95, hp_warn, 100, 167,
                                     top, false);
    freetype::draw_line(u"100%", pos.x + 335, pos.y + 91);
    if (skill_volume_mute) {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    freetype::load_aligned(true);
    freetype::load_size(12);
    freetype::load_color(0, 0, 0, 255);
    freetype::draw_line(u"Mute", pos.x, pos.y);
    break;
  }
  case game: {
    static auto backgrnd = wz_resource::load_texture(
        wz_resource::ui->find(u"Options.img/game/layer:backgrnd"));
    SDL_FRect pos_rect{
        pos.x + 9,
        pos.y + 53,
        static_cast<float>(backgrnd->w),
        static_cast<float>(backgrnd->h),
    };
    SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
    // hp warn
    freetype::load_aligned(true);
    freetype::load_color(255, 255, 255, 255);
    freetype::draw_cstr(u"HP Warning", pos.x + 12, pos.y + 53, 105, 1.3);
    freetype::load_color(0, 0, 0, 255);

    freetype::draw_line(u"0%", pos.x + 136, pos.y + 58);
    // hp slider
    slider_ui_system::render_hslider(pos.x + 163, pos.y + 63, hp_warn, 100, 167,
                                     top, false);
    freetype::draw_line(u"100%", pos.x + 335, pos.y + 58);

    // mp warn
    freetype::load_color(255, 255, 255, 255);
    freetype::draw_cstr(u"MP Warning", pos.x + 12, pos.y + 86, 105, 1.3);
    freetype::load_color(0, 0, 0, 255);
    freetype::draw_line(u"0%", pos.x + 136, pos.y + 91);
    slider_ui_system::render_hslider(pos.x + 163, pos.y + 95, hp_warn, 100, 167,
                                     top, false);
    freetype::draw_line(u"100%", pos.x + 335, pos.y + 91);

    // enemy info
    freetype::load_color(255, 255, 255, 255);
    freetype::draw_cstr(u"Enemy Info", pos.x + 12, pos.y + 119, 105, 1.3);
    freetype::load_color(0, 0, 0, 255);
    combo_box_ui_system::render(pos.x + 131, pos.y + 124, enemy_info_cm, top,
                                false);
    // screen format
    freetype::load_color(255, 255, 255, 255);
    freetype::draw_cstr(u"ScreenShot File\n Format", pos.x + 12, pos.y + 158,
                        105, 1);
    freetype::load_color(0, 0, 0, 255);
    combo_box_ui_system::render(pos.x + 131, pos.y + 164, screenshot_format,
                                top, false);
    // screen folder
    freetype::load_color(255, 255, 255, 255);
    freetype::draw_cstr(u"ScreenShot File\n Format", pos.x + 12, pos.y + 158,
                        105, 1);
    freetype::load_color(0, 0, 0, 255);

    combo_box_ui_system::render(pos.x + 131, pos.y + 197, screenshot_location,
                                top, false);
    // burst screen shot
    SDL_Texture *t;
    if (continue_capture) {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    std::u16string text = u"Continuous Capture";
    freetype::load_size(12);
    freetype::draw_line(text, pos_rect.x + 100, pos_rect.y);

    auto tmp = std::to_string(continue_capture_num);
    text = std::u16string{tmp.begin(), tmp.end()} + u"   Shots";
    freetype::load_color(0, 0, 0, 255);
    freetype::draw_line(text, pos_rect.x + 100, pos_rect.y);

    // buttons_nodes = {
    //     wz_resource::ui->find(u"Basic.img/BtMax"),
    //     wz_resource::ui->find(u"Basic.img/BtMin"),
    //     wz_resource::ui->find(u"Basic.img/BtMax"),
    //     wz_resource::ui->find(u"Basic.img/BtMin"),
    // };
    buttons_rect = {

    };

    text = u"2 to 10 Shots Avaliable";
    freetype::draw_line(text, pos.x, pos.y);

    if (auto_capture) {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    text = u"Auto-Capture Memorable Moments";

    if (memory_map) {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    text = u"Use";

    tmp = std::to_string(memory_limit);
    text = std::u16string{tmp.begin(), tmp.end()} + u"   Shots";
    freetype::load_color(0, 0, 0, 255);
    freetype::draw_line(text, pos_rect.x + 100, pos_rect.y);

    break;
  }
  case social: {
    static auto backgrnd = wz_resource::load_texture(
        wz_resource::ui->find(u"Options.img/game/layer:backgrnd"));
    SDL_FRect pos_rect{
        pos.x + 9,
        pos.y + 53,
        static_cast<float>(backgrnd->w),
        static_cast<float>(backgrnd->h),
    };
    SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);

    SDL_Texture *t;
    //whisper
    if (whisper) {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    std::u16string text = u"Continuous Capture";
    freetype::load_size(12);
    freetype::draw_line(text, pos_rect.x + 100, pos_rect.y);
  }
  }
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

void option_ui_system::render_tab() {
  const static std::array tab_pos = {
      SDL_FPoint{10, 24},  //
      SDL_FPoint{81, 24},  //
      SDL_FPoint{152, 24}, //
      SDL_FPoint{223, 24}, //
      SDL_FPoint{294, 24}, //
  };
  const static auto tab_node = wz_resource::ui->find(u"Options.img/tab");
  const static std::array active_texture = {
      wz_resource::load_texture(tab_node->find(u"enabled/0")),
      wz_resource::load_texture(tab_node->find(u"enabled/1")),
      wz_resource::load_texture(tab_node->find(u"enabled/2")),
      wz_resource::load_texture(tab_node->find(u"enabled/3")),
      wz_resource::load_texture(tab_node->find(u"enabled/4")),
  };
  const static std::array disabled_texture = {
      wz_resource::load_texture(tab_node->find(u"disabled/0")),
      wz_resource::load_texture(tab_node->find(u"disabled/1")),
      wz_resource::load_texture(tab_node->find(u"disabled/2")),
      wz_resource::load_texture(tab_node->find(u"disabled/3")),
      wz_resource::load_texture(tab_node->find(u"disabled/4")),
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

    active_tab = game;

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

void option_ui_system::event_tab(SDL_Event *event) {
  const static std::array tab_rect = {
      SDL_FRect{5, 26, 33, 19},  //
      SDL_FRect{38, 26, 33, 19}, //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      active_tab = i;
    }
  }
}

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

void option_ui_system::event_motion_bt_audio(SDL_Event *event) {
  if (cursor_game_instance::cursor_ui != render) {
    return;
  }
  auto wh = load_wh();
  std::vector<SDL_FRect> buttons_rect = {};

  static int8_t bt_audio = -1;
  auto &mouse_pos = window::mouse_pos;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      bt_in = i;
      break;
    }
  }
  if (bt_in != -1 && bt_audio != bt_in) {
    audio_game_instance::load_audio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
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

bool option_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}