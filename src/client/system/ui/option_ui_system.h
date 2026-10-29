#pragma once

#include "src/client/system/ui/combo_box_ui_system.h"
#include "src/client/system/ui/text_input_ui_system.h"

#include <SDL3/SDL_events.h>

#include <cstdint>
#include <optional>

namespace sdlms {

class OptionUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderButton();
  static void RenderTab();
  static void RenderOption();

  static bool EventButton(SDL_Event* event);
  static void EventButtonOk();

  static void EventTab(SDL_Event* event);

  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);
  static void EventMotionBtAudio(SDL_Event* event);

  static SDL_FPoint LoadWh();

  static inline std::optional<SDL_FPoint> drag_;

 public:
  // sound
  static inline uint8_t master_volume_;
  static inline bool master_volume_mute_;
  static inline uint8_t bgm_volume_;
  static inline bool bgm_volume_mute_;
  static inline uint8_t normal_volume_;
  static inline bool normal_volume_mute_;
  static inline uint8_t monster_volume_;
  static inline bool monster_volume_mute_;
  static inline uint8_t skill_volume_;
  static inline bool skill_volume_mute_;

  // social
  static inline bool whisper_;
  static inline bool chat_invite_;
  static inline bool friend_chat_;
  static inline bool party_invite_;
  static inline bool trade_request_;
  static inline bool friend_request_;
  static inline bool friend_login_;
  static inline bool message_time_;
  static inline uint8_t chatbox_size_;

  // game
  static inline uint8_t hp_warn_;
  static inline uint8_t mp_warn_;
  enum class EnemyInfoEnum {
    kNameHp,
    kName,
    kHp,
  };
  static inline ComboBox enemy_info_cm_ = {
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
  static inline ComboBox screenshot_format_ = {
      .open = false,
      .w = 226,
      .selected = u"JPEG",
      .selects = {u"JPEG", u"PNG"},
      .val = (uint8_t)OptionUiSystem::ScreenshotFormatEnum::kJpeg,
  };
  enum class ScreenshotLocationEnum {
    kFolder,
  };
  static inline ComboBox screenshot_location_ = {
      .open = false,
      .w = 226,
      .selected = u"MapleStory Folder",
      .selects = {u"MapleStory Folder"},
      .val = (uint8_t)OptionUiSystem::ScreenshotLocationEnum::kFolder,
  };

  static inline bool continue_capture_;
  static inline int32_t continue_capture_num_;

  static inline bool auto_capture_;

  static inline bool memory_map_;

  static inline int32_t memory_limit_;

  enum class LanguageEnum {
    kEnglish,
  };
  static inline ComboBox language_ = {
      .open = false,
      .w = 226,
      .selected = u"English",
      .selects = {u"English"},
      .val = (uint8_t)OptionUiSystem::LanguageEnum::kEnglish,
  };

  static inline uint8_t active_tab_;

  static inline SDL_FPoint pos_;
  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();
  static void Toggle();
};

}  // namespace sdlms
