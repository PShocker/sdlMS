#include "keybinding_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/quick_slot_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <flat_map>
#include <string>

const static inline std::flat_map<SDL_Scancode, SDL_FPoint>
    mapped_scancode_position = {
        {SDL_SCANCODE_F1, {82, 27}},
        {SDL_SCANCODE_F2, {116, 27}},
        {SDL_SCANCODE_F3, {150, 27}},
        {SDL_SCANCODE_F4, {184, 27}},
        {SDL_SCANCODE_F5, {226, 27}},
        {SDL_SCANCODE_F6, {260, 27}},
        {SDL_SCANCODE_F7, {294, 27}},
        {SDL_SCANCODE_F8, {328, 27}},
        {SDL_SCANCODE_F9, {370, 27}},
        {SDL_SCANCODE_F10, {404, 27}},
        {SDL_SCANCODE_F11, {438, 27}},
        {SDL_SCANCODE_F12, {472, 27}},
        {SDL_SCANCODE_GRAVE, {14, 66}},
        {SDL_SCANCODE_1, {48, 66}},
        {SDL_SCANCODE_2, {82, 66}},
        {SDL_SCANCODE_3, {116, 66}},
        {SDL_SCANCODE_4, {150, 66}},
        {SDL_SCANCODE_5, {184, 66}},
        {SDL_SCANCODE_6, {218, 66}},
        {SDL_SCANCODE_7, {252, 66}},
        {SDL_SCANCODE_8, {286, 66}},
        {SDL_SCANCODE_9, {320, 66}},
        {SDL_SCANCODE_0, {354, 66}},
        {SDL_SCANCODE_MINUS, {388, 66}},
        {SDL_SCANCODE_EQUALS, {422, 66}},
        {SDL_SCANCODE_Q, {64, 99}},
        {SDL_SCANCODE_W, {98, 99}},
        {SDL_SCANCODE_E, {132, 99}},
        {SDL_SCANCODE_R, {166, 99}},
        {SDL_SCANCODE_T, {200, 99}},
        {SDL_SCANCODE_Y, {234, 99}},
        {SDL_SCANCODE_U, {268, 99}},
        {SDL_SCANCODE_I, {302, 99}},
        {SDL_SCANCODE_O, {336, 99}},
        {SDL_SCANCODE_P, {370, 99}},
        {SDL_SCANCODE_LEFTBRACKET, {404, 99}},
        {SDL_SCANCODE_RIGHTBRACKET, {438, 99}},
        {SDL_SCANCODE_BACKSLASH, {472, 99}},
        {SDL_SCANCODE_A, {81, 132}},
        {SDL_SCANCODE_S, {115, 132}},
        {SDL_SCANCODE_D, {149, 132}},
        {SDL_SCANCODE_F, {183, 132}},
        {SDL_SCANCODE_G, {217, 132}},
        {SDL_SCANCODE_H, {251, 132}},
        {SDL_SCANCODE_J, {285, 132}},
        {SDL_SCANCODE_K, {319, 132}},
        {SDL_SCANCODE_L, {353, 132}},
        {SDL_SCANCODE_SEMICOLON, {387, 132}},
        {SDL_SCANCODE_APOSTROPHE, {421, 132}},
        {SDL_SCANCODE_LSHIFT, {457, 165}}, // 左Shift
        {SDL_SCANCODE_RSHIFT, {38, 165}},  // 右Shift
        {SDL_SCANCODE_Z, {98, 165}},
        {SDL_SCANCODE_X, {132, 165}},
        {SDL_SCANCODE_C, {166, 165}},
        {SDL_SCANCODE_V, {200, 165}},
        {SDL_SCANCODE_B, {234, 165}},
        {SDL_SCANCODE_N, {268, 165}},
        {SDL_SCANCODE_M, {302, 165}},
        {SDL_SCANCODE_COMMA, {336, 165}},
        {SDL_SCANCODE_PERIOD, {370, 165}},
        {SDL_SCANCODE_LCTRL, {22, 198}},  // 左Ctrl
        {SDL_SCANCODE_RCTRL, {460, 198}}, // 右Ctrl
        {SDL_SCANCODE_LALT, {122, 198}},  // 左Alt
        {SDL_SCANCODE_RALT, {348, 198}},  // 右Alt
        {SDL_SCANCODE_SPACE, {232, 198}},
        {SDL_SCANCODE_INSERT, {514, 66}},
        {SDL_SCANCODE_HOME, {548, 66}},
        {SDL_SCANCODE_PAGEUP, {582, 66}},
        {SDL_SCANCODE_DELETE, {514, 99}},
        {SDL_SCANCODE_END, {548, 99}},
        {SDL_SCANCODE_PAGEDOWN, {582, 99}},
        {SDL_SCANCODE_UP, {548, 165}},    // up
        {SDL_SCANCODE_DOWN, {548, 198}},  // down
        {SDL_SCANCODE_LEFT, {514, 198}},  // left
        {SDL_SCANCODE_RIGHT, {582, 198}}, // right

};

