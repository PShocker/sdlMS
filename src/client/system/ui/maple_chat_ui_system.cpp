#include "src/client/system/ui/maple_chat_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/scroll.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>

#include <algorithm>
#include <memory>
#include <string>

namespace sdlms {

namespace {

// 布局取自 MapleChat.img/MapleChat 布局节点（负 origin 换算为窗口内坐标）。
constexpr float kWinW = 342;
constexpr float kMaxH = 387;   // MapleChat_Max/backgrnd
constexpr float kNormH = 291;  // MapleChat/backgrnd
constexpr float kMinW = 186;   // MapleChat_Min/backgrnd
constexpr float kMinH = 24;
constexpr float kDelta = 96;  // Max 相对普通态的底部下移量

// 标题栏按钮（Basic.img BtMin/BtMax/BtClose 13x13）。
constexpr SDL_FRect kBtnMin = {297, 6, 13, 13};
constexpr SDL_FRect kBtnMax = {310, 6, 13, 13};
constexpr SDL_FRect kBtnClose = {323, 6, 13, 13};

// 名牌条（89x19，间距 91，每页 3 个）。
constexpr float kUserX = 24;
constexpr float kUserY = 28;
constexpr float kUserStep = 91;
constexpr SDL_FRect kBtnUserPrev = {9, 28, 11, 19};
constexpr SDL_FRect kBtnUserNext = {299, 28, 11, 19};
constexpr SDL_FRect kBtnInvite = {316, 29, 18, 18};

// 聊天记录区（chatlog_lt/rb）与滚动条（scroll:chatScroll）。
constexpr float kLogX = 7;
constexpr float kLogY = 55;
constexpr float kLogW = 308;
constexpr float kLogHNorm = 198;  // 12 行
constexpr float kLogHMax = 294;   // 18 行
constexpr float kScrollX = 321;
constexpr float kScrollY = 53;
constexpr int kScrollLenNorm = 203;
constexpr int kScrollLenMax = 299;
constexpr float kLineH = 16;
constexpr int kChatMaxLines = 200;

// 底部输入区（edit:chatinput + layer:textbox + ENTER/举报按钮）。
constexpr SDL_FRect kTextbox = {6, 267, 267, 19};
constexpr SDL_FRect kBtnEnter = {275, 268, 39, 17};
constexpr SDL_FRect kBtnClaim = {316, 267, 20, 19};

// 最小化标题条：newchat 气泡 + 还原/关闭。
constexpr SDL_FRect kNewchat = {72, 7, 12, 10};
constexpr SDL_FRect kMinBtnMax = {140, 5, 13, 13};
constexpr SDL_FRect kMinBtnClose = {166, 5, 13, 13};

// 行颜色分类。
enum LineColor {
  kChat = 0,
  kSystem = 1,
  kError = 2,
  kNotice = 3,
};

std::u16string U16FromNum(uint32_t num) {
  auto s = std::to_string(num);
  return {s.begin(), s.end()};
}

std::u16string SelfName() {
  return CharacterGameInstance::self_.name;
}

// 折行后的行列表（与显示口径一致）。
std::vector<std::u16string> WrapLines(const std::u16string& text,
                                      float max_w) {
  Freetype::LoadSize(12);
  std::vector<std::u16string> lines;
  std::u16string line;
  std::u16string indent;
  float width = 0;
  for (auto c : text) {
    const auto cw = Freetype::LoadW(std::u16string(1, c));
    if (!line.empty() && width + cw > max_w) {
      lines.push_back(line);
      line = indent;
      width = Freetype::LoadW(indent);
    }
    if (lines.empty() && line.empty()) {
      // 首行后的续行统一缩进 4 空格（095 AddChatText）。
      indent = u"    ";
    }
    line.push_back(c);
    width += cw;
  }
  lines.push_back(line);
  return lines;
}

// 聊天记录折行后的总行数。
int TotalLogLines() {
  int total = 0;
  for (const auto& l : MapleChatUiSystem::chat_log_) {
    total += static_cast<int>(WrapLines(l.text, kLogW - 12).size());
  }
  return total;
}

int VisibleLogLines(bool is_max) {
  return static_cast<int>((is_max ? kLogHMax : kLogHNorm) / kLineH);
}

}  // namespace

SDL_FPoint MapleChatUiSystem::LoadWh() {
  switch (state_) {
    case 0:
      return {kWinW, kMaxH};
    case 2:
      return {kMinW, kMinH};
    default:
      return {kWinW, kNormH};
  }
}

bool MapleChatUiSystem::IsOpen() {
  return std::ranges::contains(System::render_systems_, &Render);
}

bool MapleChatUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect);
}

