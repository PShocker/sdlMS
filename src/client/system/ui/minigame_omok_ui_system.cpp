#include "src/client/system/ui/minigame_omok_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <cmath>
#include <string>

namespace sdlms {

namespace {

// 窗口内几何（全屏版 Minigame.img 背景实测 + 095 坐标）。
// 右列两个玩家站位：bay 左 ~[382,477]，bay 右 ~[490,585]；名牌在站位下方。
constexpr float kNameTagY = 147;
constexpr float kScoreY = 170;
constexpr float kStoneBoxCx[2] = {401, 526};
constexpr float kStoneBoxY = 172;
constexpr float kReadyLampY = 198;
constexpr float kChatLogY = 255;
constexpr float kChatLineH = 14;
constexpr int kChatMaxLines = 7;

// 按钮位置（窗口内坐标）。
constexpr SDL_FRect kStartRect = {368, 224, 65, 19};
constexpr SDL_FRect kReadyRect = {494, 224, 65, 19};
constexpr SDL_FRect kForfeitRect = {28, 391, 50, 17};
constexpr SDL_FRect kUndoRect = {88, 391, 54, 17};
constexpr SDL_FRect kDrawRect = {152, 391, 54, 17};
constexpr SDL_FRect kLeaveRect = {549, 391, 41, 17};

// 房间窗口几何之外的展示角色准备：参照 CashShop 试穿的配置方式
//（face=default + 清地图态 + RunStandAction），地图外的立绘渲染必须
// 走 stand1/stand2（bone_data_ 里没有 "stand" 键，直接设置会 flat_map::at 崩）。
bool AvatarReady(const GameCharacter& g) {
  const auto& avatars = CharacterGameInstance::avatar_data_;
  const auto& faces = CharacterGameInstance::face_data_;
  return avatars.contains(g.head) && avatars.contains(g.body) &&
         avatars.contains(g.hair) && faces.contains(g.face.id) &&
         faces.at(g.face.id).data.contains(g.face.action);
}

std::u16string U16FromNum(uint32_t num) {
  auto s = std::to_string(num);
  return {s.begin(), s.end()};
}

}  // namespace

SDL_FPoint MinigameOmokUiSystem::LoadWh() {
  return {598, 419};
}

bool MinigameOmokUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect);
}

void MinigameOmokUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  auto [w, h] = LoadWh();
  auto& camera = CameraGameInstance::camera_;
  pos_.x = static_cast<float>((camera.w - w) / 2);
  pos_.y = static_cast<float>((camera.h - h) / 2);
  show_self_ = CharacterGameInstance::self_;
  PrepareShowCharacter(show_self_);
  chat_input_.Close();
  chat_input_ = TextInput{
      .max_size = 60,
      .text = u"",
      .composition = u"",
      .disable = false,
      .active = false,
      .r = SDL_Rect{static_cast<int>(pos_.x + 390),
                    static_cast<int>(pos_.y + 362), 178, 16},
      .pos = {0, 0},
      .font_color = {0, 0, 0, 255},
      .cur_color = {0, 0, 0, 255},
      .font_size = 11,
  };

  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void MinigameOmokUiSystem::Close() {
  chat_input_.Close();
  if (std::ranges::find(System::event_systems_, &KeyboardInputSystem::Event) ==
      System::event_systems_.end()) {
    System::event_systems_.push_back(KeyboardInputSystem::Event);
  }
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void MinigameOmokUiSystem::PrepareShowCharacter(GameCharacter& g) {
  // CashShop 同款：静态站立 + default 脸，清理地图态会干扰立绘的字段。
  g.face.action = u"default";
  g.face.index = 0;
  g.face.time = 0;
  g.face.destroy = 0;
  g.color = {255, 255, 255, 255};
  g.scale = 1;
  g.action = u"";  // 先清再切，迫使 RunAction 走切换分支重置帧。
  g.action_index = 0;
  g.action_time = 0;
  g.action_animate = true;
  g.chair = std::nullopt;
  g.tomb = std::nullopt;
  g.skill = std::nullopt;
  g.chatballoon = std::nullopt;
  g.effect.clear();
  g.morph.clear();
  g.debuffs.clear();
  CharacterLogicSystem::RunStandAction(g);
}

void MinigameOmokUiSystem::AddChat(const std::u16string& line) {
  chat_log_.push_back(line);
  if (chat_log_.size() > 100) {
    chat_log_.erase(chat_log_.begin());
  }
}

void MinigameOmokUiSystem::ResetRound() {
  for (auto& row : board_) {
    for (auto& cell : row) {
      cell = 0;
    }
  }
  history_.clear();
  last_stone_[0] = -1;
  last_stone_[1] = -1;
  result_ = -1;
  result_winner_color_ = -1;
  undo_pending_ = false;
  draw_pending_ = false;
}

void MinigameOmokUiSystem::SendStep(uint8_t step, uint8_t x, uint8_t y) {
  fbs::ClientCharacterMinigameT m;
  m.step = step;
  m.x = x;
  m.y = y;
  ClientRequest::SendToHost(m);
}

void MinigameOmokUiSystem::EnterRoomByItem() {
  SendStep(0);
}

void MinigameOmokUiSystem::EnsureTableItem() {
  auto& items = PackageGameInstance::data_[(int)ItemEnum::kInstall];
  for (auto& itm : items) {
    if (itm->id.starts_with(u"0408")) {
      return;
    }
  }
  for (auto& slot : items) {
    if (slot->id.empty()) {
      slot = ItemGameInstance::LoadItem(u"04080000", 1);
      return;
    }
  }
}

void MinigameOmokUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Omok/backgrnd"));
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
}

