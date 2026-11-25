#include "src/client/system/ui/minigame_card_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/ui/minigame_omok_ui_system.h"
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

// 全屏版 Minigame.img/MatchCards/backgrnd 布局（像素实测）：
// 左侧 x0-362 为卡区；右侧 378-545 为两块站位白板 y49-150、
// 两块分数小板 y169-205；底部 y389-410 为按钮条。
constexpr float kSlotX[2] = {419, 506};
constexpr float kSlotY = 142;
constexpr float kNameTagY = 150;
constexpr float kScoreCx[2] = {403, 462};
constexpr float kScoreY = 171;
constexpr float kReadyLampY = 212;
constexpr float kChatLogY = 240;
constexpr float kChatLineH = 14;
constexpr int kChatMaxLines = 7;

constexpr SDL_FRect kStartRect = {378, 224, 65, 19};
constexpr SDL_FRect kReadyCardRect = {470, 224, 65, 19};
constexpr SDL_FRect kForfeitRect = {28, 391, 50, 17};
constexpr SDL_FRect kDrawRect = {88, 391, 54, 17};
constexpr SDL_FRect kLeaveRect = {549, 391, 41, 17};

std::u16string U16FromNum(uint32_t num) {
  auto s = std::to_string(num);
  return {s.begin(), s.end()};
}

}  // namespace

SDL_FPoint MinigameCardUiSystem::LoadWh() {
  return {598, 419};
}

bool MinigameCardUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect);
}

void MinigameCardUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  auto [w, h] = LoadWh();
  auto& camera = CameraGameInstance::camera_;
  pos_.x = static_cast<float>((camera.w - w) / 2);
  pos_.y = static_cast<float>((camera.h - h) / 2);
  show_self_ = CharacterGameInstance::self_;
  MinigameOmokUiSystem::PrepareShowCharacter(show_self_);
  chat_input_.Close();
  chat_input_ = TextInput{
      .max_size = 60,
      .text = u"",
      .composition = u"",
      .disable = false,
      .active = false,
      .r = SDL_Rect{static_cast<int>(pos_.x + 380),
                    static_cast<int>(pos_.y + 362), 158, 16},
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

void MinigameCardUiSystem::Close() {
  chat_input_.Close();
  if (std::ranges::find(System::event_systems_, &KeyboardInputSystem::Event) ==
      System::event_systems_.end()) {
    System::event_systems_.push_back(KeyboardInputSystem::Event);
  }
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void MinigameCardUiSystem::AddChat(const std::u16string& line) {
  chat_log_.push_back(line);
  if (chat_log_.size() > 100) {
    chat_log_.erase(chat_log_.begin());
  }
}

void MinigameCardUiSystem::ResetRound() {
  for (auto& f : face_) {
    f = Face::kBack;
  }
  for (auto& c : cards_) {
    c = 0;
  }
  for (auto& t : match_fx_at_) {
    t = 0;
  }
  pair_score_[0] = 0;
  pair_score_[1] = 0;
  picked_ = 0;
  flip_back_[0] = -1;
  flip_back_[1] = -1;
  flip_back_at_ = 0;
  preview_until_ = 0;
  result_ = -1;
  my_turn_ = false;
  draw_pending_ = false;
}

void MinigameCardUiSystem::SendStep(uint8_t step, uint8_t x) {
  fbs::ClientCharacterMinigameT m;
  m.step = step;
  m.x = x;
  ClientRequest::SendToHost(m);
}

void MinigameCardUiSystem::EnterRoomByItem() {
  // x=1：卡片配对房（GameKind=2，30 张）。
  SendStep(0, 1);
}

void MinigameCardUiSystem::EnsureItem() {
  auto& items = PackageGameInstance::data_[(int)ItemEnum::kInstall];
  for (auto& itm : items) {
    if (itm->id.starts_with(u"040801")) {
      return;
    }
  }
  for (auto& slot : items) {
    if (slot->id.empty()) {
      slot = ItemGameInstance::LoadItem(u"04080100", 1);
      return;
    }
  }
}

void MinigameCardUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/MatchCards/backgrnd"));
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
}

void MinigameCardUiSystem::RenderPlayers() {
  static auto name_tag = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Common/nameTag0"));
  static auto name_tag_empty = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Common/emptyTag"));

  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);

  auto draw_side = [&](int slot, bool present, const std::u16string& name,
                       GameCharacter* g) {
    // 角色立绘（095 GetActionPosition：站台，slot0 朝右 / slot1 朝左）。
    if (present && g != nullptr) {
      const auto& avatars = CharacterGameInstance::avatar_data_;
      const auto& faces = CharacterGameInstance::face_data_;
      bool ready = avatars.contains(g->head) && avatars.contains(g->body) &&
                   avatars.contains(g->hair) && faces.contains(g->face.id) &&
                   faces.at(g->face.id).data.contains(g->face.action);
      if (ready) {
        auto& camera = CameraGameInstance::camera_;
        g->pos = {pos_.x + kSlotX[slot] + camera.x,
                  pos_.y + kSlotY + camera.y};
        g->flip = slot == 0 ? 1 : 0;
        CharacterLogicSystem::RunAnimate(*g);
        CharacterRenderSystem::RenderCharacter(*g);
      }
    }

    // 名牌（站位下方）。
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

void MinigameCardUiSystem::RenderCards() {
  static auto back = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/MatchCards/card/back2"));
  // 095 DrawEffect：GameKind=2 用 effect2（橙色发光边框，origin (6,5)），
  // 以 3 倍缩放出现后常驻在配对成功的卡上。
  static auto match_fx = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/MatchCards/effect2"));
  auto tex_of = [&](int idx) -> SDL_Texture* {
    auto v = cards_[idx];
    auto* node = WzResource::ui_->find(
        u"Minigame.img/MatchCards/card/" +
        std::u16string{U16FromNum(static_cast<uint32_t>(v))});
    return WzResource::LoadTexture(node);
  };

  bool preview = Window::dt_now_ < preview_until_;
  for (int i = 0; i < kCardCount; ++i) {
    float x = pos_.x + kCardOriginX + (i % kCardCols) * kCardGapX;
    float y = pos_.y + kCardOriginY + (i / kCardCols) * kCardGapY;
    SDL_FRect r{x, y, kCardW, kCardH};

    bool open = face_[i] == Face::kOpen || face_[i] == Face::kMatched ||
                preview;
    if (open && cards_[i] <= 14) {
      auto* tex = tex_of(i);
      SDL_SetTextureAlphaMod(tex, face_[i] == Face::kMatched ? 200 : 255);
      SDL_RenderTexture(Window::renderer_, tex, nullptr, &r);
    } else {
      SDL_SetTextureAlphaMod(back, 255);
      SDL_RenderTexture(Window::renderer_, back, nullptr, &r);
    }

    // 配对成功光效：出现时从 3 倍缩到 1 倍（zoom 300），之后常驻。
    if (face_[i] == Face::kMatched && match_fx_at_[i] != 0) {
      constexpr uint64_t kFxMs = 300;
      float scale = 1.0f;
      uint64_t el = Window::dt_now_ - match_fx_at_[i];
      if (el < kFxMs) {
        scale = 3.0f - 2.0f * el / kFxMs;
      }
      // origin (6,5)：绘制左上角 = 卡左上角 - origin，缩放以卡中心为锚。
      float cx = x + kCardW / 2;
      float cy = y + kCardH / 2;
      float w = match_fx->w * scale;
      float h = match_fx->h * scale;
      SDL_FRect fx{cx - w / 2, cy - h / 2, w, h};
      SDL_RenderTexture(Window::renderer_, match_fx, nullptr, &fx);
    }
  }
}

void MinigameCardUiSystem::RenderScore() {
  // 095 DrawScore：MatchCards/number 数字牌显示各自配对数。
  for (int slot = 0; slot < 2; ++slot) {
    uint8_t score = pair_score_[slot];
    // 分数最大 15，number/0..15 一牌一图。
    auto num = std::u16string{U16FromNum(score)};
    auto* node = WzResource::ui_->find(u"Minigame.img/MatchCards/number/" + num);
    if (node == nullptr) {
      continue;
    }
    auto* tex = WzResource::LoadTexture(node);
    SDL_FRect r{pos_.x + kScoreCx[slot] - static_cast<float>(tex->w) / 2,
                pos_.y + kScoreY, static_cast<float>(tex->w),
                static_cast<float>(tex->h)};
    SDL_RenderTexture(Window::renderer_, tex, nullptr, &r);

    // 就绪灯（095 DrawReadyOrNot）。
    static auto ready_on = WzResource::LoadTexture(
        WzResource::ui_->find(u"Minigame.img/Common/readyOn"));
    static auto ready_off = WzResource::LoadTexture(
        WzResource::ui_->find(u"Minigame.img/Common/readyOff"));
    bool ready = phase_ == Phase::kPlaying;
    if (slot == my_slot_) {
      ready = ready || my_ready_;
    } else {
      ready = ready || peer_ready_;
    }
    auto* lamp = ready ? ready_on : ready_off;
    r = {pos_.x + kScoreCx[slot] - static_cast<float>(lamp->w) / 2,
         pos_.y + kReadyLampY, static_cast<float>(lamp->w),
         static_cast<float>(lamp->h)};
    SDL_RenderTexture(Window::renderer_, lamp, nullptr, &r);
  }

  // 回合指示：当前行动者的分数牌旁画 mark。
  if (phase_ == Phase::kPlaying) {
    static auto mark = WzResource::LoadTexture(
        WzResource::ui_->find(u"Minigame.img/Common/mark"));
    int actor = my_turn_ ? my_slot_ : (1 - my_slot_);
    SDL_FRect r{pos_.x + kScoreCx[actor] - 34, pos_.y + kScoreY + 4,
                static_cast<float>(mark->w), static_cast<float>(mark->h)};
    SDL_RenderTexture(Window::renderer_, mark, nullptr, &r);
  }
}

void MinigameCardUiSystem::RenderChat() {
  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(60, 40, 20, 255);
  int begin = std::max(0, static_cast<int>(chat_log_.size()) - kChatMaxLines);
  for (int i = begin; i < static_cast<int>(chat_log_.size()); ++i) {
    Freetype::DrawLine(chat_log_[i], pos_.x + 382,
                       pos_.y + kChatLogY + (i - begin) * kChatLineH);
  }
  chat_input_.Render();
}

UIComponents& MinigameCardUiSystem::GetComponent() {
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
    add_button(u"Minigame.img/btDraw", kDrawRect, !draw_pending_,
               EventButtonDraw);
  } else if (has_peer_ && phase_ != Phase::kWaiting) {
    if (my_slot_ == 0) {
      add_button(u"Minigame.img/btStart", kStartRect, peer_ready_,
                 EventButtonStart);
    } else if (!my_ready_) {
      add_button(u"Minigame.img/btReady", kReadyCardRect, true,
                 EventButtonReady);
    }
  }
  add_button(u"Minigame.img/btLeave", kLeaveRect, true, EventButtonLeave);

  return components_;
}

bool MinigameCardUiSystem::EventUI(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), true);
}

