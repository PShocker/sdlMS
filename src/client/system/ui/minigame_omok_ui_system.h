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

class MinigameOmokUiSystem : public UiComponentHost<MinigameOmokUiSystem> {
 public:
  // 五子棋棋盘 15×15（095 COmokDlg）。
  static constexpr int kBoardSize = 15;

  enum class Phase {
    kIdle,     // 未在房间
    kWaiting,  // 只有自己，等待对手
    kReady,    // 两人齐（未开局）
    kPlaying,  // 对局中
    kResult,   // 本局结束，显示结果
  };

 private:
  static void RenderBackgrnd();
  static void RenderPlayers();
  static void RenderStoneBoxes();
  static void RenderStones();
  static void RenderTurn();
  static void RenderResult();
  static void RenderChat();
  static bool EventUI(SDL_Event* event);

  static void EventButtonStart();
  static void EventButtonReady();
  static void EventButtonForfeit();
  static void EventButtonUndo();
  static void EventButtonDraw();
  static void EventButtonLeave();

  static void SendStep(uint8_t step, uint8_t x = 0, uint8_t y = 0);
  static void PutStoneAt(float mx, float my);

 public:
  // 房间窗口几何（窗口内部坐标，0 为窗口左上角）。
  // 全屏版 Minigame.img：棋盘 15×15 交叉点，间距 24，起点 (20,40)。
  static constexpr float kBoardOriginX = 20;
  static constexpr float kBoardOriginY = 40;
  static constexpr float kBoardSpacing = 24;
  // 头像站位（095 GetActionPosition：slot0 (438,141)、slot1 (534,141)）。
  static constexpr float kSlotX[2] = {438, 534};
  static constexpr float kSlotY = 141;

  static UIComponents& GetComponent();

  // 房间状态（ServerMinigameInstance::HandleServer 写入）。
  static inline Phase phase_ = Phase::kIdle;
  static inline int my_slot_ = 0;       // 0=房主(左) 1=客方(右)
  static inline int my_color_ = 0;      // 0=黑 1=白
  static inline int turn_color_ = 0;    // 当前行棋色
  static inline bool has_peer_ = false;
  static inline bool peer_ready_ = false;
  static inline bool my_ready_ = false;  // 自己已按准备（客方）
  static inline bool undo_pending_ = false;
  static inline bool draw_pending_ = false;
  static inline GameCharacter peer_{};
  static inline fbs::PlayerT peer_player_{};
  static inline std::u16string peer_name_;
  // 自己的展示副本（地图上的 self_ 仍被移动系统驱动，房间立绘单独拷一份）。
  static inline GameCharacter show_self_{};
  static inline uint32_t my_win_ = 0;
  static inline uint32_t my_lose_ = 0;
  static inline uint32_t my_draw_ = 0;
  static inline int result_ = -1;          // -1 无 0 我胜 1 我负 2 平局
  static inline int result_winner_color_ = -1;
  static inline uint64_t turn_deadline_ = 0;

  struct StoneHistory {
    uint8_t x;
    uint8_t y;
    uint8_t color;  // 1 黑 2 白
  };
  static inline uint8_t board_[kBoardSize][kBoardSize] = {};
  static inline std::vector<StoneHistory> history_;
  static inline int last_stone_[2] = {-1, -1};
  static inline std::vector<std::u16string> chat_log_;

  static inline SDL_FPoint pos_;
  static inline TextInput chat_input_{};

  // 进房请求（背包道具触发）。
  static void EnterRoomByItem();
  // 开发便利：确保背包里有一张五子棋桌。
  static void EnsureTableItem();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event* event);

  // 把一个角色整理成房间立绘展示形态（静态站立 + 默认表情，
  // 清掉椅子/墓碑/技能/气泡/特效等地图态，按武器切 stand1/stand2）。
  // 服务端同步对手（ServerMinigameInstance::HandleServer case 1）时也要调用。
  static void PrepareShowCharacter(GameCharacter& g);

  // 房间聊天/重置（服务端状态同步时调用）。
  static void AddChat(const std::u16string& line);
  static void ResetRound();

  static SDL_FPoint LoadWh();
  static bool CursorIn();
};

}  // namespace sdlms