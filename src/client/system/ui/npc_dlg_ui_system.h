#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_NPC_DLG_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_NPC_DLG_UI_SYSTEM_H_

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <flat_map>
#include <functional>
#include <string>

namespace sdlms {

class NpcDlgUiSystem {
 private:
  static inline int8_t select_mouse_ = -1;
  static void ResetPos();

  static void RenderBackgrnd();
  static void RenderNpc();
  static void RenderButton();
  static void RenderText();
  static void RenderList();
  static void RenderObtain();

  static SDL_FPoint LoadWh();

  static void EventMotionBtAudio(SDL_Event* Event);

  static void EventList();

  static void EventButtonClose();
  static void EventButtonOk();

  static void EventButtonPrev();
  static void EventButtonNext();

  static void EventButtonQuestYes();
  static void EventButtonQuestNo();

  static bool EventButton(SDL_Event* Event);
  static void EventButtonUpdate();

  static bool StartNextQuest();

 public:
  static bool Render();

  static void Open();
  static void Close();

  static bool Event(SDL_Event* Event);

  static bool CursorIn();

  static void Toggle();

  static inline SDL_FPoint pos_;

  static inline std::u16string npc_id_;
  static inline std::u16string text_;
  static inline std::u16string quest_id_;
  static inline std::u16string quest_index_;
  static inline std::u16string script_id_;

  static inline int8_t index_;
  static inline int8_t max_index_;

  static inline int8_t select_;
  static inline int8_t max_select_;

  static inline std::flat_map<std::u16string, int> act_item_;
  static inline std::flat_map<std::u16string, int> act_random_item_;
  static inline int32_t act_meso_;
  static inline uint32_t act_exp_;

  enum class NpcDlgEnum {
    kQuest,
    kTalk,
    kSelect,

    kQuestAvaliable,

    kQuestStopItem,
    kQuestStopMob,
    kQuestStopLost,
    kQuestStopNpc,

    kQuestYes,
    kQuestNo,

    kQuestComplete,
  };
  static inline NpcDlgEnum type_;

  static inline uint64_t time_;
  static inline std::function<void()> cb_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_NPC_DLG_UI_SYSTEM_H_