void MapleChatUiSystem::SyncInputRects() {
  const float dy = state_ == 0 ? kDelta : 0;
  chat_input_.r = SDL_Rect{
      static_cast<int>(pos_.x + 9),
      static_cast<int>(pos_.y + 270 + dy),
      258,
      12,
  };
  invite_input_.r = SDL_Rect{
      static_cast<int>(pos_.x + 90),
      static_cast<int>(pos_.y + 51),
      202,
      14,
  };
}

void MapleChatUiSystem::Open() {
  auto it = std::ranges::find(System::render_systems_, &Render);
  if (it != System::render_systems_.end()) {
    return;
  }
  it = std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it == System::render_systems_.end()) {
    return;
  }
  auto& camera = CameraGameInstance::camera_;
  auto [w, h] = LoadWh();
  pos_.x = static_cast<float>((camera.w - w) / 2);
  pos_.y = static_cast<float>((camera.h - h) / 2);

  chat_input_ = TextInput{
      .max_size = 70,
      .text = u"",
      .composition = u"",
      .disable = false,
      .active = false,
      .r = SDL_Rect{0, 0, 258, 12},
      .pos = {0, 0},
      .font_color = {0, 0, 0, 255},
      .cur_color = {0, 0, 0, 255},
      .font_size = 12,
  };
  chat_input_.type.set(TextInput::kIme);
  invite_input_ = TextInput{
      .max_size = 24,
      .text = u"",
      .composition = u"",
      .disable = false,
      .active = false,
      .r = SDL_Rect{0, 0, 202, 14},
      .pos = {0, 0},
      .font_color = {0, 0, 0, 255},
      .cur_color = {0, 0, 0, 255},
      .font_size = 11,
  };
  invite_input_.type.set(TextInput::kIme);
  SyncInputRects();

  System::render_systems_.insert(it, Render);
  System::event_systems_.insert(System::event_systems_.begin(), Event);
}