bool MinigameCardUiSystem::Render() {
  // 配对失败延时翻回（095 m_bTurnBack）。
  if (flip_back_[0] >= 0 && Window::dt_now_ >= flip_back_at_) {
    face_[flip_back_[0]] = Face::kBack;
    face_[flip_back_[1]] = Face::kBack;
    flip_back_[0] = -1;
    flip_back_[1] = -1;
  }

  RenderBackgrnd();
  RenderPlayers();
  RenderCards();
  RenderScore();
  RenderChat();
  RenderUI();
  return true;
}

int MinigameCardUiSystem::CardAt(float mx, float my) {
  // 095 GetCardPositionFromPoint：卡矩形逐一命中测试。
  SDL_FPoint p{mx, my};
  for (int i = 0; i < kCardCount; ++i) {
    float x = pos_.x + kCardOriginX + (i % kCardCols) * kCardGapX;
    float y = pos_.y + kCardOriginY + (i / kCardCols) * kCardGapY;
    SDL_FRect r{x, y, kCardW, kCardH};
    if (SDL_PointInRectFloat(&p, &r)) {
      return i;
    }
  }
  return -1;
}

void MinigameCardUiSystem::EventButtonStart() {
  SendStep(2);
}

void MinigameCardUiSystem::EventButtonReady() {
  my_ready_ = true;
  AudioGameInstance::PlayNow(u"MiniGame.img/Ready");
  SendStep(1);
}

void MinigameCardUiSystem::EventButtonForfeit() {
  SendStep(10);
}

void MinigameCardUiSystem::EventButtonDraw() {
  if (draw_pending_) {
    return;
  }
  draw_pending_ = true;
  SendStep(8);
}

void MinigameCardUiSystem::EventButtonLeave() {
  SendStep(3);
  phase_ = Phase::kIdle;
  Close();
}

bool MinigameCardUiSystem::Event(SDL_Event* event) {
  if (phase_ == Phase::kIdle) {
    return true;
  }
  auto cr = chat_input_.Event(event);
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
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
          // 点击翻卡（095 OnMouseButton：轮到自己时连点两张）。
          if (phase_ == Phase::kPlaying && my_turn_ && picked_ < 2) {
            int idx = CardAt(Window::mouse_pos_.x, Window::mouse_pos_.y);
            if (idx >= 0 && face_[idx] == Face::kBack) {
              SendStep(4, static_cast<uint8_t>(idx));
              ++picked_;
            }
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