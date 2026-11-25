#include "src/client/system/ui/party_ui_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/party_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>

#include <algorithm>

namespace sdlms {

namespace {

std::u16string U16FromNum(uint32_t num) {
  auto s = std::to_string(num);
  return {s.begin(), s.end()};
}

constexpr float kLineH = 16;   // 每名成员一行的行距
constexpr float kPadding = 4;  // 内容边距

}  // namespace

void PartyUiSystem::RenderBackgrnd(float x, float y, float w, float h) {
  // 半透明黑底（对齐 CUIPartyHP 默认无贴图窗口的观感）。
  SDL_SetRenderDrawBlendMode(Window::renderer_, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(Window::renderer_, 20, 20, 20, 180);
  SDL_FRect rect{x, y, w, h};
  SDL_RenderFillRect(Window::renderer_, &rect);
  // 一像素描边。
  SDL_SetRenderDrawColor(Window::renderer_, 90, 80, 60, 255);
  SDL_RenderRect(Window::renderer_, &rect);
}

void PartyUiSystem::RenderMembers() {
  if (!PartyGameInstance::data_.has_value()) {
    return;
  }
  const auto& party = PartyGameInstance::data_.value();

  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);

  float y = pos_.y + kPadding + 4;  // 顶部留标题两行

  // 第一行：标题（队长可看"邀请"提示）。
  {
    auto is_captain = !party.members.empty() &&
                      party.members.front().player.client_id ==
                          ServerMain::local_addr_;
    std::u16string title = u"PARTY";
    if (is_captain) {
      title += u"  (P: 邀请面板)";
    }
    Freetype::LoadColor(240, 220, 130, 255);
    Freetype::DrawLine(title, pos_.x + kPadding, pos_.y + kPadding);
  }

  // 成员行：名字(队长前★) + 等级 + HP 条 + hp/max。
  float name_x = pos_.x + kPadding;
  float row_y = y + 12;
  for (size_t i = 0; i < party.members.size(); ++i) {
    const auto& m = party.members[i];
    std::u16string name;
    if (m.player.character != nullptr) {
      name.assign(m.player.character->name.begin(),
                  m.player.character->name.end());
    }
    bool captain = m.player.client_id == party.captain_id;
    auto level = m.player.character != nullptr ? m.player.character->level : 0;

    // 名字 + 等级（095 PartyHP：名字 + Lv）。
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawLine((captain ? u"[C] " : u"") + name + u"  Lv." +
                           U16FromNum(level),
                       name_x, row_y);

    // HP 条（宽 60x6）：底灰 + 绿条 + 数字。
    float bar_x = pos_.x + LoadWh().x - 118;
    float bar_w = 60;
    SDL_FRect bar{bar_x, row_y + 4, bar_w, 6};
    SDL_SetRenderDrawColor(Window::renderer_, 60, 60, 60, 255);
    SDL_RenderFillRect(Window::renderer_, &bar);
    float ratio = m.max_hp > 0
                      ? static_cast<float>(m.hp) / static_cast<float>(m.max_hp)
                      : 0.0f;
    ratio = std::clamp(ratio, 0.0f, 1.0f);
    SDL_FRect fill{bar.x, bar.y, bar_w * ratio, 6};
    // HP 颜色按百分比（绿→黄→红）。
    if (ratio > 0.5f) {
      SDL_SetRenderDrawColor(Window::renderer_, 60, 200, 80, 255);
    } else if (ratio > 0.25f) {
      SDL_SetRenderDrawColor(Window::renderer_, 220, 200, 60, 255);
    } else {
      SDL_SetRenderDrawColor(Window::renderer_, 220, 70, 60, 255);
    }
    SDL_RenderFillRect(Window::renderer_, &fill);

    // hp/max 数字。
    Freetype::LoadColor(200, 200, 200, 255);
    Freetype::DrawLine(U16FromNum(m.hp) + u"/" + U16FromNum(m.max_hp),
                       bar_x + bar_w + 6, row_y);

    row_y += kLineH;
  }
}

SDL_FPoint PartyUiSystem::LoadWh() {
  // 095 CUIPartyHP：宽 = max(名字宽 + 100, 150)。
  float w = 150;
  if (PartyGameInstance::data_.has_value()) {
    Freetype::LoadSize(11);
    for (const auto& m :
         PartyGameInstance::data_.value().members) {
      std::u16string name;
      if (m.player.character != nullptr) {
        name.assign(m.player.character->name.begin(),
                    m.player.character->name.end());
      }
      auto line_w =
          static_cast<float>(Freetype::LoadW(name + u"  Lv.000  00/00")) +
          100.0f;
      w = std::max(w, line_w);
    }
  }
  size_t count = PartyGameInstance::data_.has_value()
                     ? PartyGameInstance::data_.value().members.size()
                     : 0;
  float h = kPadding * 2 + 14 + kLineH * static_cast<float>(count) + 4;
  return {w, h};
}

bool PartyUiSystem::Render() {
  if (!PartyGameInstance::data_.has_value()) {
    return true;
  }
  auto [w, h] = LoadWh();
  RenderBackgrnd(pos_.x, pos_.y, w, h);
  RenderMembers();
  return true;
}

void PartyUiSystem::Open() {
  auto it = std::ranges::find(System::render_systems_, &Render);
  if (it != System::render_systems_.end()) {
    return;
  }
  it = std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    // 095 CUIPartyHP：默认停在左上角（聊天框下方）。
    pos_ = {10, 130};
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void PartyUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void PartyUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void PartyUiSystem::EventDragStart(SDL_Event* event) {
  auto [w, h] = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, w, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
  }
  return;
}

void PartyUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

void PartyUiSystem::EventDragMove(SDL_Event* event) {
  if (drag_.has_value()) {
    pos_ = {event->motion.x + drag_->x, event->motion.y + drag_->y};
    auto& camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  }
  return;
}

void PartyUiSystem::EventButtonLeave() {
  // 退队（095 SendWithdrawPartyMsg → CP 145 step2）。
  fbs::ClientCharacterPartyT cp;
  cp.step = 3;
  ClientRequest::SendToHost(cp);
}

void PartyUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool PartyUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool PartyUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          EventTop();
          EventDragStart(event);
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
        }
        EventDragEnd();
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventDragMove(event);
      break;
    }
    case SDL_EVENT_KEY_DOWN: {
      // 右键成员 / 无队伍时按 P 已由键位系统 Toggle；
      // 这里处理：在队伍中按 ESC 附近无动作，保持简单。
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

}  // namespace sdlms