SDL_FPoint keybinding_ui_system::load_wh() { return {636, 445}; }

const static SDL_FPoint key_lt{5, 56};

void keybinding_ui_system::render_backgrnd_key() {
  static auto keys = wz_resource::ui->find(u"KeyBindings.img/key");
  static const std::flat_map<SDL_Scancode, SDL_Texture *> key_textures = {
      {SDL_SCANCODE_ESCAPE, wz_resource::load_texture(keys->get_child(u"1"))},
      {SDL_SCANCODE_1, wz_resource::load_texture(keys->get_child(u"2"))},
      {SDL_SCANCODE_2, wz_resource::load_texture(keys->get_child(u"3"))},
      {SDL_SCANCODE_3, wz_resource::load_texture(keys->get_child(u"4"))},
      {SDL_SCANCODE_4, wz_resource::load_texture(keys->get_child(u"5"))},
      {SDL_SCANCODE_5, wz_resource::load_texture(keys->get_child(u"6"))},
      {SDL_SCANCODE_6, wz_resource::load_texture(keys->get_child(u"7"))},
      {SDL_SCANCODE_7, wz_resource::load_texture(keys->get_child(u"8"))},
      {SDL_SCANCODE_8, wz_resource::load_texture(keys->get_child(u"9"))},
      {SDL_SCANCODE_9, wz_resource::load_texture(keys->get_child(u"10"))},
      {SDL_SCANCODE_0, wz_resource::load_texture(keys->get_child(u"11"))},
      {SDL_SCANCODE_MINUS, wz_resource::load_texture(keys->get_child(u"12"))},
      {SDL_SCANCODE_EQUALS, wz_resource::load_texture(keys->get_child(u"13"))},
      {SDL_SCANCODE_Q, wz_resource::load_texture(keys->get_child(u"16"))},
      {SDL_SCANCODE_W, wz_resource::load_texture(keys->get_child(u"17"))},
      {SDL_SCANCODE_E, wz_resource::load_texture(keys->get_child(u"18"))},
      {SDL_SCANCODE_R, wz_resource::load_texture(keys->get_child(u"19"))},
      {SDL_SCANCODE_T, wz_resource::load_texture(keys->get_child(u"20"))},
      {SDL_SCANCODE_Y, wz_resource::load_texture(keys->get_child(u"21"))},
      {SDL_SCANCODE_U, wz_resource::load_texture(keys->get_child(u"22"))},
      {SDL_SCANCODE_I, wz_resource::load_texture(keys->get_child(u"23"))},
      {SDL_SCANCODE_O, wz_resource::load_texture(keys->get_child(u"24"))},
      {SDL_SCANCODE_P, wz_resource::load_texture(keys->get_child(u"25"))},
      {SDL_SCANCODE_LEFTBRACKET,
       wz_resource::load_texture(keys->get_child(u"26"))},
      {SDL_SCANCODE_RIGHTBRACKET,
       wz_resource::load_texture(keys->get_child(u"27"))},
      {SDL_SCANCODE_LCTRL, wz_resource::load_texture(keys->get_child(u"29"))},
      {SDL_SCANCODE_A, wz_resource::load_texture(keys->get_child(u"30"))},
      {SDL_SCANCODE_S, wz_resource::load_texture(keys->get_child(u"31"))},
      {SDL_SCANCODE_D, wz_resource::load_texture(keys->get_child(u"32"))},
      {SDL_SCANCODE_F, wz_resource::load_texture(keys->get_child(u"33"))},
      {SDL_SCANCODE_G, wz_resource::load_texture(keys->get_child(u"34"))},
      {SDL_SCANCODE_H, wz_resource::load_texture(keys->get_child(u"35"))},
      {SDL_SCANCODE_J, wz_resource::load_texture(keys->get_child(u"36"))},
      {SDL_SCANCODE_K, wz_resource::load_texture(keys->get_child(u"37"))},
      {SDL_SCANCODE_L, wz_resource::load_texture(keys->get_child(u"38"))},
      {SDL_SCANCODE_SEMICOLON,
       wz_resource::load_texture(keys->get_child(u"39"))},
      {SDL_SCANCODE_APOSTROPHE,
       wz_resource::load_texture(keys->get_child(u"40"))},
      {SDL_SCANCODE_GRAVE, wz_resource::load_texture(keys->get_child(u"41"))},
      {SDL_SCANCODE_LSHIFT, wz_resource::load_texture(keys->get_child(u"42"))},
      {SDL_SCANCODE_BACKSLASH,
       wz_resource::load_texture(keys->get_child(u"43"))},
      {SDL_SCANCODE_Z, wz_resource::load_texture(keys->get_child(u"44"))},
      {SDL_SCANCODE_X, wz_resource::load_texture(keys->get_child(u"45"))},
      {SDL_SCANCODE_C, wz_resource::load_texture(keys->get_child(u"46"))},
      {SDL_SCANCODE_V, wz_resource::load_texture(keys->get_child(u"47"))},
      {SDL_SCANCODE_B, wz_resource::load_texture(keys->get_child(u"48"))},
      {SDL_SCANCODE_N, wz_resource::load_texture(keys->get_child(u"49"))},
      {SDL_SCANCODE_M, wz_resource::load_texture(keys->get_child(u"50"))},
      {SDL_SCANCODE_COMMA, wz_resource::load_texture(keys->get_child(u"51"))},
      {SDL_SCANCODE_PERIOD, wz_resource::load_texture(keys->get_child(u"52"))},
      {SDL_SCANCODE_LALT, wz_resource::load_texture(keys->get_child(u"56"))},
      {SDL_SCANCODE_SPACE, wz_resource::load_texture(keys->get_child(u"57"))},
      {SDL_SCANCODE_F1, wz_resource::load_texture(keys->get_child(u"59"))},
      {SDL_SCANCODE_F2, wz_resource::load_texture(keys->get_child(u"60"))},
      {SDL_SCANCODE_F3, wz_resource::load_texture(keys->get_child(u"61"))},
      {SDL_SCANCODE_F4, wz_resource::load_texture(keys->get_child(u"62"))},
      {SDL_SCANCODE_F5, wz_resource::load_texture(keys->get_child(u"63"))},
      {SDL_SCANCODE_F6, wz_resource::load_texture(keys->get_child(u"64"))},
      {SDL_SCANCODE_F7, wz_resource::load_texture(keys->get_child(u"65"))},
      {SDL_SCANCODE_F8, wz_resource::load_texture(keys->get_child(u"66"))},
      {SDL_SCANCODE_F9, wz_resource::load_texture(keys->get_child(u"67"))},
      {SDL_SCANCODE_F10, wz_resource::load_texture(keys->get_child(u"68"))},
      {SDL_SCANCODE_F11, wz_resource::load_texture(keys->get_child(u"87"))},
      {SDL_SCANCODE_F12, wz_resource::load_texture(keys->get_child(u"88"))},

      // 以下特殊按键的映射可能需要根据实际 SDL3 定义调整
      {SDL_SCANCODE_HOME,
       wz_resource::load_texture(keys->get_child(u"71"))}, // Hm
      {SDL_SCANCODE_PAGEUP,
       wz_resource::load_texture(keys->get_child(u"73"))}, // Pup
      {SDL_SCANCODE_END,
       wz_resource::load_texture(keys->get_child(u"79"))}, // End
      {SDL_SCANCODE_PAGEDOWN,
       wz_resource::load_texture(keys->get_child(u"81"))}, // Pdn
      {SDL_SCANCODE_INSERT,
       wz_resource::load_texture(keys->get_child(u"82"))}, // Ins
      {SDL_SCANCODE_DELETE,
       wz_resource::load_texture(keys->get_child(u"83"))}, // Del

      {SDL_SCANCODE_UP, wz_resource::load_texture(keys->get_child(u"72"))},
      {SDL_SCANCODE_DOWN, wz_resource::load_texture(keys->get_child(u"80"))},
      {SDL_SCANCODE_LEFT, wz_resource::load_texture(keys->get_child(u"75"))},
      {SDL_SCANCODE_RIGHT, wz_resource::load_texture(keys->get_child(u"77"))},

  };
  for (auto [k, v] : key_textures) {
    if (!mapped_scancode_position.contains(k)) {
      continue;
    }
    auto key_pos = mapped_scancode_position.at(k);
    // render icon
    int x = pos.x + key_pos.x + key_lt.x;
    int y = pos.y + key_pos.y + key_lt.y;
    if (keyboard_game_instance::data.contains(k)) {
      const auto &input = keyboard_game_instance::data.at(k);
      quick_slot_ui_system::render_input_icon(input, x - 1, y - 1);
    }
    SDL_FRect pos_rect{
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(v->w),
        static_cast<float>(v->h),
    };
    // pos_rect.w = 32;
    // pos_rect.h = 32;
    // SDL_RenderFillRect(window::renderer, &pos_rect);
    SDL_RenderTexture(window::renderer, v, nullptr, &pos_rect);
  }
}