void MinigameOmokUiSystem::RenderPlayers() {
  static auto name_tag = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Common/nameTag0"));
  static auto name_tag_empty = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Common/emptyTag"));

  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);

  auto draw_side = [&](int slot, bool present, const std::u16string& name,
                       GameCharacter* g) {
    // 角色立绘（095 GetActionPosition：站台，slot0 朝右 / slot1 朝左）。
    if (present && g != nullptr && AvatarReady(*g)) {
      auto& camera = CameraGameInstance::camera_;
      g->pos = {pos_.x + kSlotX[slot] + camera.x,
                pos_.y + kSlotY + camera.y};
      g->flip = slot == 0 ? 1 : 0;
      CharacterLogicSystem::RunAnimate(*g);
      CharacterRenderSystem::RenderCharacter(*g);
    }

    // 名牌：站位下方居中（095 名字画在头像正下方 y=152 附近）。
    SDL_FRect tag_rect{pos_.x + kSlotX[slot] - 44, pos_.y + kNameTagY,
                       static_cast<float>(name_tag->w),
                       static_cast<float>(name_tag->h)};
    if (present) {
      Freetype::LoadColor(255, 255, 255, 255);
    } else {
      Freetype::LoadColor(150, 150, 150, 255);
    }
    SDL_RenderTexture(Window::renderer_, present ? name_tag : name_tag_empty,
                      nullptr, &tag_rect);
    if (!name.empty()) {
      Freetype::DrawLine(name, tag_rect.x + 6, tag_rect.y + 3);
    }

    // 战绩：自己一侧用服务端同步的战绩，对手一侧取镜像（胜↔负）。
    uint32_t win = slot == my_slot_ ? my_win_ : my_lose_;
    uint32_t lose = slot == my_slot_ ? my_lose_ : my_win_;
    uint32_t draw = my_draw_;
    auto score = u"WIN " + U16FromNum(win) + u"  DRAW " + U16FromNum(draw) +
                 u"  LOSE " + U16FromNum(lose);
    Freetype::LoadColor(190, 140, 60, 255);
    auto w = Freetype::LoadW(score);
    Freetype::DrawLine(score, pos_.x + kSlotX[slot] - w / 2, pos_.y + kScoreY);
  };

  bool me_left = my_slot_ == 0;
  if (me_left) {
    draw_side(0, true, CharacterGameInstance::self_.name, &show_self_);
    draw_side(1, has_peer_, peer_name_, has_peer_ ? &peer_ : nullptr);
  } else {
    draw_side(1, true, CharacterGameInstance::self_.name, &show_self_);
    draw_side(0, has_peer_, peer_name_, has_peer_ ? &peer_ : nullptr);
  }
}

