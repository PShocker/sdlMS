#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <flat_map>
#include <functional>
#include <string>
#include <vector>

namespace sdlms {

class NpcDlgUiSystem : public UiComponentHost<NpcDlgUiSystem> {
 private:
  static inline int8_t select_mouse_ = -1;
  static void ResetPos();

  static void RenderBackgrnd();
  static void RenderNpc();
  static void RenderText();
  static void RenderList();
  static void RenderObtain();

  static SDL_FPoint LoadWh();

  static void EventMotionBtAudio(SDL_Event* event);

  static void EventList();

  static void EventButtonClose();
  static void EventButtonOk();

  static void EventButtonPrev();
  static void EventButtonNext();

  static void EventButtonYes();
  static void EventButtonNo();

  static void EventButtonQuestYes();
  static void EventButtonQuestNo();

  static bool EventUI(SDL_Event* event);
  static void EventButtonUpdate();

  static bool StartNextQuest();

  // 发放 Act/<index> 的奖励。背包不足时会提示并返回 false。
  static bool SettleAct();


 public:
  static UIComponents& GetComponent();
  static bool Render();

  static void Open();
  static void Close();

  // 脚本（startscript / endscript）接管交任务流程时调用：结算当前任务
  // （quest_id_）的 Act 奖励并标记完成。成功返回 true。
  static bool FinishQuest();

  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();

  static inline SDL_FPoint pos_;

  static inline std::u16string npc_id_;
  static inline std::u16string text_;
  static inline std::u16string quest_id_;
  static inline std::u16string quest_index_;
  static inline std::u16string script_id_;

  // 脚本对话底部按钮布局：
  //   kDefault: 上一页 / 下一页 / OK
  //   kYesNo:   只有 Yes / No（选择确认页）
  //   kOk:      只有 OK，且不允许关闭（不可回退的结果页）
  enum class NpcDlgButton {
    kDefault,
    kYesNo,
    kOk,
  };
  static inline NpcDlgButton buttons_ = NpcDlgButton::kDefault;

  static inline std::function<void()> on_yes_;
  static inline std::function<void()> on_no_;
  static inline std::function<void()> on_ok_;

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
    kScript,

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
