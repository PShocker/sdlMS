#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/system/ui/combo_box_ui_system.h"
#include "src/client/system/ui/text_input_ui_system.h"
#include <cstdint>
#include <optional>
class OptionUiSystem {
private:
  static void render_backgrnd();
  static void render_button();
  static void render_tab();
  static void render_option();

  static bool event_button(SDL_Event *event);
  static void event_button_ok();

  static void event_tab(SDL_Event *event);

  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);
  static void event_motion_bt_audio(SDL_Event *event);

  static SDL_FPoint load_wh();

  static inline std::optional<SDL_FPoint> drag;

public:
  // sound
  static inline uint8_t master_volume;
  static inline bool master_volume_mute;
  static inline uint8_t bgm_volume;
  static inline bool bgm_volume_mute;
  static inline uint8_t normal_volume;
  static inline bool normal_volume_mute;
  static inline uint8_t monster_volume;
  static inline bool monster_volume_mute;
  static inline uint8_t skill_volume;
  static inline bool skill_volume_mute;

  //social
  static inline bool whisper;
  static inline bool chat_invite;
  static inline bool friend_chat;
  static inline bool party_invite;
  static inline bool trade_request;
  static inline bool friend_request;
  static inline bool friend_login;
  static inline bool message_time;
  static inline uint8_t chatbox_size;


  // game
  static inline uint8_t hp_warn;
  static inline uint8_t mp_warn;
  enum class EnemyInfoEnum {
    kNameHp,
    kName,
    kHp,
  };
  static inline ComboBox enemy_info_cm = {
      .open = false,
      .w = 226,
      .selected = u"Name and HP",
      .selects = {u"Name and HP", u"Name", u"HP"},
      .val = (uint8_t)OptionUiSystem::EnemyInfoEnum::kNameHp,
  };

  enum class ScreenshotFormatEnum {
    kJpeg,
    kPng,
  };
  static inline ComboBox screenshot_format = {
      .open = false,
      .w = 226,
      .selected = u"JPEG",
      .selects = {u"JPEG", u"PNG"},
      .val = (uint8_t)OptionUiSystem::ScreenshotFormatEnum::kJpeg,
  };
  enum class ScreenshotLocationEnum {
    kFolder,
  };
  static inline ComboBox screenshot_location = {
      .open = false,
      .w = 226,
      .selected = u"MapleStory Folder",
      .selects = {u"MapleStory Folder"},
      .val = (uint8_t)OptionUiSystem::ScreenshotLocationEnum::kFolder,
  };

  static inline bool continue_capture;
  static inline int32_t continue_capture_num;

  static inline bool auto_capture;

  static inline bool memory_map;

  static inline int32_t memory_limit;

  enum class LanguageEnum {
    kEnglish,
  };
  static inline ComboBox language = {
      .open = false,
      .w = 226,
      .selected = u"English",
      .selects = {u"English"},
      .val = (uint8_t)OptionUiSystem::LanguageEnum::kEnglish,
  };

  static inline uint8_t active_tab;

  static inline SDL_FPoint pos;
  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();
  static void toggle();
};