void MinigameOmokUiSystem::RenderStoneBoxes() {
  static auto black_stone = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Omok/stone/0/black/0"));
  static auto white_stone = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Omok/stone/0/white/0"));
  static auto ready_on = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Common/readyOn"));
  static auto ready_off = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Common/readyOff"));

  // 房主执黑：slot0 黑，slot1 白。
  for (int slot = 0; slot < 2; ++slot) {
    auto* stone = slot == 0 ? black_stone : white_stone;
    SDL_FRect r{pos_.x + kStoneBoxCx[slot] - stone->w / 2,
                pos_.y + kStoneBoxY, static_cast<float>(stone->w),
                static_cast<float>(stone->h)};
    SDL_RenderTexture(Window::renderer_, stone, nullptr, &r);

    // 就绪灯（095 DrawReadyOrNot：readyOn/readyOff）。
    bool ready = phase_ == Phase::kPlaying;
    if (slot == my_slot_) {
      ready = ready || my_ready_;
    } else {
      ready = ready || peer_ready_;
    }
    auto* lamp = ready ? ready_on : ready_off;
    r = {pos_.x + kStoneBoxCx[slot] - lamp->w / 2, pos_.y + kReadyLampY,
         static_cast<float>(lamp->w), static_cast<float>(lamp->h)};
    SDL_RenderTexture(Window::renderer_, lamp, nullptr, &r);
  }
}

void MinigameOmokUiSystem::RenderStones() {
  static auto black_stone = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Omok/stone/0/black/0"));
  static auto white_stone = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Omok/stone/0/white/0"));
  static auto mark =
      WzResource::LoadTexture(WzResource::ui_->find(u"Minigame.img/Common/mark"));

  for (int row = 0; row < kBoardSize; ++row) {
    for (int col = 0; col < kBoardSize; ++col) {
      auto cell = board_[row][col];
      if (cell == 0) {
        continue;
      }
      auto* stone = cell == 1 ? black_stone : white_stone;
      float ix = pos_.x + kBoardOriginX + col * kBoardSpacing;
      float iy = pos_.y + kBoardOriginY + row * kBoardSpacing;
      SDL_FRect r{ix - stone->w / 2, iy - stone->h / 2,
                  static_cast<float>(stone->w), static_cast<float>(stone->h)};
      SDL_RenderTexture(Window::renderer_, stone, nullptr, &r);
    }
  }

  // 最后一手标记（095 m_pLayerLastStone）。
  if (last_stone_[0] >= 0 && last_stone_[1] >= 0) {
    float ix = pos_.x + kBoardOriginX + last_stone_[0] * kBoardSpacing;
    float iy = pos_.y + kBoardOriginY + last_stone_[1] * kBoardSpacing;
    SDL_FRect r{ix - mark->w / 2, iy - mark->h / 2,
                static_cast<float>(mark->w), static_cast<float>(mark->h)};
    SDL_RenderTexture(Window::renderer_, mark, nullptr, &r);
  }
}

void MinigameOmokUiSystem::RenderTurn() {
  if (phase_ != Phase::kPlaying) {
    return;
  }
  static auto turn =
      WzResource::LoadTexture(WzResource::ui_->find(u"Minigame.img/Common/turn"));
  // 当前行棋方站台旁（095 Draw：turn 层位于两站台一带）。
  int slot = turn_color_ == 0 ? 0 : 1;
  SDL_FRect r{pos_.x + kSlotX[slot] - turn->w / 2, pos_.y + 46,
              static_cast<float>(turn->w), static_cast<float>(turn->h)};
  SDL_RenderTexture(Window::renderer_, turn, nullptr, &r);

  // 剩余时间（095 m_nTimeLeft：每手 30 秒）。
  uint64_t left = 0;
  if (Window::dt_now_ < turn_deadline_) {
    left = (turn_deadline_ - Window::dt_now_) / 1000;
  }
  auto text = u"0:" + U16FromNum(static_cast<uint32_t>(left));
  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(120, 90, 40, 255);
  Freetype::DrawLine(text, r.x + 30, r.y - 8);
}

