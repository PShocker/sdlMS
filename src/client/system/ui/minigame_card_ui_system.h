#pragma once

#include "src/client/game/game_character.h"
#include "src/client/system/ui/component/ui_component.h"
#include "src/common/flatbuffers/common.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <string>
#include <vector>

namespace sdlms {

// 卡片配对（095 CMemoryGameDlg）：30 张牌（15 对），
// 网格 6 列，起点 (33,39)，卡 49x62，间距 56x67，卡背 back2。
class MinigameCardUiSystem : public UiComponentHost<MinigameCardUiSystem> {
 public:
  static constexpr int kCardCols = 6;
  static constexpr int kCardCount = 30;
  static constexpr float kCardOriginX = 33;
  static constexpr float kCardOriginY = 39;
  static constexpr float kCardGapX = 56;
  static constexpr float kCardGapY = 67;
  static constexpr float kCardW = 49;
  static constexpr float kCardH = 62;

  // 卡面显示状态。
  enum class Face {
    kBack = 0,    // 背面
    kOpen = 1,    // 翻开
    kMatched = 2, // 已配对（保持翻开并标记）
  };

  enum class Phase {
    kIdle,
    kWaiting,
    kReady,
    kPlaying,
    kResult,
  };

 private:
  static void RenderBackgrnd();
  static void RenderPlayers();
  static void RenderCards();
  static void RenderScore();
  static void RenderChat();
  static bool EventUI(SDL_Event* event);

  static void EventButtonStart();
  static void EventButtonReady();
  static void EventButtonForfeit();
  static void EventButtonDraw();
  static void EventButtonLeave();

  static void SendStep(uint8_t step, uint8_t x = 0);
  static int CardAt(float mx, float my);

 public:
  static UIComponents& GetComponent();

  // 房间状态（ServerMinigameInstance::HandleServer 写入）。
  static inline Phase phase_ = Phase::kIdle;
  static inline int my_slot_ = 0;
  static inline bool has_peer_ = false;
  static inline bool peer_ready_ = false;
  static inline bool my_ready_ = false;
  static inline bool my_turn_ = false;
  static inline bool draw_pending_ = false;
  static inline GameCharacter peer_{};
  static inline fbs::PlayerT peer_player_{};
  static inline std::u16string peer_name_;
  static inline GameCharacter show_self_{};
  static inline uint32_t my_win_ = 0;
  static inline uint32_t my_lose_ = 0;
  static inline uint32_t my_draw_ = 0;
  static inline int result_ = -1;    // -1 无 0 我胜 1 我负 2 平局
  // 本局配对数（左=slot0 右=slot1）。
  static inline uint8_t pair_score_[2] = {0, 0};

  static inline uint8_t cards_[kCardCount] = {};  // 牌面值 0-14
  static inline Face face_[kCardCount] = {};      // 显示状态
  // 我方已点张数（点击两张后等服务器判定）。
  static inline int picked_ = 0;
  // 配对失败的两张：到时翻回（095 TurnUpCard_Back 延时翻回）。
  static inline int flip_back_[2] = {-1, -1};
  static inline uint64_t flip_back_at_ = 0;
  // 开局预览：全部正面展示一段时间（095 m_bShowAndHide 简化版）。
  static inline uint64_t preview_until_ = 0;
  static inline uint64_t turn_deadline_ = 0;
  // 配对成功特效：卡索引 → 开始时刻（095 DrawEffect：effect2 发光边框放大出现）。
  static inline uint64_t match_fx_at_[kCardCount] = {};

  static inline std::vector<std::u16string> chat_log_;
  static inline SDL_FPoint pos_;
  static inline TextInput chat_input_{};

  // 进房请求（背包使用 Match Cards 道具）。
  static void EnterRoomByItem();
  // 开发便利：确保背包里有一套 Match Cards。
  static void EnsureItem();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event* event);

  static void AddChat(const std::u16string& line);
  static void ResetRound();

  static SDL_FPoint LoadWh();
  static bool CursorIn();
};

}  // namespace sdlms