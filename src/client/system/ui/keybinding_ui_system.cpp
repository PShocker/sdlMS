#include "src/client/system/ui/keybinding_ui_system.h"
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
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
namespace sdlms {


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

SDL_FPoint KeybindingUiSystem::LoadWh() { return {636, 445}; }

const static SDL_FPoint key_lt{5, 56};

static std::optional<SDL_Scancode> backgrnd_key;

void KeybindingUiSystem::RenderBackgrndKey() {
  auto keys = WzResource::ui_->find(u"KeyBindings.img/key");
  static const std::flat_map<SDL_Scancode, SDL_Texture *> key_textures = {
      {SDL_SCANCODE_ESCAPE, WzResource::LoadTexture(keys->get_child(u"1"))},
      {SDL_SCANCODE_1, WzResource::LoadTexture(keys->get_child(u"2"))},
      {SDL_SCANCODE_2, WzResource::LoadTexture(keys->get_child(u"3"))},
      {SDL_SCANCODE_3, WzResource::LoadTexture(keys->get_child(u"4"))},
      {SDL_SCANCODE_4, WzResource::LoadTexture(keys->get_child(u"5"))},
      {SDL_SCANCODE_5, WzResource::LoadTexture(keys->get_child(u"6"))},
      {SDL_SCANCODE_6, WzResource::LoadTexture(keys->get_child(u"7"))},
      {SDL_SCANCODE_7, WzResource::LoadTexture(keys->get_child(u"8"))},
      {SDL_SCANCODE_8, WzResource::LoadTexture(keys->get_child(u"9"))},
      {SDL_SCANCODE_9, WzResource::LoadTexture(keys->get_child(u"10"))},
      {SDL_SCANCODE_0, WzResource::LoadTexture(keys->get_child(u"11"))},
      {SDL_SCANCODE_MINUS, WzResource::LoadTexture(keys->get_child(u"12"))},
      {SDL_SCANCODE_EQUALS, WzResource::LoadTexture(keys->get_child(u"13"))},
      {SDL_SCANCODE_Q, WzResource::LoadTexture(keys->get_child(u"16"))},
      {SDL_SCANCODE_W, WzResource::LoadTexture(keys->get_child(u"17"))},
      {SDL_SCANCODE_E, WzResource::LoadTexture(keys->get_child(u"18"))},
      {SDL_SCANCODE_R, WzResource::LoadTexture(keys->get_child(u"19"))},
      {SDL_SCANCODE_T, WzResource::LoadTexture(keys->get_child(u"20"))},
      {SDL_SCANCODE_Y, WzResource::LoadTexture(keys->get_child(u"21"))},
      {SDL_SCANCODE_U, WzResource::LoadTexture(keys->get_child(u"22"))},
      {SDL_SCANCODE_I, WzResource::LoadTexture(keys->get_child(u"23"))},
      {SDL_SCANCODE_O, WzResource::LoadTexture(keys->get_child(u"24"))},
      {SDL_SCANCODE_P, WzResource::LoadTexture(keys->get_child(u"25"))},
      {SDL_SCANCODE_LEFTBRACKET,
       WzResource::LoadTexture(keys->get_child(u"26"))},
      {SDL_SCANCODE_RIGHTBRACKET,
       WzResource::LoadTexture(keys->get_child(u"27"))},
      {SDL_SCANCODE_LCTRL, WzResource::LoadTexture(keys->get_child(u"29"))},
      {SDL_SCANCODE_RCTRL, WzResource::LoadTexture(keys->get_child(u"29"))},
      {SDL_SCANCODE_A, WzResource::LoadTexture(keys->get_child(u"30"))},
      {SDL_SCANCODE_S, WzResource::LoadTexture(keys->get_child(u"31"))},
      {SDL_SCANCODE_D, WzResource::LoadTexture(keys->get_child(u"32"))},
      {SDL_SCANCODE_F, WzResource::LoadTexture(keys->get_child(u"33"))},
      {SDL_SCANCODE_G, WzResource::LoadTexture(keys->get_child(u"34"))},
      {SDL_SCANCODE_H, WzResource::LoadTexture(keys->get_child(u"35"))},
      {SDL_SCANCODE_J, WzResource::LoadTexture(keys->get_child(u"36"))},
      {SDL_SCANCODE_K, WzResource::LoadTexture(keys->get_child(u"37"))},
      {SDL_SCANCODE_L, WzResource::LoadTexture(keys->get_child(u"38"))},
      {SDL_SCANCODE_SEMICOLON,
       WzResource::LoadTexture(keys->get_child(u"39"))},
      {SDL_SCANCODE_APOSTROPHE,
       WzResource::LoadTexture(keys->get_child(u"40"))},
      {SDL_SCANCODE_GRAVE, WzResource::LoadTexture(keys->get_child(u"41"))},
      {SDL_SCANCODE_LSHIFT, WzResource::LoadTexture(keys->get_child(u"42"))},
      {SDL_SCANCODE_RSHIFT, WzResource::LoadTexture(keys->get_child(u"42"))},
      {SDL_SCANCODE_BACKSLASH,
       WzResource::LoadTexture(keys->get_child(u"43"))},
      {SDL_SCANCODE_Z, WzResource::LoadTexture(keys->get_child(u"44"))},
      {SDL_SCANCODE_X, WzResource::LoadTexture(keys->get_child(u"45"))},
      {SDL_SCANCODE_C, WzResource::LoadTexture(keys->get_child(u"46"))},
      {SDL_SCANCODE_V, WzResource::LoadTexture(keys->get_child(u"47"))},
      {SDL_SCANCODE_B, WzResource::LoadTexture(keys->get_child(u"48"))},
      {SDL_SCANCODE_N, WzResource::LoadTexture(keys->get_child(u"49"))},
      {SDL_SCANCODE_M, WzResource::LoadTexture(keys->get_child(u"50"))},
      {SDL_SCANCODE_COMMA, WzResource::LoadTexture(keys->get_child(u"51"))},
      {SDL_SCANCODE_PERIOD, WzResource::LoadTexture(keys->get_child(u"52"))},
      {SDL_SCANCODE_LALT, WzResource::LoadTexture(keys->get_child(u"56"))},
      {SDL_SCANCODE_RALT, WzResource::LoadTexture(keys->get_child(u"56"))},
      {SDL_SCANCODE_SPACE, WzResource::LoadTexture(keys->get_child(u"57"))},
      {SDL_SCANCODE_F1, WzResource::LoadTexture(keys->get_child(u"59"))},
      {SDL_SCANCODE_F2, WzResource::LoadTexture(keys->get_child(u"60"))},
      {SDL_SCANCODE_F3, WzResource::LoadTexture(keys->get_child(u"61"))},
      {SDL_SCANCODE_F4, WzResource::LoadTexture(keys->get_child(u"62"))},
      {SDL_SCANCODE_F5, WzResource::LoadTexture(keys->get_child(u"63"))},
      {SDL_SCANCODE_F6, WzResource::LoadTexture(keys->get_child(u"64"))},
      {SDL_SCANCODE_F7, WzResource::LoadTexture(keys->get_child(u"65"))},
      {SDL_SCANCODE_F8, WzResource::LoadTexture(keys->get_child(u"66"))},
      {SDL_SCANCODE_F9, WzResource::LoadTexture(keys->get_child(u"67"))},
      {SDL_SCANCODE_F10, WzResource::LoadTexture(keys->get_child(u"68"))},
      {SDL_SCANCODE_F11, WzResource::LoadTexture(keys->get_child(u"87"))},
      {SDL_SCANCODE_F12, WzResource::LoadTexture(keys->get_child(u"88"))},

      // 以下特殊按键的映射可能需要根据实际 SDL3 定义调整
      {SDL_SCANCODE_HOME,
       WzResource::LoadTexture(keys->get_child(u"71"))}, // Hm
      {SDL_SCANCODE_PAGEUP,
       WzResource::LoadTexture(keys->get_child(u"73"))}, // Pup
      {SDL_SCANCODE_END,
       WzResource::LoadTexture(keys->get_child(u"79"))}, // End
      {SDL_SCANCODE_PAGEDOWN,
       WzResource::LoadTexture(keys->get_child(u"81"))}, // Pdn
      {SDL_SCANCODE_INSERT,
       WzResource::LoadTexture(keys->get_child(u"82"))}, // Ins
      {SDL_SCANCODE_DELETE,
       WzResource::LoadTexture(keys->get_child(u"83"))}, // Del

      {SDL_SCANCODE_UP, WzResource::LoadTexture(keys->get_child(u"72"))},
      {SDL_SCANCODE_DOWN, WzResource::LoadTexture(keys->get_child(u"80"))},
      {SDL_SCANCODE_LEFT, WzResource::LoadTexture(keys->get_child(u"75"))},
      {SDL_SCANCODE_RIGHT, WzResource::LoadTexture(keys->get_child(u"77"))},

  };
  backgrnd_key = std::nullopt;
  auto &mouse_pos = Window::mouse_pos_;
  for (auto [k, v] : key_textures) {
    if (!mapped_scancode_position.contains(k)) {
      continue;
    }
    auto key_pos = mapped_scancode_position.at(k);
    // render icon
    int x = pos_.x + key_pos.x + key_lt.x;
    int y = pos_.y + key_pos.y + key_lt.y;
    SDL_FRect pp{static_cast<float>(x), static_cast<float>(y), 32, 32};
    if (SDL_PointInRectFloat(&mouse_pos, &pp)) {
      backgrnd_key = k;
    }
    if (KeyboardGameInstance::data_.contains(k)) {
      const auto &input = KeyboardGameInstance::data_.at(k);
      QuickSlotUiSystem::RenderInputIcon(input, x - 1, y - 1);
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
    SDL_RenderTexture(Window::renderer_, v, nullptr, &pos_rect);
  }
}

void KeybindingUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"KeyBindings.img/backgrnd"));
  SDL_FRect pos_rect{pos_.x, pos_.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
  return;
}