void MinigameOmokUiSystem::RenderResult() {
  if (result_ < 0) {
    return;
  }
  static auto win =
      WzResource::LoadTexture(WzResource::ui_->find(u"Minigame.img/Common/win"));
  static auto lose =
      WzResource::LoadTexture(WzResource::ui_->find(u"Minigame.img/Common/lose"));
  static auto draw =
      WzResource::LoadTexture(WzResource::ui_->find(u"Minigame.img/Common/draw"));
  static auto char_win = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Common/charWin"));
  static auto char_lose = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Common/charLose"));
  static auto char_draw = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Common/charDraw"));

  // 大幅结算画布盖在棋盘上（095 DrawGameResult：248x117 居中于棋盘）。
  auto* big = result_ == 0 ? win : (result_ == 1 ? lose : draw);
  SDL_FRect r{pos_.x + 63, pos_.y + 151, static_cast<float>(big->w),
              static_cast<float>(big->h)};
  SDL_RenderTexture(Window::renderer_, big, nullptr, &r);

  // 头顶小牌（095：胜负牌画在头像上方 y=113）。
  auto* my_tag = result_ == 0   ? char_win
                 : result_ == 1 ? char_lose
                                : char_draw;
  auto* peer_tag = result_ == 0   ? char_lose
                   : result_ == 1 ? char_win
                                  : char_draw;
  r = {pos_.x + kSlotX[my_slot_] - static_cast<float>(my_tag->w) / 2,
       pos_.y + 108, static_cast<float>(my_tag->w),
       static_cast<float>(my_tag->h)};
  SDL_RenderTexture(Window::renderer_, my_tag, nullptr, &r);
  r = {pos_.x + kSlotX[1 - my_slot_] - static_cast<float>(peer_tag->w) / 2,
       pos_.y + 108, static_cast<float>(peer_tag->w),
       static_cast<float>(peer_tag->h)};
  SDL_RenderTexture(Window::renderer_, peer_tag, nullptr, &r);
}

void MinigameOmokUiSystem::RenderChat() {
  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(80, 60, 20, 255);
  int begin = std::max(0, static_cast<int>(chat_log_.size()) - kChatMaxLines);
  for (int i = begin; i < static_cast<int>(chat_log_.size()); ++i) {
    Freetype::DrawLine(chat_log_[i], pos_.x + 392,
                       pos_.y + kChatLogY + (i - begin) * kChatLineH);
  }
  chat_input_.Render();
}

UIComponents& MinigameOmokUiSystem::GetComponent() {
  components_.clear();
  auto add_button = [](const char16_t* path, SDL_FRect rect, bool enable,
                       void (*callback)()) {
    if (!enable) {
      return;
    }
    Button button{.node = WzResource::ui_->find(path),
                  .rect = rect,
                  .enable = true,
                  .focused = CursorGameInstance::cursor_ui_ == Render};
    button.rect.x += pos_.x;
    button.rect.y += pos_.y;
    button.callback = callback;
    components_.push_back(button);
  };

  if (phase_ == Phase::kPlaying) {
    add_button(u"Minigame.img/btForfeit", kForfeitRect, true,
               EventButtonForfeit);
    add_button(u"Minigame.img/btUndo", kUndoRect, !undo_pending_ && !history_.empty(),
               EventButtonUndo);
    add_button(u"Minigame.img/btDraw", kDrawRect, !draw_pending_,
               EventButtonDraw);
  } else if (has_peer_ && phase_ != Phase::kWaiting) {
    if (my_slot_ == 0) {
      // 房主：等对方就绪后开始。
      add_button(u"Minigame.img/btStart", kStartRect, peer_ready_,
                 EventButtonStart);
    } else if (!my_ready_) {
      // 客方：按准备。
      add_button(u"Minigame.img/btReady", kReadyRect, true, EventButtonReady);
    }
  }
  add_button(u"Minigame.img/btLeave", kLeaveRect, true, EventButtonLeave);

  return components_;
}

