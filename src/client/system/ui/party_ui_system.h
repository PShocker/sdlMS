#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <optional>
#include <string>
#include <vector>

namespace sdlms {

// 组队信息窗口（095 CUIPartyHP）：左上常驻，成员名 + 等级 + HP 条。
class PartyUiSystem {
 private:
  static void RenderBackgrnd(float x, float y, float w, float h);
  static void RenderMembers();

  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static void EventButtonLeave();

  static SDL_FPoint LoadWh();

  static inline std::optional<SDL_FPoint> drag_;

 public:
  static inline SDL_FPoint pos_;
  // 组队系统消息（095 CHATLOG_ADD 的组队提示）。
  static inline std::vector<std::u16string> chat_log_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms