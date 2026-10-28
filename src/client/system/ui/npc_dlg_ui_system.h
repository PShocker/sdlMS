#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <flat_map>
#include <functional>
#include <string>
class NpcDlgUiSystem {
private:
  static inline int8_t select_mouse = -1;
  static void ResetPos();

  static void RenderBackgrnd();
  static void RenderNpc();
  static void RenderButton();
  static void RenderText();
  static void RenderList();
  static void RenderObtain();

  static SDL_FPoint LoadWh();

  static void EventMotionBtAudio(SDL_Event *Event);

  static void EventList();

  static void EventButtonClose();
  static void EventButtonOk();

  static void EventButtonPrev();
  static void EventButtonNext();

  static void EventButtonQuestYes();
  static void EventButtonQuestNo();

  static bool EventButton(SDL_Event *Event);
  static void EventButtonUpdate();

  static bool StartNextQuest();

public:
  static bool Render();

  static void Open();
  static void Close();

  static bool Event(SDL_Event *Event);

  static bool CursorIn();

  static void Toggle();

  static inline SDL_FPoint pos;

  static inline std::u16string npc_id;
  static inline std::u16string text;
  static inline std::u16string quest_id;
  static inline std::u16string quest_index;
  static inline std::u16string script_id;

  static inline int8_t index;
  static inline int8_t max_index;

  static inline int8_t select;
  static inline int8_t max_select;

  static inline std::flat_map<std::u16string, int> act_item;
  static inline std::flat_map<std::u16string, int> act_random_item;
  static inline int32_t act_meso;
  static inline uint32_t act_exp;

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
  static inline NpcDlgEnum type;

  static inline uint64_t time;
  static inline std::function<void()> cb;
};