bool MinigameOmokUiSystem::EventUI(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), true);
}

bool MinigameOmokUiSystem::Render() {
  RenderBackgrnd();
  RenderTurn();
  RenderStoneBoxes();
  RenderPlayers();
  RenderStones();
  RenderResult();
  RenderChat();
  RenderUI();
  return true;
}

void MinigameOmokUiSystem::PutStoneAt(float mx, float my) {
  float dx = mx - pos_.x - kBoardOriginX;
  float dy = my - pos_.y - kBoardOriginY;
  if (dx < -8 || dy < -8 || dx > (kBoardSize - 1) * kBoardSpacing + 8 ||
      dy > (kBoardSize - 1) * kBoardSpacing + 8) {
    return;
  }
  int col = static_cast<int>(std::lround(dx / kBoardSpacing));
  int row = static_cast<int>(std::lround(dy / kBoardSpacing));
  if (col < 0 || col >= kBoardSize || row < 0 || row >= kBoardSize) {
    return;
  }
  // 095 GetStoneCheckerPositionFromPoint：交叉点周围 14x14 判定。
  if (std::fabs(dx - col * kBoardSpacing) > 8 ||
      std::fabs(dy - row * kBoardSpacing) > 8) {
    return;
  }
  if (board_[row][col] != 0) {
    return;
  }
  SendStep(4, static_cast<uint8_t>(col), static_cast<uint8_t>(row));
}

void MinigameOmokUiSystem::EventButtonStart() {
  SendStep(2);
}

void MinigameOmokUiSystem::EventButtonReady() {
  my_ready_ = true;
  AudioGameInstance::PlayNow(u"MiniGame.img/Ready");
  SendStep(1);
}

void MinigameOmokUiSystem::EventButtonForfeit() {
  SendStep(10);
}

void MinigameOmokUiSystem::EventButtonUndo() {
  if (undo_pending_) {
    return;
  }
  undo_pending_ = true;
  SendStep(6);
}

void MinigameOmokUiSystem::EventButtonDraw() {
  if (draw_pending_) {
    return;
  }
  draw_pending_ = true;
  SendStep(8);
}

void MinigameOmokUiSystem::EventButtonLeave() {
  SendStep(3);
  phase_ = Phase::kIdle;
  Close();
}

bool MinigameOmokUiSystem::Event(SDL_Event* event) {
  if (phase_ == Phase::kIdle) {
    return true;
  }
  auto cr = chat_input_.Event(event);
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        // 点击输入框外时退出输入状态，恢复游戏键盘系统。
        if (cr &&
            std::ranges::find(System::event_systems_,
                               &KeyboardInputSystem::Event) ==
                System::event_systems_.end()) {
          System::event_systems_.push_back(KeyboardInputSystem::Event);
        }
        if (CursorIn()) {
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT && CursorIn()) {
        if (!EventUI(event)) {
          if (phase_ == Phase::kPlaying && turn_color_ == my_color_) {
            PutStoneAt(Window::mouse_pos_.x, Window::mouse_pos_.y);
          }
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
        if (chat_input_.active) {
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
          EventButtonLeave();
        }
        r = false;
      } else if (scan_code == SDL_SCANCODE_RETURN) {
        if (chat_input_.active) {
          if (!chat_input_.text.empty()) {
            fbs::ClientCharacterMinigameT m;
            m.step = 5;
            m.chat = {chat_input_.text.begin(), chat_input_.text.end()};
            ClientRequest::SendToHost(m);
            chat_input_.text.clear();
            chat_input_.cur = 0;
            chat_input_.composition.clear();
          }
        } else {
          chat_input_.Active();
          std::erase(System::event_systems_, KeyboardInputSystem::Event);
        }
        r = false;
      } else if (chat_input_.active) {
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