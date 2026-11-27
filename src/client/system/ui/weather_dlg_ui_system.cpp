#include "src/client/system/ui/weather_dlg_ui_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <algorithm>
#include <string>

namespace sdlms {

namespace {

// 095 CUtilDlgEx：窗口宽 529，最小高 190。素材 t/c/s 竖向三段拼接。
constexpr float kWinW = 529;
constexpr float kWinH = 190;
constexpr SDL_FRect kInput{25, 66, 479, 22};
constexpr SDL_FRect kBtnOk{413, 154, 46, 18};
constexpr SDL_FRect kBtnCancel{467, 154, 46, 18};

}  // namespace

bool WeatherDlgUiSystem::IsOpen() {
  return std::ranges::contains(System::render_systems_, &Render);
}

bool WeatherDlgUiSystem::CursorIn() {
  SDL_FRect pos_rect{pos_.x, pos_.y, kWinW, kWinH};
  return SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect);
}

void WeatherDlgUiSystem::Open(const std::u16string& item_id) {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it == System::render_systems_.end()) {
    return;
  }
  item_id_ = item_id;

  auto& camera = CameraGameInstance::camera_;
  pos_.x = static_cast<float>((camera.w - kWinW) / 2);
  pos_.y = static_cast<float>((camera.h - kWinH) / 2);

  input_ = TextInput{
      .max_size = 35,
      .text = u"",
      .composition = u"",
      .disable = false,
      .active = false,
      .r =
          SDL_Rect{
              static_cast<int>(pos_.x + kInput.x + 4),
              static_cast<int>(pos_.y + kInput.y + 2),
              static_cast<int>(kInput.w - 8),
              static_cast<int>(kInput.h - 4),
          },
      .pos = {0, 0},
      .font_color = {255, 255, 255, 255},
      .cur_color = {255, 255, 255, 255},
      .font_size = 12,
  };
  input_.type.set(TextInput::kDigit);
  input_.type.set(TextInput::kLetter);
  input_.type.set(TextInput::kSymbol);
  input_.type.set(TextInput::kIme);

  System::render_systems_.insert(it, Render);
  System::event_systems_.insert(System::event_systems_.begin(), Event);

  input_.Active();
  std::erase(System::event_systems_, KeyboardInputSystem::Event);
}

void WeatherDlgUiSystem::RestoreKeyboard() {
  input_.Close();
  if (std::ranges::find(System::event_systems_, &KeyboardInputSystem::Event) ==
      System::event_systems_.end()) {
    System::event_systems_.push_back(KeyboardInputSystem::Event);
  }
}

void WeatherDlgUiSystem::Close() {
  RestoreKeyboard();
  item_id_.clear();
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void WeatherDlgUiSystem::EventButtonOk() {
  // 095：输入为空（trim 后）视为取消。
  if (input_.text.empty()) {
    Close();
    return;
  }
  if (!item_id_.empty()) {
    fbs::ClientCharacterWeatherT m;
    m.map_id = SceneSystemInstance::map_id_;
    m.item_id = static_cast<uint32_t>(std::stoul(std::string{
        item_id_.begin(), item_id_.end()}));
    m.message = {input_.text.begin(), input_.text.end()};
    ClientRequest::SendToHost(m);
    // 现金道具使用即消耗（095 服务端 CP 消耗后回包；本地存档同步扣除）。
    ItemGameInstance::DecItemById(item_id_);
  }
  Close();
}

void WeatherDlgUiSystem::EventButtonCancel() { Close(); }

UIComponents& WeatherDlgUiSystem::GetComponent() {
  components_.clear();
  auto add_button = [&](const char16_t* path, SDL_FRect rect,
                        void (*callback)()) {
    Button b{
        .node = WzResource::ui_->find(path),
        .rect = rect,
        .callback = callback,
        .enable = true,
        .focused = CursorGameInstance::cursor_ui_ == Render,
    };
    b.rect.x += pos_.x;
    b.rect.y += pos_.y;
    components_.push_back(b);
  };
  add_button(u"UtilDlgEx.img/UtilDlgEx/BtOK", kBtnOk, EventButtonOk);
  add_button(u"UtilDlgEx.img/UtilDlgEx/BtNo", kBtnCancel, EventButtonCancel);
  return components_;
}

bool WeatherDlgUiSystem::EventUI(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), true);
}

void WeatherDlgUiSystem::RenderBackgrnd() {
  static auto t =
      WzResource::LoadTexture(WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/t"));
  static auto c =
      WzResource::LoadTexture(WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/c"));
  static auto s =
      WzResource::LoadTexture(WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/s"));

  SDL_FRect rect{
      pos_.x,
      pos_.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &rect);

  rect = {
      pos_.x,
      pos_.y + t->h,
      static_cast<float>(c->w),
      kWinH - t->h - s->h,
  };
  SDL_RenderTextureTiled(Window::renderer_, c, nullptr, 1, &rect);

  rect = {
      pos_.x,
      pos_.y + kWinH - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_RenderTexture(Window::renderer_, s, nullptr, &rect);
}

void WeatherDlgUiSystem::RenderText() {
  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(255, 255, 255, 255);
  auto text = WzResource::LoadMSString(u"weatherPrompt",
                                       u"请输入要显示的文字。");
  auto w = Freetype::LoadW(text);
  Freetype::DrawLine(text, pos_.x + (kWinW - w) / 2, pos_.y + 36);
}

void WeatherDlgUiSystem::RenderInput() {
  SDL_FRect rect{
      pos_.x + kInput.x,
      pos_.y + kInput.y,
      kInput.w,
      kInput.h,
  };
  SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, 160);
  SDL_RenderFillRect(Window::renderer_, &rect);
  SDL_SetRenderDrawColor(Window::renderer_, 210, 210, 210, 255);
  SDL_RenderRect(Window::renderer_, &rect);
  input_.Render();
}

bool WeatherDlgUiSystem::Render() {
  RenderBackgrnd();
  RenderText();
  RenderInput();
  RenderUI();
  return true;
}

bool WeatherDlgUiSystem::Event(SDL_Event* event) {
  // 095 CUtilDlgEx 是模态窗：事件不向下层透传。
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT && CursorIn()) {
        // 点在输入框内聚焦；点在其他位置时 TextInput 自己失焦。
        input_.Event(event);
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT && CursorIn()) {
        EventUI(event);
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      input_.Event(event);
      break;
    }
    case SDL_EVENT_TEXT_INPUT:
    case SDL_EVENT_TEXT_EDITING: {
      input_.Event(event);
      break;
    }
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = event->key.scancode;
      if (scan_code == SDL_SCANCODE_ESCAPE) {
        Close();
      } else if (scan_code == SDL_SCANCODE_RETURN) {
        EventButtonOk();
      } else if (input_.active) {
        input_.Event(event);
      }
      break;
    }
    default: {
      break;
    }
  }
  return false;
}

}  // namespace sdlms