void keybinding_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"KeyBindings.img/backgrnd"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
  return;
}

void keybinding_ui_system::render_tab() {
  static auto node = wz_resource::ui->find(u"KeyBindings.img/tab:kind");
  const SDL_FPoint lt{9, 24};
  for (auto i : {0, 1}) {
    SDL_Texture *texture;
    if (active_tab == i) {
      texture = wz_resource::load_texture(
          node->get_child(u"selected")->get_child(std::to_string(i)));
    } else {
      texture = wz_resource::load_texture(
          node->get_child(u"normal")->get_child(std::to_string(i)));
    }
    SDL_FRect pos_rect = {
        int(pos.x) + lt.x + texture->w * i,
        int(pos.y) + lt.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
}

void keybinding_ui_system::render_backgrnd2() {
  static auto backgrnd2 = wz_resource::load_texture(
      wz_resource::ui->find(u"KeyBindings.img/backgrnd_keyboard"));
  SDL_FPoint lt{9, 53};
  SDL_FRect pos_rect{
      pos.x + lt.x,
      pos.y + lt.y,
      static_cast<float>(backgrnd2->w),
      static_cast<float>(backgrnd2->h),
  };
  SDL_RenderTexture(window::renderer, backgrnd2, nullptr, &pos_rect);
  return;
}

bool keybinding_ui_system::render() {
  render_backgrnd();
  render_backgrnd2();
  render_tab();
  render_backgrnd_key();
  return true;
}

void keybinding_ui_system::open() {
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

void keybinding_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  event_drag_end();
}

bool keybinding_ui_system::event(SDL_Event *event) { return true; }

void keybinding_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void keybinding_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void keybinding_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void keybinding_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void keybinding_ui_system::toggle() {
  audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}