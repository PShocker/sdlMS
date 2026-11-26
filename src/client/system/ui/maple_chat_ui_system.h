#pragma once

#include "src/client/system/ui/component/ui_component.h"
#include "src/common/flatbuffers/common.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

// MapleChat 聊天室（095 CUIMessenger，素材 UI/MapleChat.img）。
// 三态（095 SetState 轮换）：0=Max(342x387) 1=普通(342x291) 2=标题条(186x24)。
// 顶部名牌条（89x19，每页 3 个，prev/next 翻页，"+" 邀请），
// 中间聊天记录（VScr4 滚动），底部输入框 + ENTER/举报按钮，
// 最小化时新消息用 newchat 气泡闪烁提示（095 StartBlink）。
class MapleChatUiSystem : public UiComponentHost<MapleChatUiSystem> {
 public:
  struct Member {
    fbs::PlayerT player;
    std::u16string name;
    bool typing = false;
    uint64_t typing_at = 0;  // 打字状态有效期（超时自动熄灭）
  };

  struct ChatLine {
    std::u16string text;
    int color = 0;  // 0=聊天 1=系统 2=错误 3=成功
  };

  // 095 房间 3 人；本包 MapleChat.img/NameTag 有 0..3 四个名牌，取 4。
  static constexpr size_t kMaxMembers = 4;
  static constexpr int kUsersPerPage = 3;

 private:
  static void RenderBackgrnd();
  static void RenderUsers();
  static void RenderChatLog();
  static void RenderInput();
  static void RenderBlink();

  static bool EventUI(SDL_Event* event);

  static void EventButtonMin();
  static void EventButtonMax();
  static void EventButtonClose();
  static void EventButtonEnter();
  static void EventButtonClaim();
  static void EventButtonInvite();
  static void EventButtonUserPrev();
  static void EventButtonUserNext();

  static void ProcessChat();           // 095 ProcessChat：发消息/斜杠命令
  static void HandleSlashCommand(const std::u16string& text);
  static void SendTyping(bool typing);  // 095 Update 的 "name0/1" 打字状态
  static void SyncInputRects();         // 窗口拖动后同步输入框坐标
  static void DetachWindow();           // 只排 render/event，不发网络

 public:
  static UIComponents& GetComponent();
  static SDL_FPoint LoadWh();
  static bool CursorIn();
  static bool IsOpen();

  // 打开/关闭窗口；Close 会发送离开房间（095 OnDestroy step2）。
  static void Open();
  static void Close();
  // 095 TryNew(0)：未在聊天室时创建并进入新房间，已开时无动作。
  static void TryOpen();
  static void Toggle();

  // 接受邀请后按 SN 进房（095 OnCreate pData=SN）。
  static void JoinRoom(uint64_t sn);
  // 右键玩家聊天邀请：没开窗时先建房（095 TryNew(0)+SendInviteMsg）。
  static void InviteClient(uint64_t client_id);
  // 输入名字邀请（窗口 "+" 按钮）。
  static void InviteName(const std::u16string& name);
  // 离开房间（发 step2 并复位本地状态）。
  static void LeaveRoom();

  static bool Render();
  static bool Event(SDL_Event* event);

  static void AddChat(const std::u16string& text, int color = 0);

  // ---- 服务端 ServerCharacterMessenger 写入（095 SP 143 各分支）----
  // step1：自己进入结果（sn=房间SN，x=slot，players 按下标=slot）。
  static void OnSelfEnter(uint64_t sn, uint8_t slot,
                          std::vector<std::unique_ptr<fbs::PlayerT>>& players);
  // step0：成员进入（095 OnEnter）。
  static void OnMemberEnter(fbs::PlayerT& player, uint8_t slot);
  // step2：成员离开（095 OnLeave）。
  static void OnMemberLeave(fbs::PlayerT* player, uint64_t client_id,
                           uint8_t slot);
  // step6：聊天 / 打字状态（095 OnChat）。
  static void OnChat(fbs::PlayerT* player, const std::u16string& text,
                     uint8_t typing);
  // step1 x=255：不能参与受招待的聊天房（095 OnSelfEnter nIdx<0）。
  static void EnterFailed();

  // ---- 窗口状态 ----
  static inline int state_ = 1;  // 0=Max 1=普通 2=标题条
  static inline SDL_FPoint pos_;
  static inline std::optional<SDL_FPoint> drag_;
  static inline uint64_t room_sn_ = 0;
  static inline int self_slot_ = -1;
  static inline std::vector<Member> members_;
  static inline std::vector<ChatLine> chat_log_;
  static inline uint32_t page_ = 0;      // 聊天记录向上翻的行数
  static inline uint32_t user_page_ = 0;  // 名牌条页码
  static inline TextInput chat_input_{};
  static inline TextInput invite_input_{};
  static inline bool invite_active_ = false;
  static inline std::vector<std::u16string> history_;
  static inline size_t history_idx_ = 0;
  static inline bool unread_ = false;  // 最小化期间有新消息（095 blink）
  static inline bool typing_sent_ = false;
  static inline uint64_t typing_sent_at_ = 0;
};

}  // namespace sdlms