void MapleChatUiSystem::DetachWindow() {
  chat_input_.Close();
  invite_input_.Close();
  invite_active_ = false;
  drag_ = std::nullopt;
  if (std::ranges::find(System::event_systems_, &KeyboardInputSystem::Event) ==
      System::event_systems_.end()) {
    System::event_systems_.push_back(KeyboardInputSystem::Event);
  }
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void MapleChatUiSystem::Close() {
  // 095 OnDestroy：关窗即离开房间（CP 143 step2）。
  LeaveRoom();
  DetachWindow();
}

void MapleChatUiSystem::LeaveRoom() {
  if (room_sn_ != 0) {
    fbs::ClientCharacterMessengerT m;
    m.step = 2;
    ClientRequest::SendToHost(m);
  }
  room_sn_ = 0;
  self_slot_ = -1;
  members_.clear();
  chat_log_.clear();
  history_.clear();
  history_idx_ = 0;
  page_ = 0;
  user_page_ = 0;
  unread_ = false;
  typing_sent_ = false;
  typing_sent_at_ = 0;
  chat_input_.text.clear();
  chat_input_.cur = 0;
  chat_input_.composition.clear();
}

void MapleChatUiSystem::TryOpen() {
  // 095 TryNew(0)：已有窗口时无动作。
  if (IsOpen()) {
    return;
  }
  Open();
  room_sn_ = 0;
  // 095 OnCreate 帮助文本（String 816/1900/817/818/1900）。
  AddChat(u"[MapleStory通讯帮助]", kSystem);
  AddChat(u"-------------------------", kSystem);
  AddChat(u"招待 ：/招待 角色名", kSystem);
  AddChat(u"结束 ：/q", kSystem);
  AddChat(u"-------------------------", kSystem);
  // 095 CP 143 step0：请求进入房间（SN=0 新建）。
  fbs::ClientCharacterMessengerT m;
  m.step = 0;
  m.to_id = 0;
  ClientRequest::SendToHost(m);
}

void MapleChatUiSystem::Toggle() {
  // 095 快捷键语义：只负责打开（TryNew），关闭走 ESC/关闭按钮。
  TryOpen();
}

void MapleChatUiSystem::JoinRoom(uint64_t sn) {
  if (sn == 0) {
    return;
  }
  if (IsOpen()) {
    // 已在别的聊天室：先离开再加入新邀请的房间。
    LeaveRoom();
    chat_log_.clear();
  } else {
    Open();
  }
  AddChat(u"接受邀请，进入聊天室…", kSystem);
  // 095 OnCreate pData=SN：按 SN 请求进房。
  fbs::ClientCharacterMessengerT m;
  m.step = 0;
  m.to_id = sn;
  ClientRequest::SendToHost(m);
}

void MapleChatUiSystem::InviteClient(uint64_t client_id) {
  // 095：右键聊天邀请 = TryNew(0) + SendInviteMsg。
  if (client_id == 0) {
    return;
  }
  if (!IsOpen()) {
    TryOpen();
  }
  fbs::ClientCharacterMessengerT m;
  m.step = 3;
  m.to_id = client_id;
  ClientRequest::SendToHost(m);
  AudioGameInstance::PlayNow(u"UI.img/BtMouseClick");
}

void MapleChatUiSystem::InviteName(const std::u16string& name) {
  // 095 SendInviteMsg：校验非空且不是自己。
  if (name.empty()) {
    return;
  }
  if (name == SelfName()) {
    // 095 String[822]。
    AddChat(u"- 请输入另外角色名。", kError);
    return;
  }
  fbs::ClientCharacterMessengerT m;
  m.step = 3;
  m.chat = {name.begin(), name.end()};
  ClientRequest::SendToHost(m);
}

void MapleChatUiSystem::AddChat(const std::u16string& text, int color) {
  chat_log_.push_back({text, color});
  if (chat_log_.size() > static_cast<size_t>(kChatMaxLines)) {
    chat_log_.erase(chat_log_.begin());
  }
}

// ---- 服务端状态写入 ----

void MapleChatUiSystem::OnSelfEnter(
    uint64_t sn, uint8_t slot,
    std::vector<std::unique_ptr<fbs::PlayerT>>& players) {
  room_sn_ = sn;
  self_slot_ = slot;
  members_.assign(kMaxMembers, Member{});
  for (size_t i = 0; i < players.size() && i < kMaxMembers; ++i) {
    if (players[i] == nullptr) {
      continue;
    }
    Member m;
    m.player = *players[i];
    if (m.player.character != nullptr) {
      m.name = {m.player.character->name.begin(),
                m.player.character->name.end()};
    }
    members_[i] = std::move(m);
  }
  if (!IsOpen()) {
    Open();
  }
  // 095 OnSelfEnter bNew：应邀加入时提示，自己新建的空房不提示。
  if (players.size() > 1) {
    AddChat(u"已进入聊天室。", kSystem);
  }
}

void MapleChatUiSystem::OnMemberEnter(fbs::PlayerT& player, uint8_t slot) {
  if (slot >= kMaxMembers) {
    return;
  }
  // 进房前收到成员进入：先补齐槽位。
  if (members_.size() < kMaxMembers) {
    members_.resize(kMaxMembers);
  }
  Member m;
  m.player = player;
  if (m.player.character != nullptr) {
    m.name = {m.player.character->name.begin(),
              m.player.character->name.end()};
  }
  members_[slot] = std::move(m);
  // 095 String[819]：- 's'玩家入场了。
  AddChat(u"-" + members_[slot].name + u"玩家入场了。", kSystem);
  if (state_ == 2) {
    unread_ = true;
  }
}

void MapleChatUiSystem::OnMemberLeave(fbs::PlayerT* player, uint64_t client_id,
                                      uint8_t slot) {
  if (slot >= kMaxMembers || slot >= members_.size()) {
    return;
  }
  std::u16string name;
  if (player != nullptr && player->character != nullptr) {
    name = {player->character->name.begin(), player->character->name.end()};
  } else if (members_[slot].player.client_id == client_id) {
    name = members_[slot].name;
  }
  // 095 String[821]：- 's'玩家退出房间。
  AddChat(u"-" + name + u"玩家退出房间。", kSystem);
  members_[slot] = Member{};
  if (state_ == 2) {
    unread_ = true;
  }
}

void MapleChatUiSystem::OnChat(fbs::PlayerT* player,
                               const std::u16string& text, uint8_t typing) {
  if (player == nullptr) {
    return;
  }
  if (!text.empty()) {
    std::u16string name =
        player->character != nullptr
            ? std::u16string{player->character->name.begin(),
                             player->character->name.end()}
            : std::u16string{u"???"};
    AddChat(name + u" : " + text, kChat);
    if (state_ == 2) {
      unread_ = true;
    }
    return;
  }
  // 打字状态（095 "name0/1"）：点亮对应名牌的省略号。
  for (auto& m : members_) {
    if (m.player.client_id == player->client_id) {
      m.typing = typing != 0;
      m.typing_at = Window::dt_now_ + 5000;
    }
  }
}

void MapleChatUiSystem::EnterFailed() {
  // 095 String[820]：- 不能参与受招待的聊天房。
  AddChat(u"- 不能参与受招待的聊天房。", kError);
  room_sn_ = 0;
  self_slot_ = -1;
  members_.clear();
}

// ---- 渲染 ----

void MapleChatUiSystem::RenderBackgrnd() {
  static auto tex_norm = WzResource::LoadTexture(
      WzResource::ui_->find(u"MapleChat.img/MapleChat/backgrnd"));
  static auto tex_max = WzResource::LoadTexture(
      WzResource::ui_->find(u"MapleChat.img/MapleChat_Max/backgrnd"));
  static auto tex_min = WzResource::LoadTexture(
      WzResource::ui_->find(u"MapleChat.img/MapleChat_Min/backgrnd"));
  auto* tex = state_ == 0 ? tex_max : (state_ == 2 ? tex_min : tex_norm);
  SDL_FRect pos_rect{pos_.x, pos_.y, static_cast<float>(tex->w),
                     static_cast<float>(tex->h)};
  SDL_RenderTexture(Window::renderer_, tex, nullptr, &pos_rect);
}

void MapleChatUiSystem::RenderUsers() {
  static auto name_tag = [](int slot) {
    // 名牌按房间 slot 取 0..3；空位用 empty。
    if (slot >= 0 && slot < 4) {
      return WzResource::LoadTexture(WzResource::ui_->find(
          u"_Canvas/MapleChat.img/NameTag/" + U16FromNum(slot)));
    }
    return WzResource::LoadTexture(
        WzResource::ui_->find(u"_Canvas/MapleChat.img/NameTag/empty"));
  };
  static auto tag_empty = WzResource::LoadTexture(
      WzResource::ui_->find(u"_Canvas/MapleChat.img/NameTag/empty"));

  const uint64_t now = Window::dt_now_;
  const int page_base = static_cast<int>(user_page_) * kUsersPerPage;
  for (int i = 0; i < kUsersPerPage; ++i) {
    const int slot = page_base + i;
    // members_ 在进房前可能为空，下标必须双重约束。
    if (slot >= static_cast<int>(members_.size()) ||
        slot >= static_cast<int>(kMaxMembers)) {
      continue;
    }
    auto& m = members_[slot];
    const bool present = m.player.client_id != 0;
    // 打字状态超时自动熄灭。
    if (m.typing && now > m.typing_at) {
      m.typing = false;
    }

    SDL_FRect r{pos_.x + kUserX + i * kUserStep, pos_.y + kUserY, 89, 19};
    auto* tex = present ? name_tag(slot) : tag_empty;
    SDL_RenderTexture(Window::renderer_, tex, nullptr, &r);

    Freetype::LoadSize(11);
    Freetype::LoadAligned(true);
    Freetype::LoadColor(255, 255, 255, 255);
    if (!present) {
      continue;
    }
    std::u16string name = m.name;
    while (!name.empty() && Freetype::LoadW(name) > 66) {
      name.pop_back();
      if (Freetype::LoadW(name + u"...") <= 66) {
        name += u"...";
        break;
      }
    }
    Freetype::DrawLine(name, r.x + 6, r.y + 3);
    if (m.typing) {
      // 095：对方正在输入时名牌旁亮出省略号气泡。
      Freetype::LoadColor(255, 180, 60, 255);
      Freetype::DrawLine(u"...", r.x + 70, r.y + 3);
    }
  }

  // 页码（超过一页时可翻）。
  const int max_user_page =
      std::max(0, (static_cast<int>(kMaxMembers) - 1) / kUsersPerPage);
  Freetype::LoadSize(10);
  Freetype::LoadColor(90, 90, 90, 255);
  Freetype::DrawLine(U16FromNum(user_page_ + 1) + u"/" +
                         U16FromNum(static_cast<uint32_t>(max_user_page + 1)),
                     pos_.x + 338, pos_.y + 30);

  // 邀请输入框（点 "+" 后出现）。
  if (invite_active_) {
    SDL_SetRenderDrawBlendMode(Window::renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(Window::renderer_, 255, 255, 255, 235);
    SDL_FRect box{pos_.x + 24, pos_.y + 50, 272, 16};
    SDL_RenderFillRect(Window::renderer_, &box);
    SDL_SetRenderDrawColor(Window::renderer_, 110, 110, 110, 255);
    SDL_RenderRect(Window::renderer_, &box);
    Freetype::LoadSize(11);
    Freetype::LoadAligned(true);
    Freetype::LoadColor(70, 70, 70, 255);
    Freetype::DrawLine(u"邀请:", pos_.x + 28, pos_.y + 52);
    invite_input_.Render();
  }
}

void MapleChatUiSystem::RenderChatLog() {
  const bool is_max = state_ == 0;
  const int visible = VisibleLogLines(is_max);

  std::vector<std::pair<std::u16string, int>> lines;
  for (const auto& l : chat_log_) {
    for (auto& seg : WrapLines(l.text, kLogW - 12)) {
      lines.push_back({seg, l.color});
    }
  }
  const int total = static_cast<int>(lines.size());
  const int max_page = std::max(0, total - visible);
  page_ = std::min(page_, static_cast<uint32_t>(max_page));
  const int first = max_page - static_cast<int>(page_);

  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  for (int i = 0; i < visible; ++i) {
    const int idx = first + i;
    if (idx < 0 || idx >= total) {
      continue;
    }
    switch (lines[idx].second) {
      case kSystem:
        Freetype::LoadColor(70, 100, 170, 255);
        break;
      case kError:
        Freetype::LoadColor(190, 50, 50, 255);
        break;
      case kNotice:
        Freetype::LoadColor(90, 130, 60, 255);
        break;
      default:
        Freetype::LoadColor(30, 30, 30, 255);
        break;
    }
    Freetype::DrawLine(lines[idx].first, pos_.x + kLogX,
                       pos_.y + kLogY + i * kLineH);
  }
}

void MapleChatUiSystem::RenderInput() {
  const float dy = state_ == 0 ? kDelta : 0;
  static auto textbox = WzResource::LoadTexture(
      WzResource::ui_->find(u"MapleChat.img/MapleChat/layer:textbox"));
  SDL_FRect r{pos_.x + kTextbox.x, pos_.y + kTextbox.y + dy,
              static_cast<float>(textbox->w), static_cast<float>(textbox->h)};
  SDL_RenderTexture(Window::renderer_, textbox, nullptr, &r);

  if (!chat_input_.active && chat_input_.text.empty()) {
    Freetype::LoadSize(11);
    Freetype::LoadAligned(true);
    Freetype::LoadColor(150, 150, 150, 255);
    Freetype::DrawLine(u"按 ENTER 发言", pos_.x + 12, pos_.y + 271 + dy);
  }
  chat_input_.Render();
}

void MapleChatUiSystem::RenderBlink() {
  // 095 StartBlink/ToggleBlink：最小化时有新消息气泡闪烁。
  static auto tex_on = WzResource::LoadTexture(WzResource::ui_->find(
      u"MapleChat.img/MapleChat_Min/layer:newchat_enabled"));
  static auto tex_off = WzResource::LoadTexture(WzResource::ui_->find(
      u"MapleChat.img/MapleChat_Min/layer:newchat_disabled"));
  bool on = false;
  if (unread_) {
    on = (Window::dt_now_ / 250) % 2 == 0;
  }
  SDL_FRect r{pos_.x + kNewchat.x, pos_.y + kNewchat.y,
              static_cast<float>(tex_off->w), static_cast<float>(tex_off->h)};
  SDL_RenderTexture(Window::renderer_, on ? tex_on : tex_off, nullptr, &r);
}

UIComponents& MapleChatUiSystem::GetComponent() {
  components_.clear();
  const bool top =
      CursorGameInstance::cursor_ui_ == Render &&
      (CursorGameInstance::modal_overlay_ == nullptr ||
       CursorGameInstance::modal_overlay_ == Render);

  auto add_button = [&](const char16_t* path, SDL_FRect rect, bool enable,
                        void (*callback)()) {
    Button b{.node = WzResource::ui_->find(path),
             .rect = rect,
             .enable = enable,
             .focused = top};
    b.rect.x += pos_.x;
    b.rect.y += pos_.y;
    b.callback = callback;
    components_.push_back(b);
  };

  if (state_ == 2) {
    // 最小化标题条：还原 / 关闭。
    add_button(u"MapleChat.img/MapleChat/button:max", kMinBtnMax, true,
               EventButtonMax);
    add_button(u"MapleChat.img/MapleChat/button:close", kMinBtnClose, true,
               EventButtonClose);
    return components_;
  }

  const float dy = state_ == 0 ? kDelta : 0;

  // 标题栏按钮。
  add_button(u"MapleChat.img/MapleChat/button:min", kBtnMin, true,
             EventButtonMin);
  add_button(u"MapleChat.img/MapleChat/button:max", kBtnMax, true,
             EventButtonMax);
  add_button(u"MapleChat.img/MapleChat/button:close", kBtnClose, true,
             EventButtonClose);

  // 名牌条：翻页 + 邀请。
  const int max_user_page =
      std::max(0, (static_cast<int>(kMaxMembers) - 1) / kUsersPerPage);
  add_button(u"MapleChat.img/MapleChat/button:prev", kBtnUserPrev,
             user_page_ > 0, EventButtonUserPrev);
  add_button(u"MapleChat.img/MapleChat/button:next", kBtnUserNext,
             user_page_ < static_cast<uint32_t>(max_user_page),
             EventButtonUserNext);
  add_button(u"MapleChat.img/MapleChat/button:invite", kBtnInvite, true,
             EventButtonInvite);

  // 输入区：ENTER（空输入时置灰，095 BtEnter SetEnable）+ 举报。
  add_button(u"MapleChat.img/MapleChat/button:enter",
             {kBtnEnter.x, kBtnEnter.y + dy, kBtnEnter.w, kBtnEnter.h},
             self_slot_ >= 0 && !chat_input_.text.empty(), EventButtonEnter);
  add_button(u"MapleChat.img/MapleChat/button:claim",
             {kBtnClaim.x, kBtnClaim.y + dy, kBtnClaim.w, kBtnClaim.h}, true,
             EventButtonClaim);

  // 聊天记录滚动条（VScr4）。
  const bool is_max = state_ == 0;
  const int visible = VisibleLogLines(is_max);
  const int total = TotalLogLines();
  const int max_page = std::max(0, total - visible);
  page_ = std::min(page_, static_cast<uint32_t>(max_page));
  const uint32_t first =
      static_cast<uint32_t>(std::max(0, max_page - static_cast<int>(page_)));
  Scroll scroll(first, static_cast<uint32_t>(total),
                is_max ? kScrollLenMax : kScrollLenNorm,
                static_cast<uint32_t>(visible));
  scroll.pos = {pos_.x + kScrollX, pos_.y + kScrollY};
  scroll.top = top;
  components_.push_back(scroll);

  return components_;
}

bool MapleChatUiSystem::Render() {
  // 我方打字状态上报（095 Update："name0/1"，3 秒心跳）。
  if (self_slot_ >= 0) {
    const bool now_typing = chat_input_.active && !chat_input_.text.empty();
    if (now_typing != typing_sent_ ||
        (now_typing && Window::dt_now_ - typing_sent_at_ > 3000)) {
      SendTyping(now_typing);
    }
  }

  RenderBackgrnd();
  if (state_ == 2) {
    RenderBlink();
  } else {
    RenderUsers();
    RenderChatLog();
    RenderInput();
  }
  RenderUI();
  return true;
}

// ---- 按钮/输入事件 ----

void MapleChatUiSystem::EventButtonMin() {
  // 095 BtMin：SetState((state+1)%3)。
  state_ = (state_ + 1) % 3;
  if (state_ != 2) {
    unread_ = false;
  }
  SyncInputRects();
}

void MapleChatUiSystem::EventButtonMax() {
  // 095 BtMax：SetState((state-1)%3)。
  state_ = (state_ + 2) % 3;
  if (state_ != 2) {
    unread_ = false;
  }
  SyncInputRects();
}

void MapleChatUiSystem::EventButtonClose() {
  Close();
}

void MapleChatUiSystem::EventButtonEnter() {
  ProcessChat();
}

void MapleChatUiSystem::EventButtonClaim() {
  // 095 BtClame：把聊天记录交给 GM 系统；本服无 GM 端点，
  // 转为复制可举报内容到剪贴板。
  std::u16string copied;
  for (const auto& l : chat_log_) {
    if (!copied.empty()) {
      copied.push_back(u'\n');
    }
    copied += l.text;
  }
  if (!copied.empty()) {
    auto utf8 = TextGameInstance::LoadU8str(copied);
    SDL_SetClipboardText(utf8.c_str());
    AddChat(u"- 已复制聊天记录。", kNotice);
  } else {
    AddChat(u"- 没有可举报的聊天记录。", kSystem);
  }
}

void MapleChatUiSystem::EventButtonInvite() {
  invite_active_ = !invite_active_;
  if (invite_active_) {
    invite_input_.text.clear();
    invite_input_.cur = 0;
    invite_input_.composition.clear();
    invite_input_.Active();
    // 输入期间屏蔽游戏键盘（与聊天输入一致）。
    std::erase(System::event_systems_, KeyboardInputSystem::Event);
  } else {
    invite_input_.Close();
  }
}

void MapleChatUiSystem::EventButtonUserPrev() {
  if (user_page_ > 0) {
    --user_page_;
  }
}

void MapleChatUiSystem::EventButtonUserNext() {
  const int max_user_page =
      std::max(0, (static_cast<int>(kMaxMembers) - 1) / kUsersPerPage);
  if (user_page_ < static_cast<uint32_t>(max_user_page)) {
    ++user_page_;
  }
}

void MapleChatUiSystem::SendTyping(bool typing) {
  if (self_slot_ < 0) {
    return;
  }
  fbs::ClientCharacterMessengerT m;
  m.step = 6;
  m.x = typing ? 1 : 0;
  ClientRequest::SendToHost(m);
  typing_sent_ = typing;
  typing_sent_at_ = Window::dt_now_;
}

void MapleChatUiSystem::ProcessChat() {
  // 095 ProcessChat：空文本或未进房不处理。
  if (chat_input_.text.empty() || self_slot_ < 0) {
    return;
  }
  std::u16string text = chat_input_.text;
  chat_input_.text.clear();
  chat_input_.cur = 0;
  chat_input_.composition.clear();

  if (text.starts_with(u'/')) {
    HandleSlashCommand(text);
    return;
  }

  // 聊天历史（095 CChatHelper History）。
  history_.push_back(text);
  if (history_.size() > 50) {
    history_.erase(history_.begin());
  }
  history_idx_ = history_.size();

  AddChat(SelfName() + u" : " + text, kChat);
  fbs::ClientCharacterMessengerT m;
  m.step = 6;
  m.chat = {text.begin(), text.end()};
  ClientRequest::SendToHost(m);
  if (typing_sent_) {
    SendTyping(false);
  }
}

void MapleChatUiSystem::HandleSlashCommand(const std::u16string& text) {
  // 095 HandleSlashCommand：/q 退出；/招待(或/invite) 角色名 邀请。
  if (text == u"/q" || text == u"/Q") {
    Close();
    return;
  }
  constexpr std::u16string_view kInviteCmds[] = {u"/招待", u"/invite",
                                                u"/INVITE"};
  for (auto cmd : kInviteCmds) {
    if (text.starts_with(cmd)) {
      auto name = text.substr(cmd.size());
      // 去掉命令与名字间的空白。
      while (!name.empty() && (name.front() == u' ' || name.front() == u'　')) {
        name.erase(name.begin());
      }
      while (!name.empty() && name.back() == u' ') {
        name.pop_back();
      }
      if (name.empty() || name == SelfName()) {
        // 095 String[822]。
        AddChat(u"- 请输入另外角色名。", kError);
      } else {
        InviteName(name);
      }
      return;
    }
  }
  AddChat(u"- 未知命令。", kSystem);
}

bool MapleChatUiSystem::EventUI(SDL_Event* event) {
  auto& comps = GetComponent();
  bool consumed = DispatchButtonClick(comps, true);
  // 滚动条点击（chat_log 同款映射）。
  for (auto& component : comps) {
    if (auto* scroll = std::get_if<Scroll>(&component)) {
      const int visible = VisibleLogLines(state_ == 0);
      const int total = TotalLogLines();
      const int max_page = std::max(0, total - visible);
      auto first = scroll->Click();
      page_ = static_cast<uint32_t>(
          std::clamp(max_page - static_cast<int>(first), 0, max_page));
    }
  }
  return consumed;
}

bool MapleChatUiSystem::Event(SDL_Event* event) {
  static int vscroll_drag = -1;
  bool r = true;
  // 输入框先处理（激活/失焦/文本编辑）；无激活输入时自动恢复游戏键盘。
  if (invite_active_) {
    invite_input_.Event(event);
  }
  chat_input_.Event(event);
  if (!chat_input_.active && !invite_input_.active &&
      std::ranges::find(System::event_systems_, &KeyboardInputSystem::Event) ==
          System::event_systems_.end()) {
    System::event_systems_.push_back(KeyboardInputSystem::Event);
  }

  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorIn()) {
          // 抬到最上层。
          std::erase(System::render_systems_, Render);
          std::erase(System::event_systems_, Event);
          auto it = std::ranges::find(System::render_systems_,
                                      &CursorRenderSystem::Render);
          if (it != System::render_systems_.end()) {
            System::render_systems_.insert(it, Render);
            System::event_systems_.insert(System::event_systems_.begin(),
                                           Event);
          }
          // 标题栏拖动（最小化时整条可拖）。
          auto [w, h] = LoadWh();
          SDL_FRect bar{pos_.x, pos_.y, w, state_ == 2 ? h : 24};
          SDL_FPoint mp{static_cast<float>(event->button.x),
                        static_cast<float>(event->button.y)};
          if (SDL_PointInRectFloat(&mp, &bar)) {
            drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
          }
          DispatchScrollDragStart(GetComponent(), vscroll_drag,
                                  Window::mouse_pos_);
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorIn()) {
          r = !EventUI(event);
        }
        vscroll_drag = -1;
        drag_ = std::nullopt;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if (drag_.has_value()) {
        pos_ = {event->motion.x + drag_->x, event->motion.y + drag_->y};
        auto& camera = CameraGameInstance::camera_;
        auto [w, h] = LoadWh();
        pos_.x = std::clamp(pos_.x, 0.0f, camera.w - w);
        pos_.y = std::clamp(pos_.y, 0.0f, camera.h - h);
        SyncInputRects();
      }
      if (vscroll_drag >= 0) {
        const int visible = VisibleLogLines(state_ == 0);
        const int total = TotalLogLines();
        const int max_page = std::max(0, total - visible);
        int first = std::clamp(max_page - static_cast<int>(page_), 0, max_page);
        uint32_t val = static_cast<uint32_t>(first);
        if (DispatchScrollDragMove(GetComponent(), vscroll_drag,
                                   {event->motion.x, event->motion.y}, val)) {
          page_ = static_cast<uint32_t>(
              std::clamp(max_page - static_cast<int>(val), 0, max_page));
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
      if (CursorIn() && state_ != 2) {
        const int visible = VisibleLogLines(state_ == 0);
        const int total = TotalLogLines();
        const int max_page = std::max(0, total - visible);
        if (event->wheel.integer_y > 0) {
          page_ = static_cast<uint32_t>(
              std::min<int>(max_page, static_cast<int>(page_) + 1));
        } else if (event->wheel.integer_y < 0) {
          page_ = static_cast<uint32_t>(
              std::max(0, static_cast<int>(page_) - 1));
        }
        r = false;
      }
      break;
    }
    case SDL_EVENT_TEXT_INPUT:
    case SDL_EVENT_TEXT_EDITING: {
      r = false;
      break;
    }
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = event->key.scancode;
      if (scan_code == SDL_SCANCODE_ESCAPE) {
        if (invite_active_) {
          EventButtonInvite();  // 再点一次 = 取消
        } else if (chat_input_.active) {
          chat_input_.Close();
          chat_input_.text.clear();
          chat_input_.cur = 0;
          chat_input_.composition.clear();
          if (std::ranges::find(System::event_systems_,
                                &KeyboardInputSystem::Event) ==
              System::event_systems_.end()) {
            System::event_systems_.push_back(KeyboardInputSystem::Event);
          }
        } else {
          Close();
        }
        r = false;
      } else if (scan_code == SDL_SCANCODE_RETURN) {
        if (invite_active_) {
          auto name = invite_input_.text;
          EventButtonInvite();  // 关闭输入框
          if (!name.empty()) {
            InviteName(name);
          }
        } else if (chat_input_.active) {
          ProcessChat();
        } else {
          chat_input_.Active();
          std::erase(System::event_systems_, KeyboardInputSystem::Event);
        }
        r = false;
      } else if (scan_code == SDL_SCANCODE_UP && chat_input_.active) {
        if (!history_.empty()) {
          history_idx_ = history_idx_ > 0 ? history_idx_ - 1 : 0;
          chat_input_.text = history_[history_idx_];
          chat_input_.cur = static_cast<int>(chat_input_.text.size());
          chat_input_.composition.clear();
        }
        r = false;
      } else if (scan_code == SDL_SCANCODE_DOWN && chat_input_.active) {
        // 095 HistoryDown：到最新位置后清空（history_idx_ == size 为最新空位）。
        if (!history_.empty() && history_idx_ < history_.size()) {
          ++history_idx_;
        }
        if (history_idx_ < history_.size()) {
          chat_input_.text = history_[history_idx_];
        } else {
          chat_input_.text.clear();
        }
        chat_input_.cur = static_cast<int>(chat_input_.text.size());
        chat_input_.composition.clear();
        r = false;
      } else if (chat_input_.active || invite_input_.active) {
        r = false;
      }
      break;
    }
    default: {
      break;
    }
  }
  return r;
}

}  // namespace sdlms
