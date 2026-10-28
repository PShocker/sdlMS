#include "keybinding_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_input.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/quick_slot_ui_system.h"
#include "src/client/system/ui/tooltip_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <flat_map>
#include <optional>
#include <string>

static std::flat_map<SDL_Scancode, GameInput> data_bak;

const static inline std::flat_map<SDL_Scancode, SDL_FPoint>
    mapped_scancode_position = {
        {SDL_SCANCODE_ESCAPE, {14, 27}},
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
        {SDL_SCANCODE_RSHIFT, {457, 165}}, // 左Shift
        {SDL_SCANCODE_LSHIFT, {38, 165}},  // 右Shift
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

SDL_FPoint KeybindingUiSystem::load_wh() { return {636, 445}; }

const static SDL_FPoint key_lt{5, 56};

static std::optional<SDL_Scancode> backgrnd_key;

void KeybindingUiSystem::render_backgrnd_key() {
  auto keys = WzResource::ui->find(u"KeyBindings.img/key");
  static const std::flat_map<SDL_Scancode, SDL_Texture *> key_textures = {
      {SDL_SCANCODE_ESCAPE, WzResource::load_texture(keys->get_child(u"1"))},
      {SDL_SCANCODE_1, WzResource::load_texture(keys->get_child(u"2"))},
      {SDL_SCANCODE_2, WzResource::load_texture(keys->get_child(u"3"))},
      {SDL_SCANCODE_3, WzResource::load_texture(keys->get_child(u"4"))},
      {SDL_SCANCODE_4, WzResource::load_texture(keys->get_child(u"5"))},
      {SDL_SCANCODE_5, WzResource::load_texture(keys->get_child(u"6"))},
      {SDL_SCANCODE_6, WzResource::load_texture(keys->get_child(u"7"))},
      {SDL_SCANCODE_7, WzResource::load_texture(keys->get_child(u"8"))},
      {SDL_SCANCODE_8, WzResource::load_texture(keys->get_child(u"9"))},
      {SDL_SCANCODE_9, WzResource::load_texture(keys->get_child(u"10"))},
      {SDL_SCANCODE_0, WzResource::load_texture(keys->get_child(u"11"))},
      {SDL_SCANCODE_MINUS, WzResource::load_texture(keys->get_child(u"12"))},
      {SDL_SCANCODE_EQUALS, WzResource::load_texture(keys->get_child(u"13"))},
      {SDL_SCANCODE_Q, WzResource::load_texture(keys->get_child(u"16"))},
      {SDL_SCANCODE_W, WzResource::load_texture(keys->get_child(u"17"))},
      {SDL_SCANCODE_E, WzResource::load_texture(keys->get_child(u"18"))},
      {SDL_SCANCODE_R, WzResource::load_texture(keys->get_child(u"19"))},
      {SDL_SCANCODE_T, WzResource::load_texture(keys->get_child(u"20"))},
      {SDL_SCANCODE_Y, WzResource::load_texture(keys->get_child(u"21"))},
      {SDL_SCANCODE_U, WzResource::load_texture(keys->get_child(u"22"))},
      {SDL_SCANCODE_I, WzResource::load_texture(keys->get_child(u"23"))},
      {SDL_SCANCODE_O, WzResource::load_texture(keys->get_child(u"24"))},
      {SDL_SCANCODE_P, WzResource::load_texture(keys->get_child(u"25"))},
      {SDL_SCANCODE_LEFTBRACKET,
       WzResource::load_texture(keys->get_child(u"26"))},
      {SDL_SCANCODE_RIGHTBRACKET,
       WzResource::load_texture(keys->get_child(u"27"))},
      {SDL_SCANCODE_LCTRL, WzResource::load_texture(keys->get_child(u"29"))},
      {SDL_SCANCODE_RCTRL, WzResource::load_texture(keys->get_child(u"29"))},
      {SDL_SCANCODE_A, WzResource::load_texture(keys->get_child(u"30"))},
      {SDL_SCANCODE_S, WzResource::load_texture(keys->get_child(u"31"))},
      {SDL_SCANCODE_D, WzResource::load_texture(keys->get_child(u"32"))},
      {SDL_SCANCODE_F, WzResource::load_texture(keys->get_child(u"33"))},
      {SDL_SCANCODE_G, WzResource::load_texture(keys->get_child(u"34"))},
      {SDL_SCANCODE_H, WzResource::load_texture(keys->get_child(u"35"))},
      {SDL_SCANCODE_J, WzResource::load_texture(keys->get_child(u"36"))},
      {SDL_SCANCODE_K, WzResource::load_texture(keys->get_child(u"37"))},
      {SDL_SCANCODE_L, WzResource::load_texture(keys->get_child(u"38"))},
      {SDL_SCANCODE_SEMICOLON,
       WzResource::load_texture(keys->get_child(u"39"))},
      {SDL_SCANCODE_APOSTROPHE,
       WzResource::load_texture(keys->get_child(u"40"))},
      {SDL_SCANCODE_GRAVE, WzResource::load_texture(keys->get_child(u"41"))},
      {SDL_SCANCODE_LSHIFT, WzResource::load_texture(keys->get_child(u"42"))},
      {SDL_SCANCODE_RSHIFT, WzResource::load_texture(keys->get_child(u"42"))},
      {SDL_SCANCODE_BACKSLASH,
       WzResource::load_texture(keys->get_child(u"43"))},
      {SDL_SCANCODE_Z, WzResource::load_texture(keys->get_child(u"44"))},
      {SDL_SCANCODE_X, WzResource::load_texture(keys->get_child(u"45"))},
      {SDL_SCANCODE_C, WzResource::load_texture(keys->get_child(u"46"))},
      {SDL_SCANCODE_V, WzResource::load_texture(keys->get_child(u"47"))},
      {SDL_SCANCODE_B, WzResource::load_texture(keys->get_child(u"48"))},
      {SDL_SCANCODE_N, WzResource::load_texture(keys->get_child(u"49"))},
      {SDL_SCANCODE_M, WzResource::load_texture(keys->get_child(u"50"))},
      {SDL_SCANCODE_COMMA, WzResource::load_texture(keys->get_child(u"51"))},
      {SDL_SCANCODE_PERIOD, WzResource::load_texture(keys->get_child(u"52"))},
      {SDL_SCANCODE_LALT, WzResource::load_texture(keys->get_child(u"56"))},
      {SDL_SCANCODE_RALT, WzResource::load_texture(keys->get_child(u"56"))},
      {SDL_SCANCODE_SPACE, WzResource::load_texture(keys->get_child(u"57"))},
      {SDL_SCANCODE_F1, WzResource::load_texture(keys->get_child(u"59"))},
      {SDL_SCANCODE_F2, WzResource::load_texture(keys->get_child(u"60"))},
      {SDL_SCANCODE_F3, WzResource::load_texture(keys->get_child(u"61"))},
      {SDL_SCANCODE_F4, WzResource::load_texture(keys->get_child(u"62"))},
      {SDL_SCANCODE_F5, WzResource::load_texture(keys->get_child(u"63"))},
      {SDL_SCANCODE_F6, WzResource::load_texture(keys->get_child(u"64"))},
      {SDL_SCANCODE_F7, WzResource::load_texture(keys->get_child(u"65"))},
      {SDL_SCANCODE_F8, WzResource::load_texture(keys->get_child(u"66"))},
      {SDL_SCANCODE_F9, WzResource::load_texture(keys->get_child(u"67"))},
      {SDL_SCANCODE_F10, WzResource::load_texture(keys->get_child(u"68"))},
      {SDL_SCANCODE_F11, WzResource::load_texture(keys->get_child(u"87"))},
      {SDL_SCANCODE_F12, WzResource::load_texture(keys->get_child(u"88"))},

      // 以下特殊按键的映射可能需要根据实际 SDL3 定义调整
      {SDL_SCANCODE_HOME,
       WzResource::load_texture(keys->get_child(u"71"))}, // Hm
      {SDL_SCANCODE_PAGEUP,
       WzResource::load_texture(keys->get_child(u"73"))}, // Pup
      {SDL_SCANCODE_END,
       WzResource::load_texture(keys->get_child(u"79"))}, // End
      {SDL_SCANCODE_PAGEDOWN,
       WzResource::load_texture(keys->get_child(u"81"))}, // Pdn
      {SDL_SCANCODE_INSERT,
       WzResource::load_texture(keys->get_child(u"82"))}, // Ins
      {SDL_SCANCODE_DELETE,
       WzResource::load_texture(keys->get_child(u"83"))}, // Del

      {SDL_SCANCODE_UP, WzResource::load_texture(keys->get_child(u"72"))},
      {SDL_SCANCODE_DOWN, WzResource::load_texture(keys->get_child(u"80"))},
      {SDL_SCANCODE_LEFT, WzResource::load_texture(keys->get_child(u"75"))},
      {SDL_SCANCODE_RIGHT, WzResource::load_texture(keys->get_child(u"77"))},

  };
  backgrnd_key = std::nullopt;
  auto &mouse_pos = Window::mouse_pos;
  for (auto [k, v] : key_textures) {
    if (!mapped_scancode_position.contains(k)) {
      continue;
    }
    auto key_pos = mapped_scancode_position.at(k);
    // render icon
    int x = pos.x + key_pos.x + key_lt.x;
    int y = pos.y + key_pos.y + key_lt.y;
    SDL_FRect pp{static_cast<float>(x), static_cast<float>(y), 32, 32};
    if (SDL_PointInRectFloat(&mouse_pos, &pp)) {
      backgrnd_key = k;
    }
    if (KeyboardGameInstance::data.contains(k)) {
      const auto &input = KeyboardGameInstance::data.at(k);
      QuickSlotUiSystem::render_input_icon(input, x - 1, y - 1);
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
    SDL_RenderTexture(Window::renderer, v, nullptr, &pos_rect);
  }
}

void KeybindingUiSystem::render_backgrnd() {
  static auto backgrnd = WzResource::load_texture(
      WzResource::ui->find(u"KeyBindings.img/backgrnd"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);
  return;
}

void KeybindingUiSystem::render_tab() {
  static auto node = WzResource::ui->find(u"KeyBindings.img/tab:kind");
  const SDL_FPoint lt{9, 24};
  for (auto i : {0, 1}) {
    SDL_Texture *texture;
    if (active_tab == i) {
      texture = WzResource::load_texture(
          node->get_child(u"selected")->get_child(std::to_string(i)));
    } else {
      texture = WzResource::load_texture(
          node->get_child(u"normal")->get_child(std::to_string(i)));
    }
    SDL_FRect pos_rect = {
        int(pos.x) + lt.x + texture->w * i,
        int(pos.y) + lt.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
  }
}

void KeybindingUiSystem::render_backgrnd2() {
  static auto backgrnd2 = WzResource::load_texture(
      WzResource::ui->find(u"KeyBindings.img/backgrnd_keyboard"));
  SDL_FPoint lt{9, 53};
  SDL_FRect pos_rect{
      pos.x + lt.x,
      pos.y + lt.y,
      static_cast<float>(backgrnd2->w),
      static_cast<float>(backgrnd2->h),
  };
  SDL_RenderTexture(Window::renderer, backgrnd2, nullptr, &pos_rect);
  return;
}

void KeybindingUiSystem::render_button() {
  const static std::array buttons_nodes = {
      WzResource::ui->find(u"Basic.img/BtClose"),
      WzResource::ui->find(u"KeyBindings.img/button:Default"),
      WzResource::ui->find(u"KeyBindings.img/button:Delete"),
      WzResource::ui->find(u"KeyBindings.img/button:quickslots"),

      WzResource::ui->find(u"KeyBindings.img/button:presetPage1"),
      WzResource::ui->find(u"KeyBindings.img/button:presetPage2"),
      WzResource::ui->find(u"KeyBindings.img/button:presetPage3"),
      WzResource::ui->find(u"KeyBindings.img/button:presetSave"),
      WzResource::ui->find(u"KeyBindings.img/button:presetUndo"),
  };
  auto wh = load_wh();
  const std::array buttons_rect = {
      SDL_FRect{wh.x - 18, 6, 12, 12}, //
      SDL_FRect{6, 421, 54, 16},       //
      SDL_FRect{65, 421, 54, 16},      //
      SDL_FRect{172, 59, 84, 16},      //

      SDL_FRect{74, 60, 25, 14},   //
      SDL_FRect{103, 60, 25, 14},  //
      SDL_FRect{132, 60, 25, 14},  //
      SDL_FRect{495, 419, 65, 19}, //
      SDL_FRect{564, 419, 65, 19}, //

  };

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void KeybindingUiSystem::render_remain_icon(int i, int x, int y,
                                              uint8_t alpha) {
  auto input = KeyboardGameInstance::find_input_by_cursor(i);
  QuickSlotUiSystem::render_input_icon(input, x, y, alpha);
}

static std::optional<uint32_t> remain_key;
static SDL_FPoint remain_lt{12, 300};

void KeybindingUiSystem::render_remain_key() {
  remain_key = std::nullopt;

  const static std::flat_map<int, SDL_FPoint> m = {
      {CursorGameInstance::keybind_ui_menu, SDL_FPoint{0, 0}},
      {CursorGameInstance::keybind_face_hit, SDL_FPoint{34, 0}},
      {CursorGameInstance::keybind_face_smile, SDL_FPoint{68, 0}},
      {CursorGameInstance::keybind_face_troubled, SDL_FPoint{102, 0}},
      {CursorGameInstance::keybind_face_cry, SDL_FPoint{136, 0}},
      {CursorGameInstance::keybind_face_angry, SDL_FPoint{170, 0}},
      {CursorGameInstance::keybind_face_bewildered, SDL_FPoint{204, 0}},
      {CursorGameInstance::keybind_face_stunned, SDL_FPoint{238, 0}},
      {CursorGameInstance::keybind_ui_toAll, SDL_FPoint{272, 0}},
      {CursorGameInstance::keybind_ui_toParty, SDL_FPoint{306, 0}},
      {CursorGameInstance::keybind_ui_quest, SDL_FPoint{340, 0}},
      {CursorGameInstance::keybind_ui_worldMap, SDL_FPoint{374, 0}},
      {CursorGameInstance::keybind_ui_equip, SDL_FPoint{408, 0}},
      {CursorGameInstance::keybind_ui_package, SDL_FPoint{442, 0}},
      {CursorGameInstance::keybind_ui_buddy, SDL_FPoint{476, 0}},
      {CursorGameInstance::keybind_ui_party, SDL_FPoint{510, 0}},
      {CursorGameInstance::keybind_ui_mapleChat, SDL_FPoint{544, 0}},
      {CursorGameInstance::keybind_ui_quickSlot, SDL_FPoint{578, 0}},
      {CursorGameInstance::keybind_ui_keyBind, SDL_FPoint{0, 34}},
      {CursorGameInstance::keybind_ui_characterStat, SDL_FPoint{34, 34}},
      {CursorGameInstance::keybind_ui_whisper, SDL_FPoint{68, 34}},
      {CursorGameInstance::keybind_ui_skill, SDL_FPoint{102, 34}},
      {CursorGameInstance::keybind_ui_craft, SDL_FPoint{136, 34}},
      {CursorGameInstance::keybind_ui_mailBox, SDL_FPoint{170, 34}},
      {CursorGameInstance::keybind_ui_chatPlus, SDL_FPoint{204, 34}},
      {CursorGameInstance::keybind_ui_miniMap, SDL_FPoint{238, 34}},
      {CursorGameInstance::keybind_ui_changeChannel, SDL_FPoint{272, 34}},
      {CursorGameInstance::keybind_ui_ok, SDL_FPoint{306, 34}},
      {CursorGameInstance::keybind_action_pick, SDL_FPoint{340, 34}},
      {CursorGameInstance::keybind_action_sit, SDL_FPoint{374, 34}},
      {CursorGameInstance::keybind_action_attack, SDL_FPoint{408, 34}},
      {CursorGameInstance::keybind_action_jump, SDL_FPoint{442, 34}},
      {CursorGameInstance::keybind_action_harvest, SDL_FPoint{476, 34}},
      {CursorGameInstance::keybind_action_up, SDL_FPoint{510, 34}},
      {CursorGameInstance::keybind_action_down, SDL_FPoint{544, 34}},
      {CursorGameInstance::keybind_action_left, SDL_FPoint{578, 34}},
      {CursorGameInstance::keybind_action_right, SDL_FPoint{0, 68}},
  };
  const auto &key_data = KeyboardGameInstance::data;
  auto &mouse_pos = Window::mouse_pos;
  for (auto [k, v] : m) {
    auto input = KeyboardGameInstance::find_input_by_cursor(k);
    if (KeyboardGameInstance::find_key_by_val(input.val)) {
      continue;
    }
    SDL_FRect pos_rect;
    pos_rect.x = (int)pos.x + remain_lt.x + v.x;
    pos_rect.y = (int)pos.y + remain_lt.y + v.y;
    pos_rect.w = 32;
    pos_rect.h = 32;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      remain_key = k;
    }
    render_remain_icon(k, pos_rect.x, pos_rect.y);
  }
}

bool KeybindingUiSystem::render() {
  render_backgrnd();
  render_backgrnd2();
  render_button();
  render_tab();
  render_backgrnd_key();
  render_remain_key();

  return true;
}

void KeybindingUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    data_bak = KeyboardGameInstance::data;

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void KeybindingUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::render_systems, render_info);
  std::erase(System::event_systems, event);

  event_drag_end();
}

void KeybindingUiSystem::event_button_close() {
  close();
  KeyboardGameInstance::data = data_bak;
  return;
}

void KeybindingUiSystem::event_button_default() {
  KeyboardGameInstance::reset();
  return;
}

void KeybindingUiSystem::event_button_delete() {
  KeyboardGameInstance::data.clear();
  return;
}

void KeybindingUiSystem::event_button_quickSlot() { return; }

void KeybindingUiSystem::event_button_prePage1() { return; }

void KeybindingUiSystem::event_button_prePage2() { return; }

void KeybindingUiSystem::event_button_prePage3() { return; }

void KeybindingUiSystem::event_button_save() {
  close();
  return;
}

void KeybindingUiSystem::event_button_undo() {
  event_button_close();
  return;
}

void KeybindingUiSystem::event_motion_bt_audio(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  auto wh = load_wh();
  const std::array buttons_rect = {
      SDL_FRect{wh.x - 18, 6, 12, 12}, //
      SDL_FRect{6, 421, 54, 16},       //
      SDL_FRect{65, 421, 54, 16},      //
      SDL_FRect{172, 59, 84, 16},      //

      SDL_FRect{74, 60, 25, 14},   //
      SDL_FRect{103, 60, 25, 14},  //
      SDL_FRect{132, 60, 25, 14},  //
      SDL_FRect{495, 419, 65, 19}, //
      SDL_FRect{564, 419, 65, 19}, //

  };
  static int8_t bt_audio = -1;
  auto &mouse_pos = Window::mouse_pos;
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
    AudioGameInstance::load_audio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

void KeybindingUiSystem::event_backgrnd_key() {
  if (CursorGameInstance::cursor_hand_net.has_value()) {
    return;
  }
  if (!backgrnd_key.has_value()) {
    return;
  }
  auto scan_code = backgrnd_key.value();
  auto &cursor_hand = CursorGameInstance::cursor_hand;
  if (cursor_hand.has_value()) {
    auto input = KeyboardGameInstance::load_cursor_input();
    KeyboardGameInstance::insert_input(scan_code, input.value());
    CursorGameInstance::cursor_hand = std::nullopt;
    AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
  } else {
    if (KeyboardGameInstance::data.contains(scan_code)) {
      CursorGameInstance::cursor_hand = {
          .type = CursorGameInstance::keybind,
          .val = scan_code,
      };
      AudioGameInstance::load_audio(u"UI.img/DragStart", 0);
    }
  }
}

void KeybindingUiSystem::event_remain_key() {
  if (CursorGameInstance::cursor_hand_net.has_value()) {
    return;
  }
  const auto b_w = 1;
  const auto b_h = 1;
  SDL_FRect pos_rect;
  pos_rect.x = pos.x + remain_lt.x;
  pos_rect.y = pos.y + remain_lt.y;
  pos_rect.w = b_w;
  pos_rect.h = b_h;
  auto &mouse_pos = Window::mouse_pos;
  if (CursorGameInstance::cursor_hand.has_value()) {
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      CursorGameInstance::cursor_hand = std::nullopt;
      AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
      return;
    }
  }

  if (remain_key.has_value()) {
    CursorGameInstance::cursor_hand = {
        .type = CursorGameInstance::keybind,
        .val = SDL_SCANCODE_UNKNOWN,
        .sub_val = remain_key.value(),
    };
    AudioGameInstance::load_audio(u"UI.img/DragStart", 0);
  }
}

bool KeybindingUiSystem::event_button(SDL_Event *event) {
  std::vector<void (*)()> fns;
  auto wh = load_wh();
  const std::array buttons_rect = {
      SDL_FRect{wh.x - 18, 6, 12, 12}, //
      SDL_FRect{6, 421, 54, 16},       //
      SDL_FRect{65, 421, 54, 16},      //
      SDL_FRect{172, 59, 84, 16},      //

      SDL_FRect{74, 60, 25, 14},   //
      SDL_FRect{103, 60, 25, 14},  //
      SDL_FRect{132, 60, 25, 14},  //
      SDL_FRect{495, 419, 65, 19}, //
      SDL_FRect{564, 419, 65, 19}, //

  };
  fns = {
      event_button_close,     event_button_default,  event_button_delete,
      event_button_quickSlot, event_button_prePage1, event_button_prePage2,
      event_button_prePage3,  event_button_save,     event_button_undo,
  };

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      fns[i]();
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }

  return false;
}

bool KeybindingUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      event_button_close();
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
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
      if (CursorGameInstance::cursor_ui == render) {
        // event_tab(event);
        event_backgrnd_key();
        event_remain_key();
        r = !event_button(event);
      }
      event_drag_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_motion(event);
    event_drag_move(event);
    event_motion_bt_audio(event);
    break;
  }
  case SDL_EVENT_MOUSE_WHEEL: {
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

void KeybindingUiSystem::event_top() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void KeybindingUiSystem::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void KeybindingUiSystem::event_drag_end() {
  drag = std::nullopt;
  return;
}

void KeybindingUiSystem::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void KeybindingUiSystem::toggle() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool KeybindingUiSystem::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool KeybindingUiSystem::render_info() {
  if (CursorGameInstance::modal_overlay) {
    return true;
  }
  if (CursorGameInstance::cursor_ui != render) {
    return true;
  }
  if (CursorGameInstance::cursor_hand.has_value()) {
    return true;
  }
  if (remain_key.has_value()) {
    std::u16string text;
    switch ((CursorGameInstance::CursorKeybindType)remain_key.value()) {
    case CursorGameInstance::keybind_ui_menu: {
      text = u"menu";
      break;
    }
    case CursorGameInstance::keybind_face_hit: {
      text = u"hit";
      break;
    }
    case CursorGameInstance::keybind_face_smile: {
      text = u"smile";
      break;
    }
    case CursorGameInstance::keybind_face_troubled: {
      text = u"troubled";
      break;
    }
    case CursorGameInstance::keybind_face_cry: {
      text = u"menu";
      break;
    }
    case CursorGameInstance::keybind_face_angry: {
      text = u"angry";
      break;
    }
    case CursorGameInstance::keybind_face_bewildered: {
      text = u"bewildered";
      break;
    }
    case CursorGameInstance::keybind_face_stunned: {
      text = u"bewildered";
      break;
    }
    case CursorGameInstance::keybind_ui_toAll: {
      text = u"to all";
      break;
    }
    case CursorGameInstance::keybind_ui_toParty: {
      text = u"to party";
      break;
    }
    case CursorGameInstance::keybind_ui_quest: {
      text = u"quest";
      break;
    }
    case CursorGameInstance::keybind_ui_worldMap: {
      text = u"worldMap";
      break;
    }
    case CursorGameInstance::keybind_ui_equip: {
      text = u"equip";
      break;
    }
    case CursorGameInstance::keybind_ui_package: {
      text = u"item";
      break;
    }
    case CursorGameInstance::keybind_ui_buddy: {
      text = u"buddy";
      break;
    }
    case CursorGameInstance::keybind_ui_party: {
      text = u"party";
      break;
    }
    case CursorGameInstance::keybind_ui_mapleChat: {
      text = u"mapleChat";
      break;
    }
    case CursorGameInstance::keybind_ui_quickSlot: {
      text = u"quickSlot";
      break;
    }
    case CursorGameInstance::keybind_ui_keyBind: {
      text = u"keyBind";
      break;
    }
    case CursorGameInstance::keybind_ui_characterStat: {
      text = u"characterStat";
      break;
    }
    case CursorGameInstance::keybind_ui_whisper: {
      text = u"whisper";
      break;
    }
    case CursorGameInstance::keybind_ui_skill: {
      text = u"skill";
      break;
    }
    case CursorGameInstance::keybind_ui_craft: {
      text = u"craft";
      break;
    }
    case CursorGameInstance::keybind_ui_mailBox: {
      text = u"mailBox";
      break;
    }
    case CursorGameInstance::keybind_ui_chatPlus: {
      text = u"chat+";
      break;
    }
    case CursorGameInstance::keybind_ui_miniMap: {
      text = u"miniMap";
      break;
    }
    case CursorGameInstance::keybind_ui_changeChannel: {
      text = u"changeChannel";
      break;
    }
    case CursorGameInstance::keybind_ui_ok: {
      text = u"ok";
      break;
    }
    case CursorGameInstance::keybind_action_pick: {
      text = u"pick";
      break;
    }
    case CursorGameInstance::keybind_action_sit: {
      text = u"sit";
      break;
    }
    case CursorGameInstance::keybind_action_attack: {
      text = u"ok";
      break;
    }
    case CursorGameInstance::keybind_action_jump: {
      text = u"jump";
      break;
    }
    case CursorGameInstance::keybind_action_harvest: {
      text = u"harvest";
      break;
    }
    case CursorGameInstance::keybind_action_up: {
      text = u"up";
      break;
    }
    case CursorGameInstance::keybind_action_down: {
      text = u"down";
      break;
    }
    case CursorGameInstance::keybind_action_left: {
      text = u"left";
      break;
    }
    case CursorGameInstance::keybind_action_right: {
      text = u"right";
      break;
    }
    case CursorGameInstance::keybind_face_vomit: {
      text = u"vomit";
      break;
    }
    }
    auto &mouse_pos = Window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    TooltipUiSystem::render_str(text, show_pos.x, show_pos.y);
  } else if (backgrnd_key.has_value() &&
             KeyboardGameInstance::data.contains(backgrnd_key.value())) {
    auto scan_code = backgrnd_key.value();
    auto input = KeyboardGameInstance::data.at(scan_code);
    auto &mouse_pos = Window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    if (input.type == "skill") {
      auto ski_id = input.val;
      std::u16string ski_id2{ski_id.begin(), ski_id.end()};
      auto ski_lv = JobSkillGameInstance::load_ski_level(ski_id2);
      TooltipUiSystem::render_skill(ski_id2, ski_lv, show_pos.x, show_pos.y);
    } else if (input.type == "item") {
      auto item_id = input.val;
      GameItem item;
      item.id = {item_id.begin(), item_id.end()};
      TooltipUiSystem::render_item(item, show_pos.x, show_pos.y);
    } else {
    }
  }
  return true;
}

void KeybindingUiSystem::event_motion(SDL_Event *event) {
  auto &sys = System::render_systems;
  std::erase(sys, render_info);
  auto it = std::ranges::find(sys, &CursorRenderSystem::render);
  if (it != sys.end()) {
    sys.insert(it, render_info);
  }
}