void KeybindingUiSystem::RenderTab() {
  static auto node = WzResource::ui_->find(u"KeyBindings.img/tab:kind");
  const SDL_FPoint lt{9, 24};
  for (auto i : {0, 1}) {
    SDL_Texture *texture;
    if (active_tab_ == i) {
      texture = WzResource::LoadTexture(
          node->get_child(u"selected")->get_child(std::to_string(i)));
    } else {
      texture = WzResource::LoadTexture(
          node->get_child(u"normal")->get_child(std::to_string(i)));
    }
    SDL_FRect pos_rect = {
        int(pos_.x) + lt.x + texture->w * i,
        int(pos_.y) + lt.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
}

void KeybindingUiSystem::RenderBackgrnd2() {
  static auto backgrnd2 = WzResource::LoadTexture(
      WzResource::ui_->find(u"KeyBindings.img/backgrnd_keyboard"));
  SDL_FPoint lt{9, 53};
  SDL_FRect pos_rect{
      pos_.x + lt.x,
      pos_.y + lt.y,
      static_cast<float>(backgrnd2->w),
      static_cast<float>(backgrnd2->h),
  };
  SDL_RenderTexture(Window::renderer_, backgrnd2, nullptr, &pos_rect);
  return;
}

void KeybindingUiSystem::RenderButton() {
  const static std::array buttons_nodes = {
      WzResource::ui_->find(u"Basic.img/BtClose"),
      WzResource::ui_->find(u"KeyBindings.img/button:Default"),
      WzResource::ui_->find(u"KeyBindings.img/button:Delete"),
      WzResource::ui_->find(u"KeyBindings.img/button:quickslots"),

      WzResource::ui_->find(u"KeyBindings.img/button:presetPage1"),
      WzResource::ui_->find(u"KeyBindings.img/button:presetPage2"),
      WzResource::ui_->find(u"KeyBindings.img/button:presetPage3"),
      WzResource::ui_->find(u"KeyBindings.img/button:presetSave"),
      WzResource::ui_->find(u"KeyBindings.img/button:presetUndo"),
  };
  auto wh = LoadWh();
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
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos_;
    // 判断按钮是否被遮挡
    auto CursorIn = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && CursorIn == Render &&
        !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::LoadTexture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer_, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::LoadTexture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer_, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::LoadTexture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer_, normal, nullptr, &pos_rect);
    }
  }
}

void KeybindingUiSystem::RenderRemainIcon(int i, int x, int y,
                                              uint8_t alpha) {
  auto input = KeyboardGameInstance::FindInputByCursor(i);
  QuickSlotUiSystem::RenderInputIcon(input, x, y, alpha);
}

static std::optional<uint32_t> remain_key;
static SDL_FPoint remain_lt{12, 300};

void KeybindingUiSystem::RenderRemainKey() {
  remain_key = std::nullopt;

  const static std::flat_map<CursorGameInstance::CursorKeybindType, SDL_FPoint> m = {
      {CursorGameInstance::CursorKeybindType::kKeybindUiMenu, SDL_FPoint{0, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindFaceHit, SDL_FPoint{34, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindFaceSmile, SDL_FPoint{68, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindFaceTroubled, SDL_FPoint{102, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindFaceCry, SDL_FPoint{136, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindFaceAngry, SDL_FPoint{170, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindFaceBewildered, SDL_FPoint{204, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindFaceStunned, SDL_FPoint{238, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiToAll, SDL_FPoint{272, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiToParty, SDL_FPoint{306, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiQuest, SDL_FPoint{340, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiWorldMap, SDL_FPoint{374, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiEquip, SDL_FPoint{408, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiPackage, SDL_FPoint{442, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiBuddy, SDL_FPoint{476, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiParty, SDL_FPoint{510, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiMapleChat, SDL_FPoint{544, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiQuickSlot, SDL_FPoint{578, 0}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiKeyBind, SDL_FPoint{0, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiCharacterStat, SDL_FPoint{34, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiWhisper, SDL_FPoint{68, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiSkill, SDL_FPoint{102, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiCraft, SDL_FPoint{136, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiMailBox, SDL_FPoint{170, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiChatPlus, SDL_FPoint{204, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiMiniMap, SDL_FPoint{238, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiChangeChannel, SDL_FPoint{272, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindUiOk, SDL_FPoint{306, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindActionPick, SDL_FPoint{340, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindActionSit, SDL_FPoint{374, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindActionAttack, SDL_FPoint{408, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindActionJump, SDL_FPoint{442, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindActionHarvest, SDL_FPoint{476, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindActionUp, SDL_FPoint{510, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindActionDown, SDL_FPoint{544, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindActionLeft, SDL_FPoint{578, 34}},
      {CursorGameInstance::CursorKeybindType::kKeybindActionRight, SDL_FPoint{0, 68}},
  };
  const auto &key_data = KeyboardGameInstance::data_;
  auto &mouse_pos = Window::mouse_pos_;
  for (auto [k, v] : m) {
    auto input =
        KeyboardGameInstance::FindInputByCursor(static_cast<int>(k));
    if (KeyboardGameInstance::FindKeyByVal(input.val)) {
      continue;
    }
    SDL_FRect pos_rect;
    pos_rect.x = (int)pos_.x + remain_lt.x + v.x;
    pos_rect.y = (int)pos_.y + remain_lt.y + v.y;
    pos_rect.w = 32;
    pos_rect.h = 32;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      remain_key = static_cast<uint32_t>(k);
    }
    RenderRemainIcon(static_cast<int>(k), pos_rect.x, pos_rect.y);
  }
}

bool KeybindingUiSystem::Render() {
  RenderBackgrnd();
  RenderBackgrnd2();
  RenderButton();
  RenderTab();
  RenderBackgrndKey();
  RenderRemainKey();

  return true;
}

void KeybindingUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto &camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    data_bak = KeyboardGameInstance::data_;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void KeybindingUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::render_systems_, RenderInfo);
  std::erase(System::event_systems_, Event);

  EventDragEnd();
}

void KeybindingUiSystem::EventButtonClose() {
  Close();
  KeyboardGameInstance::data_ = data_bak;
  return;
}

void KeybindingUiSystem::EventButtonDefault() {
  KeyboardGameInstance::Reset();
  return;
}

void KeybindingUiSystem::EventButtonDelete() {
  KeyboardGameInstance::data_.clear();
  return;
}

void KeybindingUiSystem::EventButtonQuickSlot() { return; }

void KeybindingUiSystem::EventButtonPrePage1() { return; }

void KeybindingUiSystem::EventButtonPrePage2() { return; }

void KeybindingUiSystem::EventButtonPrePage3() { return; }

void KeybindingUiSystem::EventButtonSave() {
  Close();
  return;
}

void KeybindingUiSystem::EventButtonUndo() {
  EventButtonClose();
  return;
}

void KeybindingUiSystem::EventMotionBtAudio(SDL_Event *Event) {
  if (CursorGameInstance::cursor_ui != Render) {
    return;
  }
  auto wh = LoadWh();
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
  auto &mouse_pos = Window::mouse_pos_;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos_.x;
    pos_rect.y += (int)pos_.y;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      bt_in = i;
      break;
    }
  }
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

void KeybindingUiSystem::EventBackgrndKey() {
  if (CursorGameInstance::cursor_hand_net_.has_value()) {
    return;
  }
  if (!backgrnd_key.has_value()) {
    return;
  }
  auto scan_code = backgrnd_key.value();
  auto &cursor_hand = CursorGameInstance::cursor_hand_;
  if (cursor_hand.has_value()) {
    auto input = KeyboardGameInstance::LoadCursorInput();
    KeyboardGameInstance::InsertInput(scan_code, input.value());
    CursorGameInstance::cursor_hand_ = std::nullopt;
    AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
  } else {
    if (KeyboardGameInstance::data_.contains(scan_code)) {
      CursorGameInstance::cursor_hand_ = {
          .type = CursorGameInstance::CursorHandType::kKeybind,
          .val = scan_code,
      };
      AudioGameInstance::LoadAudio(u"UI.img/DragStart", 0);
    }
  }
}

void KeybindingUiSystem::EventRemainKey() {
  if (CursorGameInstance::cursor_hand_net_.has_value()) {
    return;
  }
  const auto b_w = 1;
  const auto b_h = 1;
  SDL_FRect pos_rect;
  pos_rect.x = pos_.x + remain_lt.x;
  pos_rect.y = pos_.y + remain_lt.y;
  pos_rect.w = b_w;
  pos_rect.h = b_h;
  auto &mouse_pos = Window::mouse_pos_;
  if (CursorGameInstance::cursor_hand_.has_value()) {
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      CursorGameInstance::cursor_hand_ = std::nullopt;
      AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
      return;
    }
  }

  if (remain_key.has_value()) {
    CursorGameInstance::cursor_hand_ = {
        .type = CursorGameInstance::CursorHandType::kKeybind,
        .val = SDL_SCANCODE_UNKNOWN,
        .sub_val = remain_key.value(),
    };
    AudioGameInstance::LoadAudio(u"UI.img/DragStart", 0);
  }
}

bool KeybindingUiSystem::EventButton(SDL_Event *Event) {
  std::vector<void (*)()> fns;
  auto wh = LoadWh();
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
      EventButtonClose,     EventButtonDefault,  EventButtonDelete,
      EventButtonQuickSlot, EventButtonPrePage1, EventButtonPrePage2,
      EventButtonPrePage3,  EventButtonSave,     EventButtonUndo,
  };

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      fns[i]();
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }

  return false;
}

bool KeybindingUiSystem::Event(SDL_Event *Event) {
  bool r = true;
  switch (Event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = Event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      EventButtonClose();
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
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
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        EventTop();
        EventDragStart(Event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        // event_tab(event);
        EventBackgrndKey();
        EventRemainKey();
        r = !EventButton(Event);
      }
      EventDragEnd();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    EventMotion(Event);
    EventDragMove(Event);
    EventMotionBtAudio(Event);
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

void KeybindingUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void KeybindingUiSystem::EventDragStart(SDL_Event *Event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos = {Event->button.x, Event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - Event->button.x, pos_.y - Event->button.y};
  }
  return;
}

void KeybindingUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

void KeybindingUiSystem::EventDragMove(SDL_Event *Event) {
  if (drag_.has_value()) {
    pos_ = {Event->motion.x + drag_->x, Event->motion.y + drag_->y};
    auto &camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  }
  return;
}

void KeybindingUiSystem::Toggle() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool KeybindingUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto &mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool KeybindingUiSystem::RenderInfo() {
  if (CursorGameInstance::modal_overlay) {
    return true;
  }
  if (CursorGameInstance::cursor_ui != Render) {
    return true;
  }
  if (CursorGameInstance::cursor_hand_.has_value()) {
    return true;
  }
  if (remain_key.has_value()) {
    std::u16string text;
    switch ((CursorGameInstance::CursorKeybindType)remain_key.value()) {
    case CursorGameInstance::CursorKeybindType::kKeybindUiMenu: {
      text = u"menu";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceHit: {
      text = u"hit";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceSmile: {
      text = u"smile";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceTroubled: {
      text = u"troubled";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceCry: {
      text = u"menu";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceAngry: {
      text = u"angry";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceBewildered: {
      text = u"bewildered";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceStunned: {
      text = u"bewildered";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiToAll: {
      text = u"to all";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiToParty: {
      text = u"to party";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiQuest: {
      text = u"quest";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiWorldMap: {
      text = u"worldMap";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiEquip: {
      text = u"equip";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiPackage: {
      text = u"item";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiBuddy: {
      text = u"buddy";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiParty: {
      text = u"party";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiMapleChat: {
      text = u"mapleChat";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiQuickSlot: {
      text = u"quickSlot";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiKeyBind: {
      text = u"keyBind";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiCharacterStat: {
      text = u"characterStat";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiWhisper: {
      text = u"whisper";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiSkill: {
      text = u"skill";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiCraft: {
      text = u"craft";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiMailBox: {
      text = u"mailBox";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiChatPlus: {
      text = u"chat+";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiMiniMap: {
      text = u"miniMap";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiChangeChannel: {
      text = u"changeChannel";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiOk: {
      text = u"ok";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionPick: {
      text = u"pick";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionSit: {
      text = u"sit";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionAttack: {
      text = u"ok";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionJump: {
      text = u"jump";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionHarvest: {
      text = u"harvest";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionUp: {
      text = u"up";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionDown: {
      text = u"down";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionLeft: {
      text = u"left";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionRight: {
      text = u"right";
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceVomit: {
      text = u"vomit";
      break;
    }
    }
    auto &mouse_pos = Window::mouse_pos_;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    TooltipUiSystem::RenderStr(text, show_pos.x, show_pos.y);
  } else if (backgrnd_key.has_value() &&
             KeyboardGameInstance::data_.contains(backgrnd_key.value())) {
    auto scan_code = backgrnd_key.value();
    auto input = KeyboardGameInstance::data_.at(scan_code);
    auto &mouse_pos = Window::mouse_pos_;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    if (input.type == "skill") {
      auto ski_id = input.val;
      std::u16string ski_id2{ski_id.begin(), ski_id.end()};
      auto ski_lv = JobSkillGameInstance::LoadSkiLevel(ski_id2);
      TooltipUiSystem::RenderSkill(ski_id2, ski_lv, show_pos.x, show_pos.y);
    } else if (input.type == "item") {
      auto item_id = input.val;
      GameItem item;
      item.id = {item_id.begin(), item_id.end()};
      TooltipUiSystem::RenderItem(item, show_pos.x, show_pos.y);
    } else {
    }
  }
  return true;
}

void KeybindingUiSystem::EventMotion(SDL_Event *Event) {
  auto &sys = System::render_systems_;
  std::erase(sys, RenderInfo);
  auto it = std::ranges::find(sys, &CursorRenderSystem::Render);
  if (it != sys.end()) {
    sys.insert(it, RenderInfo);
  }
}

}  // namespace sdlms
